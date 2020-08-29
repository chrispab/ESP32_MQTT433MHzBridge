#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

// #include <Arduino.h>
#include <config.h>

#include "IOBase.h"

class LightSensor : public IOBase {
   private:
    //bool state;
    int pin;
    bool newLevelFlag;
    u_int currentLevel;
    u_int previousLevel = 500;
    u_int readIntervalMillis = 30000;  //min interval between reading sensor in ms
    u_int lastReadMillis = -30000;

    //hysteresis and state related properties
    //u_int thresholdLevel = 600;
    int lowerThresholdLevel = LIGHT_SENSOR_LOWER_THRESHOLD;
    int upperThresholdLevel = LIGHT_SENSOR_UPPER_THRESHOLD;
    //bool trigger = false;

    //int currentLevel;
    //int lowerHys = targetLevel - lowerLevel;
    //int upperHys = targetLevel + upperLevel;

   public:
    LightSensor(uint8_t pin);
    u_int getLevel();
    u_int clearNewLevelFlag();
    bool hasNewLevel();
    void setThresholdLevel();
    int getLightSensor();
    bool getState();
};

#endif
