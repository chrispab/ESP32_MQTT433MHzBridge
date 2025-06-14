#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

// #include <Arduino.h>
#include <config.h>

#include "IOBase.h"
#include <PubSubClient.h>

class LightSensor : public IOBase {
   private:
    // bool state;
    int pin;
    bool newLevelFlag;
    u_int currentLevel;
    u_int previousLevel = 500;
    u_int readIntervalMillis = LIGHT_SENSOR_READ_INTERVAL;  // min interval between reading sensor in ms
    u_int lastReadMillis = 0 - LIGHT_SENSOR_READ_INTERVAL;

    // hysteresis and state related properties
    // u_int thresholdLevel = 600;
    int lowerThresholdLevel = LIGHT_SENSOR_LOWER_THRESHOLD;
    int upperThresholdLevel = LIGHT_SENSOR_UPPER_THRESHOLD;

    // bool trigger = false;

    // int currentLevel;
    // int lowerHys = targetLevel - lowerLevel;
    // int upperHys = targetLevel + upperLevel;

   public:
    LightSensor(uint8_t pin);
    // u_int readLevel();
    void clearNewLevelFlag();
    u_int readLevelIfDue();
    bool hasNewLevel();
    // void setThresholdLevel();
    // int getLightSensor();
    bool updateState();

    bool getState();
    int getLevel();
    void process(PubSubClient& MQTTclient);
    // void process(PubSubClient MQTTclient);
    // void process();
    // int getLowerThresholdLevel();
    // int getUpperThresholdLevel();
};

#endif
