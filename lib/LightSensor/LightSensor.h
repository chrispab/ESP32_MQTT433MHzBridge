#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include <Arduino.h>
#include "IOBase.h"

class LightSensor : public IOBase
{
private:
  //bool state;
  int pin;
  bool newLevelFlag;
  u_int currentLevel;
  u_int previousLevel = 500;
  u_int readIntervalMillis = 5000; //min interval between reading sensor in ms
  u_int lastReadMillis = -5000;

  //hysteresis and state related properties
  //u_int thresholdLevel = 600;
  int lowerThresholdLevel = 1200;
  int upperThresholdLevel = 2100;
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
