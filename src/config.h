#ifndef __SETTINGS_H
#define __SETTINGS_H

//#define DEBUG_WSERIAL

#define RELEASE



#define LIGHT_SENSOR_LOWER_THRESHOLD 1500
#define LIGHT_SENSOR_UPPER_THRESHOLD 1900


// #define HEART_BEAT_TIME 1000
//#define LIGHT_SENSOR_UPPER_THRESHOLD 1900

#define ZONE_WAIT_BEFORE_FLAG_AWAY 100   //in seconds time window to wait before classed as zone gone away
#define ZONE_HEARTBEAT_TIMEOUT_MS (1000UL * 420UL)   //max millisces to wait if no ack from pi before power cycling pi
#define ZONE_COLD_BOOT_TIME_MS  (1000UL * 180UL)     //estimated time for a zone controller to boot from power cycle reset

#define ESP32_WATCHDOG_TIMEOUT_SECS 60
#define ESP32_WATCHDOG_RESET_INTERVAL_SECS 30


#endif
