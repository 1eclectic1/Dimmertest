#ifndef CONFIG_H
#define CONFIG_H

// Define behavior when MAX_LIMITS is set to 0
// If true, a channel with MAX_LIMITS = 0 will always be off (0% brightness).
// If false, a channel with MAX_LIMITS = 0 will allow full PWM_RANGE (1023) as its max, effectively disabling individual channel scaling for that channel.
#define MAX_LIMITS_ZERO_MEANS_OFF true 

#endif