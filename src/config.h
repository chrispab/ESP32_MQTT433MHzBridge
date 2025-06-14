#ifndef __CONFIG_H
#define __CONFIG_H
#include "debug.h"

#pragma once

// Project version
constexpr auto PROJECT_VERSION = "1.0.0";
constexpr auto SW_VERSION = "V5.6:refactor1";
constexpr auto TITLE_LINE1 = "     ESP32";
constexpr auto TITLE_LINE2 = "MQTT 433MhZ Bridge";
constexpr auto TITLE_LINE3 = "Zone RF24 Dog";
constexpr auto TITLE_LINE4 = "Temp Light PIR";
constexpr auto TITLE_LINE5 = SW_VERSION;
constexpr auto TITLE_LINE6 = "mqtt rssi quality";

// WiFi settings (non-secret, e.g. hostname)
constexpr auto WIFI_HOSTNAME = "esp32-mqtt-bridge";

// MQTT settings
constexpr auto MQTT_BROKER = "192.168.1.100";
constexpr int  MQTT_PORT   = 1883;
constexpr auto MQTT_CLIENT_ID = "esp32Bridge";

// Timing intervals (ms)
constexpr unsigned long MQTT_RECONNECT_INTERVAL = 10000;
constexpr unsigned long SENSOR_POLL_INTERVAL   = 500;

// Feature toggles
constexpr bool ENABLE_DISPLAY = true;
constexpr bool ENABLE_PIR     = true;

// Add more as needed...

// time stuff
constexpr unsigned long NTP_OFFSET = 0;            // 60 * 60      // In seconds, 0 for GMT, 60*60 for BST
constexpr unsigned long NTP_INTERVAL = 60 * 1000;  // In miliseconds
constexpr auto NTP_ADDRESS = "europe.pool.ntp.org";



constexpr unsigned long PIR_READ_INTERVAL = 999;  // ms, minimum interval between reading sensor

constexpr unsigned long WIFI_CONNECTION_CHECK_INTERVAL = 30000; // ms, interval to check wifi connection
//#define DEBUG_WSERIAL

// - Use `extern` in headers for global variables.
// - Provide the actual definition in a single `.cpp` file.
extern char publishLightStateTopic[];
extern char publishLightLevelTopic[];
extern char publishPIRStateTopic[];


constexpr auto HEART_BEAT_TIME = 500;

// #define RELEASE


constexpr auto LIGHT_SENSOR_READ_INTERVAL = 30000;  // ms

constexpr auto LIGHT_SENSOR_LOWER_THRESHOLD = 1500;
constexpr auto LIGHT_SENSOR_UPPER_THRESHOLD = 1900;


// #define HEART_BEAT_TIME 1000
//#define LIGHT_SENSOR_UPPER_THRESHOLD 1900

constexpr auto ZONE_WAIT_BEFORE_FLAG_AWAY = 100;   //in seconds time window to wait before classed as zone gone away
constexpr auto ZONE_HEARTBEAT_TIMEOUT_MS = (1000UL * 420UL);   //max millisces to wait if no ack from pi before power cycling pi
constexpr auto ZONE_COLD_BOOT_TIME_MS = (1000UL * 180UL);     //estimated time for a zone controller to boot from power cycle reset

constexpr auto ESP32_WATCHDOG_TIMEOUT_SECS = 60;
constexpr auto ESP32_WATCHDOG_RESET_INTERVAL_SECS = 30;

constexpr auto MQTT_LAST_OCTET = 100;

#endif
