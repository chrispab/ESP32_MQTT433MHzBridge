#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include <Arduino.h>
#include "IOBase.h"

class LightSensor : public IOBase
{
private:
  //bool state;
  int pin;

public:
  LightSensor(uint8_t pin);
  // bool getState();
  bool sampleState();
  int getLevel();
  bool hasNewLevel();
  void setThresholdLevel();

  int getLightSensor();
};

#endif
