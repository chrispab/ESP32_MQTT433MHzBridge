#ifndef __PIRSENSOR_H
#define __PIRSENSOR_H

#include <Arduino.h>
#include "IOBase.h"

class PIRSensor : public IOBase
{
private:
  //bool state;
  int pin;
  bool newLevelFlag;
  u_int currentLevel;
  u_int previousLevel = 500;
  u_int readIntervalMillis = 5000; //min interval between reading sensor in ms
  u_int lastReadMillis = -5000;

public:
  PIRSensor(uint8_t pin);
  u_int getLevel();
  u_int clearNewLevelFlag();
  bool hasNewLevel();
  void setThresholdLevel();
  int getPIRSensor();
  bool getState();
};

#endif
