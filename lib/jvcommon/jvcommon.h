#ifndef JVCOMMON_H
#define JVCOMMON_H

// ────────────────────────────────────────────────────────────────
// JVCOMMON.H - Clean reusable library (Verbose #ifdef style)
// ────────────────────────────────────────────────────────────────

#ifndef libver
#define libver "2025.04.28"
#endif

#ifndef serialclock
#define serialclock 9600
#endif
#ifndef me
#define me "testing"
#endif

// Logging
enum LogLevel { LOG_ERROR = 0,
                LOG_WARN = 1,
                LOG_INFO = 2,
                LOG_DEBUG = 3 };
#ifndef LOG_ENABLED
#define LOG_ENABLED true
#endif
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

// Feature defaults
#ifndef OW_PIN1
#define OW_PIN1 -1
#endif
#ifndef OW_PIN2
#define OW_PIN2 -1
#endif
#ifndef OW_PIN3
#define OW_PIN3 -1
#endif

#ifndef MAX_SENSORS_PER_BUS
#define MAX_SENSORS_PER_BUS 10
#endif
#ifndef MAX_PUBLISHED_SENSORS
#define MAX_PUBLISHED_SENSORS (MAX_SENSORS_PER_BUS * 3)
#endif

#ifndef DHTPIN
#define DHTPIN -1
#endif

#ifndef battfactor
#define battfactor 172.5
#endif

#ifndef i2cdata
#ifdef ESP32
#define i2cdata 27
#define i2cclock 22
#else
#define i2cdata D6
#define i2cclock D5
#endif
#endif

// MQTT LWT
#ifndef LWT_TOPIC
#define LWT_TOPIC "fish/status"
#endif
#ifndef LWT_PAYLOAD_OFFLINE
#define LWT_PAYLOAD_OFFLINE "{\"status\":\"offline\"}"
#endif
#ifndef LWT_PAYLOAD_ONLINE
#define LWT_PAYLOAD_ONLINE "{\"status\":\"online\"}"
#endif

#ifndef SENSOR_DEVICE_NAME
#define SENSOR_DEVICE_NAME "SNFT"
#endif
#ifndef MQTT_PUBLISH_TOPIC
#define MQTT_PUBLISH_TOPIC "fish/SNFT"
#endif

#include <credentials.h>

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiUdp.h>
#include "Arduino.h"
#include <WiFiClient.h>
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ezTime.h>

// ────────────────────────────────────────────────────────────────
// CONDITIONAL FILE MANAGER GLOBALS
// ────────────────────────────────────────────────────────────────
#ifdef INCLUDE_FILE_MANAGER
  #include <LittleFS.h>
  #ifdef ESP32
    #include <WebServer.h>
    typedef WebServer JvWebServer;
    inline JvWebServer jvFsServer(80);
  #else
    #include <ESP8266WebServer.h>
    typedef ESP8266WebServer JvWebServer;
    inline JvWebServer jvFsServer(80);
  #endif
  inline File jvUploadFile;
#endif // This line must be the only thing closing the block here!

// Declarations
void initJvFSManager();
void runJvFSManager();


#ifndef NO_ONEWIRE  // <--- ADD THIS WRAPPER
#if OW_PIN1 >= 0 || OW_PIN2 >= 0 || OW_PIN3 >= 0
#include <OneWire.h>
#include <DallasTemperature.h>
#endif
#endif  // <--- ADD THIS WRAPPER


#ifdef DHTPIN
#if DHTPIN >= 0
#include <dhtnew.h>
#endif
#endif

#ifdef BME
#include <Adafruit_BME280.h>
#endif

#ifdef BMP
#include <Adafruit_BMP280.h>
#endif

// Forward declaration for sensor-specific function
void ReadSensorInformation();

// Forward declarations
#ifdef BAT
void readbat();
extern double batv;
extern uint16_t batt;
#endif

#ifdef BME
void readbme();
extern double bmetemp, bmehum, bmepres, bmedew, bmehi;
#endif

#ifdef BMP
void readbmp();
extern double bmptemp, bmppres;
#endif

#ifdef DHTPIN
#if DHTPIN >= 0
void readdht();
extern double dhttemp, dhthum, dhtdew, dhthi;
#endif
#endif

// Globals
WiFiClient espClient;
PubSubClient client(espClient);
Timezone myTZ;
DynamicJsonDocument doc(6000);

String hostname = me;
String myip;
long rssi;
String combinedVersion;
String sensorDeviceName = SENSOR_DEVICE_NAME;
String mqttPublishTopic = MQTT_PUBLISH_TOPIC;
String subClient;
char msg[512];

// OneWire globals
#if OW_PIN1 >= 0
OneWire oneWireBus1(OW_PIN1);
DallasTemperature sensorsBus1(&oneWireBus1);
DeviceAddress addrBus1[MAX_SENSORS_PER_BUS];
double tempBus1[MAX_SENSORS_PER_BUS];
int numSensorsBus1 = 0;
#endif

#if OW_PIN2 >= 0
OneWire oneWireBus2(OW_PIN2);
DallasTemperature sensorsBus2(&oneWireBus2);
DeviceAddress addrBus2[MAX_SENSORS_PER_BUS];
double tempBus2[MAX_SENSORS_PER_BUS];
int numSensorsBus2 = 0;
#endif

#if OW_PIN3 >= 0
OneWire oneWireBus3(OW_PIN3);
DallasTemperature sensorsBus3(&oneWireBus3);
DeviceAddress addrBus3[MAX_SENSORS_PER_BUS];
double tempBus3[MAX_SENSORS_PER_BUS];
int numSensorsBus3 = 0;
#endif

#ifndef NO_ONEWIRE
// DS18B20 publishing arrays
double publishedTemps[MAX_PUBLISHED_SENSORS];
DeviceAddress publishedAddrs[MAX_PUBLISHED_SENSORS];
int publishedSensorCount = 0;
double lastPublishedTemps[MAX_PUBLISHED_SENSORS];
#endif
unsigned long publishCount = 0;
const unsigned long HEARTBEAT_CYCLES = 10;

// Legacy sensor globals
#ifdef BAT
double batv = 0.0;
uint16_t batt = 0;
#endif

#ifdef BME
Adafruit_BME280 bme;
double bmetemp = -999.9, bmehum = 0.0, bmepres = 0.0, bmedew = -999.9, bmehi = -999.9;
#endif

#ifdef BMP
Adafruit_BMP280 bmp;
double bmptemp = -999.9, bmppres = 0.0;
#endif

#ifdef DHTPIN
#if DHTPIN >= 0
DHTNEW dht11(DHTPIN);
double dhttemp = -999.9, dhthum = 0.0, dhtdew = -999.9, dhthi = -999.9;
#endif
#endif

// Helper Functions
double roundTemp(double arg) {
  return (int)(arg * 100 + 0.5) / 100.0;
}

double feels(double thi, double rh) {
  double heatindex = -42.379 + (2.04901523 * thi) + (10.14333127 * rh)
                     - (0.22475541 * thi * rh) - (6.83783e-3 * thi * thi)
                     - (5.481717e-2 * rh * rh) + (1.22874e-3 * thi * thi * rh)
                     + (8.5282e-4 * thi * rh * rh) - (1.99e-6 * thi * thi * rh * rh);
  return roundTemp(heatindex);
}

bool isValidTemp(double tempF) {
  return (tempF > -100.0 && tempF < 300.0);
}

#ifndef NO_ONEWIRE
void formatAddress(const DeviceAddress& addr, char* buf) {
  sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X%02X",
          addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
}
#endif

// Logging
#define LOG_ERROR(...) \
  if (LOG_ENABLED && LOG_LEVEL >= LOG_ERROR) logMessage(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) \
  if (LOG_ENABLED && LOG_LEVEL >= LOG_WARN) logMessage(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) \
  if (LOG_ENABLED && LOG_LEVEL >= LOG_INFO) logMessage(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) \
  if (LOG_ENABLED && LOG_LEVEL >= LOG_DEBUG) logMessage(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

void logMessage(LogLevel level, const char* file, int line, const char* format, ...) {
  if (!LOG_ENABLED) return;
  String ts = myTZ.dateTime("d-g:i:s");
  const char* lvl = (level == LOG_ERROR) ? "ERROR" : (level == LOG_WARN) ? "WARN "
                                                   : (level == LOG_INFO) ? "INFO "
                                                                         : "DEBUG";
  const char* fn = strrchr(file, '/') ? strrchr(file, '/') + 1 : file;
  char buf[256];
  snprintf(buf, sizeof(buf), "[%s] %s (%s:%d) ", ts.c_str(), lvl, fn, line);
  va_list args;
  va_start(args, format);
  vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), format, args);
  va_end(args);
  Serial.println(buf);
  Serial.flush();
}

// OneWire Functions
#if OW_PIN1 >= 0 || OW_PIN2 >= 0 || OW_PIN3 >= 0

void readBus(DallasTemperature& sensors, DeviceAddress* addrs, double* temps, int& numSensors, int busId) {
  int detected = sensors.getDeviceCount();
  if (detected != numSensors) {
    LOG_INFO("Bus %d sensor count changed: %d → %d", busId, numSensors, detected);
    numSensors = std::min(detected, MAX_SENSORS_PER_BUS);
    for (int i = 0; i < numSensors; i++) sensors.getAddress(addrs[i], i);
  }

  for (int i = 0; i < numSensors; i++) {
    float c = sensors.getTempC(addrs[i]);
    double f = NAN;
    if (c != DEVICE_DISCONNECTED_C && c != 85.0f) {
      f = roundTemp(c * 1.8 + 32.0);
      if (!isValidTemp(f)) f = NAN;
    }
    temps[i] = f;

    char amsg[32];
    formatAddress(addrs[i], amsg);
    LOG_DEBUG("Bus %d Addr=%s Temp=%.2f°F", busId, amsg, isnan(f) ? 0.0 : f);
    delay(20);
  }
}

void combineBusesForPublish() {
  publishedSensorCount = 0;
#if OW_PIN1 >= 0
  for (int i = 0; i < numSensorsBus1; i++)
    if (!isnan(tempBus1[i])) {
      publishedTemps[publishedSensorCount] = tempBus1[i];
      memcpy(publishedAddrs[publishedSensorCount], addrBus1[i], sizeof(DeviceAddress));
      publishedSensorCount++;
    }
#endif
#if OW_PIN2 >= 0
  for (int i = 0; i < numSensorsBus2; i++)
    if (!isnan(tempBus2[i])) {
      publishedTemps[publishedSensorCount] = tempBus2[i];
      memcpy(publishedAddrs[publishedSensorCount], addrBus2[i], sizeof(DeviceAddress));
      publishedSensorCount++;
    }
#endif
#if OW_PIN3 >= 0
  for (int i = 0; i < numSensorsBus3; i++)
    if (!isnan(tempBus3[i])) {
      publishedTemps[publishedSensorCount] = tempBus3[i];
      memcpy(publishedAddrs[publishedSensorCount], addrBus3[i], sizeof(DeviceAddress));
      publishedSensorCount++;
    }
#endif
}
#endif // <-- just added this
#ifndef NO_ONEWIRE
void readAllBuses() {
#if OW_PIN1 >= 0
  sensorsBus1.requestTemperatures();
#endif
#if OW_PIN2 >= 0
  sensorsBus2.requestTemperatures();
#endif
#if OW_PIN3 >= 0
  sensorsBus3.requestTemperatures();
#endif
  delay(2000);

#if OW_PIN1 >= 0
  readBus(sensorsBus1, addrBus1, tempBus1, numSensorsBus1, 1);
#endif
#if OW_PIN2 >= 0
  readBus(sensorsBus2, addrBus2, tempBus2, numSensorsBus2, 2);
#endif
#if OW_PIN3 >= 0
  readBus(sensorsBus3, addrBus3, tempBus3, numSensorsBus3, 3);
#endif

  combineBusesForPublish();
}
#endif

// WiFi and MQTT
void JVWIFISetUp(void) {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.hostname(hostname.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    byte count = 0;
    WiFi.begin(mySSID, myPASSWORD);
    Serial.print("connecting to AP");
    delay(1000);
    while (WiFi.status() != WL_CONNECTED && count < 30) {
      Serial.print(".");
      count++;
      delay(1000);
    }
  }
  rssi = WiFi.RSSI();
  myip = WiFi.localIP().toString();

  // Fire up the file manager automatically if it was compiled in
  initJvFSManager();

}

void setupMQTTClient() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13];
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  subClient = hostname + "-" + macStr;
  LOG_INFO("MQTT Client ID: %s", subClient.c_str());
}

void reconnect() {
  if (client.connected()) return;

  if (WiFi.status() != WL_CONNECTED) {
    JVWIFISetUp();
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) return;
  }

  LOG_INFO("Attempting MQTT connection with client ID: %s", subClient.c_str());

  if (client.connect(subClient.c_str(), mqttID, mqttPASS, LWT_TOPIC, 1, true, LWT_PAYLOAD_OFFLINE)) {
    LOG_INFO("MQTT connected with JSON LWT");
    client.setBufferSize(512);
    client.publish(LWT_TOPIC, LWT_PAYLOAD_ONLINE, true);
  } else {
    LOG_WARN("MQTT connect failed, rc=%d", client.state());
  }
}

void publishOldStyle() {
  doc.clear();

  doc["name"] = hostname;

  ReadSensorInformation();

#if OW_PIN1 >= 0 || OW_PIN2 >= 0 || OW_PIN3 >= 0
  for (int i = 0; i < publishedSensorCount; i++) {
    if (!isnan(publishedTemps[i])) {
      char key[20];
      formatAddress(publishedAddrs[i], key);
      doc[key] = publishedTemps[i];
    }
  }
#endif

  doc["IP"] = myip;
  doc["RSSI"] = rssi;
  doc["Ver"] = combinedVersion;

  serializeJson(doc, msg);
  bool success = client.publish(mqttPublishTopic.c_str(), msg);

  LOG_INFO("publishOldStyle: published to %s → success = %d", mqttPublishTopic.c_str(), success);

  serializeJsonPretty(doc, Serial);
  Serial.println();
  Serial.flush();
}

// Setup
#ifndef NOSETUP
void setup() {
#else
void mysetup() {
#endif
  Serial.begin(serialclock);
  delay(2000);

  myTZ.setLocation(F("America/New_York"));

#ifndef mainver
#define mainver "0.0.0"
#endif
  combinedVersion = String(mainver) + ":" + String(libver);

  LOG_INFO("Booting: %s  %s", hostname.c_str(), combinedVersion.c_str());

  JVWIFISetUp();
  waitForSync();

  Wire.begin(i2cdata, i2cclock);
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(500);

#ifndef NO_ONEWIRE
#if OW_PIN1 >= 0 || OW_PIN2 >= 0 || OW_PIN3 >= 0
#if OW_PIN1 >= 0
  sensorsBus1.begin();
  sensorsBus1.setWaitForConversion(false);
  sensorsBus1.setResolution(12);
  numSensorsBus1 = std::min(static_cast<int>(sensorsBus1.getDeviceCount()), MAX_SENSORS_PER_BUS);
  LOG_INFO("Bus 1 (pin %d): Found %d DS18B20 sensors", (int)OW_PIN1, numSensorsBus1);
  for (int i = 0; i < numSensorsBus1; i++) sensorsBus1.getAddress(addrBus1[i], i);
#endif
#if OW_PIN2 >= 0
  sensorsBus2.begin();
  sensorsBus2.setWaitForConversion(false);
  sensorsBus2.setResolution(12);
  numSensorsBus2 = std::min(static_cast<int>(sensorsBus2.getDeviceCount()), MAX_SENSORS_PER_BUS);
  LOG_INFO("Bus 2 (pin %d): Found %d DS18B20 sensors", (int)OW_PIN2, numSensorsBus2);
  for (int i = 0; i < numSensorsBus2; i++) sensorsBus2.getAddress(addrBus2[i], i);
#endif
#if OW_PIN3 >= 0
  sensorsBus3.begin();
  sensorsBus3.setWaitForConversion(false);
  sensorsBus3.setResolution(12);
  numSensorsBus3 = std::min(static_cast<int>(sensorsBus3.getDeviceCount()), MAX_SENSORS_PER_BUS);
  LOG_INFO("Bus 3 (pin %d): Found %d DS18B20 sensors", (int)OW_PIN3, numSensorsBus3);
  for (int i = 0; i < numSensorsBus3; i++) sensorsBus3.getAddress(addrBus3[i], i);
#endif

  LOG_INFO("Performing initial conversion on DS18B20 buses...");
#if OW_PIN1 >= 0
  sensorsBus1.requestTemperatures();
#endif
#if OW_PIN2 >= 0
  sensorsBus2.requestTemperatures();
#endif
#if OW_PIN3 >= 0
  sensorsBus3.requestTemperatures();
#endif
  delay(2500);

#if OW_PIN1 >= 0
  readBus(sensorsBus1, addrBus1, tempBus1, numSensorsBus1, 1);
#endif
#if OW_PIN2 >= 0
  readBus(sensorsBus2, addrBus2, tempBus2, numSensorsBus2, 2);
#endif
#if OW_PIN3 >= 0
  readBus(sensorsBus3, addrBus3, tempBus3, numSensorsBus3, 3);
#endif

  combineBusesForPublish();

  for (int i = 0; i < MAX_PUBLISHED_SENSORS; i++) {
    lastPublishedTemps[i] = -999.0;
  }
  publishCount = 0;
#endif
#endif

  LOG_INFO("WiFi connected - IP: %s, RSSI: %ld dBm", myip.c_str(), rssi);
}

// Legacy Sensor Functions
#ifdef BAT
void readbat() {
  batv = 0.0;
  batt = analogRead(A0);
  if (batt > 50) {
    batv = double(batt) / battfactor;
  } else {
    LOG_WARN("Battery value not valid");
  }
}
#endif

#ifdef BME
void readbme() {
  if (!bme.begin(0x76)) {
    bmetemp = -999.9;
    bmepres = 0.0;
    bmehum = 0.0;
    LOG_WARN("Could not find a valid BME280 sensor");
    return;
  }

  bmehum = bme.readHumidity();
  double sensor_temperatureC = bme.readTemperature();  // raw BME temp in °C
  bmetemp = sensor_temperatureC * 1.8 + 32;            // for display

  // ==================== Sea Level Pressure Calculation ====================
  double altitude_m = 118.3;  // GPS elevation in meters
  double lapse_rate = 0.0065;
  double altadj = lapse_rate * altitude_m;

  double column_tempC;

  // Prefer outdoor temp when available
  if (outdoorTempC > -100.0 && (millis() - lastOutdoorUpdate < OUTDOOR_TIMEOUT)) {
    column_tempC = outdoorTempC;
    LOG_DEBUG("SLP using outdoor temp: %.2f°C", column_tempC);
  } else {
    column_tempC = sensor_temperatureC;
    LOG_WARN("SLP using BME280 fallback: %.2f°C", column_tempC);
  }

  double T_K = column_tempC + altadj + 273.15;

  double station_Pa = bme.readPressure();
  double sl_Pa = station_Pa * pow((1.0 - (altadj / T_K)), -5.257);

  bmepres = sl_Pa / 3386.39;  // Convert to inHg

  // Optional bias (tune if needed)
  // bmepres += 0.00;

  bmedew = (sensor_temperatureC - (100 - bmehum) / 5.0) * 9.0 / 5.0 + 32;
  bmehi = feels(bmetemp, bmehum);

  // Final rounding
  bmepres = round(bmepres * 100.0) / 100.0;
  bmedew = roundTemp(bmedew);
  bmetemp = roundTemp(bmetemp);
  bmehum = roundTemp(bmehum);
}
#endif

#ifdef BMP
void readbmp() {
  if (!bmp.begin(0x76)) {
    bmptemp = -999.9;
    bmppres = 0.0;
    LOG_WARN("Could not find a valid BMP280 sensor");
  } else {
    double sensor_temperatureC = bmp.readTemperature();
    bmptemp = sensor_temperatureC * 1.8 + 32;
    double altadj = 0.0065 * 110;
    bmppres = bmp.readPressure() * pow((1 - (altadj / (sensor_temperatureC + altadj + 273.15))), -5.257) / 3386;
    bmppres = roundTemp(bmppres);
    bmptemp = roundTemp(bmptemp);
  }
}
#endif

#ifdef DHTPIN
#if DHTPIN >= 0
void readdht() {
  dht11.read();
  dhthum = dht11.getHumidity();
  if (dhthum > 10) {
    double sensor_temperatureC = dht11.getTemperature();
    dhttemp = sensor_temperatureC * 1.8 + 32;
    dhtdew = (sensor_temperatureC - (100 - dhthum) / 5.0) * 9 / 5 + 32;
    dhthi = feels(dhttemp, dhthum);
    dhtdew = roundTemp(dhtdew);
    dhttemp = roundTemp(dhttemp);
    dhthum = roundTemp(dhthum);
  } else {
    LOG_WARN("DHT sensor data invalid or missing.");
    dhttemp = -999.9;
    dhthum = 0.0;
    dhtdew = -999.9;
  }
}
#endif
#endif

// ────────────────────────────────────────────────────────────────
// CONDITIONAL LITTLEFS FILE MANAGER CORE
// ────────────────────────────────────────────────────────────────
#ifdef INCLUDE_FILE_MANAGER

const char JV_FS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>jvcommon FS Manager</title>
<style>body{font-family:Arial,sans-serif;margin:20px;background:#f4f4f9;color:#333;}h2{color:#0076a3;margin-top:0;}.card{background:white;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);margin-bottom:20px;}input[type=file]{margin:10px 0;}button{background:#0076a3;color:white;border:none;padding:10px 15px;border-radius:4px;cursor:pointer;}button:hover{background:#005577;}ul{list-style-type:none;padding:0;}li{padding:10px;background:#eee;margin:5px 0;border-radius:4px;display:flex;justify-content:space-between;align-items:center;}a{color:#0076a3;text-decoration:none;font-weight:bold;}.del{color:red;margin-left:15px;text-decoration:none;}.stats{color:#555;font-size:14px;margin-top:10px;font-weight:bold;}</style></head>
<body><div class='card'><h2>Upload Sketch File</h2><form method='POST' action='/littlefs/upload' enctype='multipart/form-data'><input type='file' name='name'><button type='submit'>Upload File</button></form></div>
<div class='card'><h2>Stored Source Files</h2><div id='filelist'>Loading...</div><div class='stats' id='storageStats'>Calculating...</div></div>
<script>
function loadFiles(){fetch('/littlefs/list').then(res=>res.json()).then(data=>{let html='<ul>';if(data.files.length===0)html+='<li>No files stored yet</li>';data.files.forEach(f=>{html+=`<li><a href="/littlefs/view?path=${encodeURIComponent(f.name)}" download>${f.name.substring(1)} (${f.size} KB)</a><a class="del" href="#" onclick="deleteFile('${f.name}')">&#x2716; Delete</a></li>`;});html+='</ul>';document.getElementById('filelist').innerHTML=html;document.getElementById('storageStats').innerText=`Used: ${data.used} KB / Total: ${data.total} KB (Free: ${data.free} KB)`;});}
function deleteFile(path){if(confirm('Delete '+path+'?')){fetch('/littlefs/delete?path='+encodeURIComponent(path),{method:'POST'}).then(()=>loadFiles());}}window.onload=loadFiles;
</script></body></html>
)rawliteral";

inline void _jvHandleIndex() {
  jvFsServer.send_P(200, "text/html", JV_FS_HTML);
}

inline void _jvHandleList() {
  size_t totalBytes = 0, usedBytes = 0;
#ifdef ESP32
  totalBytes = LittleFS.totalBytes(); usedBytes = LittleFS.usedBytes();
#else
  FSInfo fs_info; LittleFS.info(fs_info); totalBytes = fs_info.totalBytes; usedBytes = fs_info.usedBytes;
#endif
  String json = "{\"total\":" + String(totalBytes / 1024) + ",\"used\":" + String(usedBytes / 1024) + ",\"free\":" + String((totalBytes - usedBytes) / 1024) + ",\"files\":[";
#ifdef ESP32
  File root = LittleFS.open("/"); File file = root.openNextFile();
  while(file){ 
    if(json.endsWith("}")) json += ","; 
    String fname = String(file.name());
    if (!fname.startsWith("/")) fname = "/" + fname; // Ensure leading slash on ESP32
    json += "{\"name\":\"" + fname + "\",\"size\":" + String((float)file.size() / 1024.0, 2) + "}"; 
    file = root.openNextFile(); 
  }
#else
  Dir dir = LittleFS.openDir("/");
  while(dir.next()){ 
    if(json.endsWith("}")) json += ","; 
    String fname = dir.fileName();
    if (!fname.startsWith("/")) fname = "/" + fname; // Safety switch for older core compliance
    json += "{\"name\":\"" + fname + "\",\"size\":" + String((float)dir.fileSize() / 1024.0, 2) + "}"; 
  }
#endif
  json += "]}"; jvFsServer.send(200, "application/json", json);
}


inline void _jvHandleView() {
  if (!jvFsServer.hasArg("path")) return jvFsServer.send(400, "text/plain", "Bad Path");
  String path = jvFsServer.arg("path");
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    jvFsServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    jvFsServer.streamFile(file, "text/plain");
    file.close();
  } else {
    jvFsServer.send(404, "text/plain", "Not Found");
  }
}

inline void _jvHandleUpload() {
  HTTPUpload& upload = jvFsServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    
    // Cross-platform safe leading slash check
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    
    jvUploadFile = LittleFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (jvUploadFile) jvUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (jvUploadFile) jvUploadFile.close();
    jvFsServer.sendHeader("Location", "/littlefs"); jvFsServer.send(303);
  }
}

inline void _jvHandleDelete() { 
  if (!jvFsServer.hasArg("path")) return jvFsServer.send(400, "text/plain", "Bad Path"); 
  String path = jvFsServer.arg("path"); 
  if (LittleFS.exists(path)) { 
    LittleFS.remove(path); 
    jvFsServer.send(200, "text/plain", "Deleted"); 
  } else { 
    jvFsServer.send(404, "text/plain", "Not Found"); 
  } 
} 

inline void initJvFSManager() { 
#ifdef ESP32 
  if (!LittleFS.begin(true)) return; 
#else 
  if (!LittleFS.begin()) { 
    LittleFS.format(); 
    LittleFS.begin(); 
  } 
#endif 
  jvFsServer.on("/littlefs", HTTP_GET, _jvHandleIndex); 
  jvFsServer.on("/littlefs/list", HTTP_GET, _jvHandleList); 
  jvFsServer.on("/littlefs/view", HTTP_GET, _jvHandleView); 
  jvFsServer.on("/littlefs/upload", HTTP_POST, []() {}, _jvHandleUpload); 
  jvFsServer.on("/littlefs/delete", HTTP_POST, _jvHandleDelete); 
  jvFsServer.begin(); 
  Serial.print(F("[jvcommon] FS Manager ready at: http://")); 
  Serial.print(WiFi.localIP()); 
  Serial.println(F("/littlefs")); 
} 

inline void runJvFSManager() { 
  jvFsServer.handleClient(); 
} 

#else // This handles when INCLUDE_FILE_MANAGER is NOT defined

// Empty stubs compile to 0 bytes when file manager is disabled 
inline void initJvFSManager() {} 
inline void runJvFSManager() {} 

#endif // Ends the INCLUDE_FILE_MANAGER guard block

#endif // Ends the master JVCOMMON_H file guard block
