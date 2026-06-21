#define me "FTxx"
#define i2cdata D1
#define i2cclock D2
#define INCLUDE_FILE_MANAGER
#define NO_ONEWIRE // Tells jvcommon to strip out all sensor loops & delays
#define NOSETUP
#define LOG_LEVEL LOG_INFO
#include <jvcommon.h>
#include <cctype>
#include <cmath> // Exponential math core

String version = "1.0.4-scaled90";

// --- DIMMER HARDWARE CONFIGURATION ---
const int NUM_CHANNELS = 4;
const int CHANNELS[] = { 14, 12, 13, 4 }; // D5, D6, D7, D2
const int PWM_RANGE = 1023;

// INDIVIDUAL CHANNEL MAX BRIGHTNESS SCALING (0 to 1023)
// 921 matches exactly 90% of your maximum 10-bit PWM capacity (1023 * 0.90)
// Adjust these values individually once your new waterproof strings arrive!
const int MAX_LIMITS[NUM_CHANNELS] = { 921, 921, 921, 921 }; 

const float RATE_FAST = (float)PWM_RANGE / 20000.0;
const float RATE_SLOW = (float)PWM_RANGE / 900000.0;

enum FadeSpeed { IMMEDIATE, FAST, SLOW };

struct DimmerChannel {
  float currentDuty;       // Tracks linear trajectory
  int targetDuty;          // Target linear endpoint
  int requestedPercent;    // Safe dashboard value tracking
  FadeSpeed activeSpeed;
  bool isMoving;
};

DimmerChannel dimmer[NUM_CHANNELS];
unsigned long lastFadeUpdate = 0;

// --- MQTT & SYSTEM TASK TIMERS ---
unsigned long t1old = 0, t1interval = 120 * 60 * 1000;
unsigned long t3old = 0, t3interval = 10 * 1000; 
unsigned long t4old = 0;
unsigned long lastTimeSync = 0;
const unsigned long TIME_SYNC_INTERVAL = 3600 * 1000;
unsigned long lastSubscriptionCheck = 0;
const char* test_topic = "fish/dimtest";

void ReadSensorInformation() {}

// --- LOGARITHMIC TRANSLATION ENGINE (WITH INVERSION HOOKS) ---
void writeHardwarePWM(int ch, int rawLinearDuty) {
  int maxLimit = MAX_LIMITS[ch];
  int scaledPerceptualDuty = 0;

  if (rawLinearDuty > 0 && maxLimit > 0) {
    float normalized = (float)rawLinearDuty / (float)maxLimit;
    // Exponential power warp for visual perception (active for fast and slow dimming)
    scaledPerceptualDuty = (int)(pow(normalized, 2.5) * maxLimit);
  }

  scaledPerceptualDuty = constrain(scaledPerceptualDuty, 0, maxLimit);

  // INVERSE HARDWARE LOGIC FILTER:
  // PWM_RANGE (1023) = Dark (matches your 1k pullups), 0 = Max bright allowed
  int hardwareInvertedDuty = PWM_RANGE - scaledPerceptualDuty;
  hardwareInvertedDuty = constrain(hardwareInvertedDuty, 0, PWM_RANGE);

  analogWrite(CHANNELS[ch], hardwareInvertedDuty);
}

// --- MQTT MONITOR/PUBLISH STATUS ---
void pubstats(bool forceRetain) {
  if (WiFi.status() != WL_CONNECTED) {
    JVWIFISetUp();
    if (WiFi.status() != WL_CONNECTED) return;
  }
  if (!client.connected()) return; 

  StaticJsonDocument<256> localDoc;
  localDoc.clear();
  localDoc["name"] = "FTDS";
  
  for (int i = 0; i < NUM_CHANNELS; i++) {
    char key[8]; // Correct array size prevents stack memory overflow corruption
    snprintf(key, sizeof(key), "FTD%d", i);
    
    int currentPercent = 0;
    if (MAX_LIMITS[i] > 0) {
      currentPercent = (int)round(((float)dimmer[i].currentDuty / (float)MAX_LIMITS[i]) * 100.0);
    }
    currentPercent = constrain(currentPercent, 0, 100);
    localDoc[key] = currentPercent;
  }
  
  localDoc["IP"] = myip;
  localDoc["Ver"] = version;
  
  String localMsgString;
  serializeJson(localDoc, localMsgString);
  
  bool success = client.publish("fish/FTDS", localMsgString.c_str(), forceRetain);
  if (success) {
    Serial.printf("[MQTT] Status published. Retained: %s\n", forceRetain ? "YES" : "NO");
  } else {
    LOG_WARN("pubstats update failed");
  }
  Serial.flush();
}

void pubstats() {
  pubstats(true);
}

// --- SEAMLESS BACKGROUND INTERPOLATOR ---
void updateDimmerLoop() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastFadeUpdate;
  if (elapsed == 0) return;
  lastFadeUpdate = now;

  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (!dimmer[i].isMoving) continue;
    
    float stepRate = (dimmer[i].activeSpeed == FAST) ? RATE_FAST : RATE_SLOW;
    float totalStep = stepRate * elapsed;
    float distanceLeft = abs(dimmer[i].targetDuty - dimmer[i].currentDuty);
    
    if (totalStep >= distanceLeft || distanceLeft < 0.9) {
      dimmer[i].currentDuty = (float)dimmer[i].targetDuty;
      dimmer[i].isMoving = false;
    } else {
      if (dimmer[i].currentDuty < dimmer[i].targetDuty) {
        dimmer[i].currentDuty += totalStep;
      } else {
        dimmer[i].currentDuty -= totalStep;
      }
    }
    
    writeHardwarePWM(i, (int)round(dimmer[i].currentDuty));
    
    if (!dimmer[i].isMoving) {
      LOG_INFO("Channel %d arrived safely at destination.", i);
      pubstats(true); // Node-RED recovery sync confirmation
    }
  }
}

// --- API COMMAND INTERFACE ---
void setChannelDimmer(int ch, int targetPercent, FadeSpeed speedMode) {
  if (ch < 0 || ch >= NUM_CHANNELS) return;
  
  targetPercent = constrain(targetPercent, 0, 100);
  dimmer[ch].requestedPercent = targetPercent;
  
  int targetPWM = map(targetPercent, 0, 100, 0, MAX_LIMITS[ch]);
  targetPWM = constrain(targetPWM, 0, MAX_LIMITS[ch]);
  
  dimmer[ch].isMoving = false; 
  dimmer[ch].targetDuty = targetPWM;
  dimmer[ch].activeSpeed = speedMode;
  
  if (speedMode == IMMEDIATE) {
    dimmer[ch].currentDuty = (float)targetPWM;
    writeHardwarePWM(ch, targetPWM);
  } else {
    if ((int)round(dimmer[ch].currentDuty) != targetPWM) {
      dimmer[ch].isMoving = true;
    } else {
      dimmer[ch].currentDuty = (float)targetPWM;
    }
  }
}

// --- MQTT ARDUINOJSON INCOMING PAYLOAD PARSER ---
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> callbackDoc;
  callbackDoc.clear();
  DeserializationError error = deserializeJson(callbackDoc, payload, length);
  if (error) return;

  if (strcmp(topic, test_topic) == 0) {
    if (!callbackDoc.containsKey("ch") || !callbackDoc.containsKey("state")) return;
    
    int targetCh = callbackDoc["ch"];
    int targetState = constrain((int)callbackDoc["state"], 0, 100);
    const char* speedStr = callbackDoc["speed"];
    
    FadeSpeed targetSpeed = IMMEDIATE;
    if (speedStr != NULL) {
      String spd = String(speedStr);
      spd.toUpperCase();
      if (spd == "FAST") targetSpeed = FAST;
      else if (spd == "SLOW") targetSpeed = SLOW;
    }
    
    if (targetCh >= 0 && targetCh < NUM_CHANNELS) {
      if (dimmer[targetCh].requestedPercent == targetState && dimmer[targetCh].activeSpeed == targetSpeed) {
        return; 
      }
      
      LOG_INFO("Command accepted: Ch %d to %d%%", targetCh, targetState);
      setChannelDimmer(targetCh, targetState, targetSpeed);
      pubstats(true);
    }
  }
}

void setup() {
  mysetup();
  analogWriteRange(PWM_RANGE);
  analogWriteFreq(1000);
  
  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(CHANNELS[i], OUTPUT);
    analogWrite(CHANNELS[i], PWM_RANGE); // Starts OFF (matches pullups)
    
    dimmer[i].currentDuty = 0.0;
    dimmer[i].targetDuty = 0;
    dimmer[i].requestedPercent = 0;
    dimmer[i].activeSpeed = IMMEDIATE;
    dimmer[i].isMoving = false;
  }
  
  client.setCallback(callback);
  if (client.connected()) {
    client.subscribe(test_topic);
  }
  waitForSync();
  lastTimeSync = millis();
  pubstats();
  
  unsigned long now = millis();
  t1old = now;
  t3old = now;
  t4old = now;
  lastSubscriptionCheck = now;
  lastFadeUpdate = now;
}

void loop() {
  unsigned long now = millis();
  
  client.loop();
  updateDimmerLoop();
  runJvFSManager();

  // Dynamic Live Dimming Update Windows
  bool isAnyChannelFading = false;
  bool isAnyFastFadeRunning = false;
  
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (dimmer[i].isMoving) {
      isAnyChannelFading = true;
      if (dimmer[i].activeSpeed == FAST) {
        isAnyFastFadeRunning = true;
      }
    }
  }

  unsigned long activeInterval = isAnyFastFadeRunning ? (5 * 1000) : (60 * 1000);
  if (isAnyChannelFading) {
    if (now - t4old >= activeInterval) {
      t4old = now;
      pubstats(false); 
    }
  } else {
    t4old = now;
  }

  // Subscription Verification Timer
  if (now - lastSubscriptionCheck >= 5000) {
    lastSubscriptionCheck = now;
    if (client.connected()) {
      client.subscribe(test_topic);
    }
  }

  // Standard 2-Hour Heartbeat Timer
  if (now - t1old >= t1interval) {
    t1old = now;
    LOG_INFO("Standard 2-hour heartbeat status update.");
    pubstats(true);
  }

  // Non-Blocking Connection Verification Timer
  if (now - t3old >= t3interval) {
    t3old = now;
    
    // 1. If WiFi itself dropped out, proactively trigger recovery
    if (WiFi.status() != WL_CONNECTED) {
      LOG_INFO("WiFi disconnected! Running recovery...");
      JVWIFISetUp(); 
    } 
    // 2. If WiFi is fine but MQTT broke, reconnect to broker
    else if (!client.connected()) {
      LOG_INFO("MQTT Disconnected. Attempting non-blocking reconnect...");
      reconnect();
    }
  }

  // Allow ezTime to process NTP background updates so the clock never drifts
  events(); 

  yield();
}