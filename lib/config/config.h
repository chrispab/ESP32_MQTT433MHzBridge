#ifndef __SETTINGS_H
#define __SETTINGS_H

#define LIGHT_SENSOR_LOWER_THRESHOLD 1500
#define LIGHT_SENSOR_UPPER_THRESHOLD 1900


// #define HEART_BEAT_TIME 1000
//#define LIGHT_SENSOR_UPPER_THRESHOLD 1900

#define ZONE_HEARTBEAT_TIMEOUT_MS (1000UL * 420UL)   //max millisces to wait if no ack from pi before power cycling pi
#define ZONE_COLD_BOOT_TIME_MS  (1000UL * 180UL)     //estimated time for a zone controller to boot from power cycle reset



#endif
