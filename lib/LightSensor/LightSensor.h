#ifndef __LIGHT_H
#define __LIGHT_H

#include <Arduino.h>
#include "IOBase.h"

class Light : public IOBase
{
private:
  //bool state;
  int pin;

public:
  Light(uint8_t pin);
  // bool getState();
  //bool State();
  int getLevel();
  void setThresholdLevel();

  int getLightSensor();
};

#endif
