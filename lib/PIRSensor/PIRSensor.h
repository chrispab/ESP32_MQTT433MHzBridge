#ifndef __PIRSENSOR_H
#define __PIRSENSOR_H

#include <Arduino.h>
#include "IOBase.h"

class PIRSensor : public IOBase
{
private:
  //bool state;
  int pin;
  //bool newLevelFlag;
  bool currentState;
  //u_int previousLevel = 500;
  u_int readIntervalMillis = 5000; //min interval between reading sensor in ms
  u_int lastReadMillis = -5000;

public:
  PIRSensor(uint8_t pin);

  //int getPIRSensor();
  bool readState(); // read the state of the PIR sensor
  // bool getState(); // get the state of the PIR sensor
};

#endif
