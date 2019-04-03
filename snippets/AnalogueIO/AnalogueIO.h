#ifndef __AIOBASE_H
#define __AIOBASE_H

#include <Arduino.h>

class IOBase
{
protected:
  int value;
  bool newValueFlag;
  int defaultValue;
  uint8_t pin;
  long prevValueChangeMillis;
  //long onMillis;
  //long offMillis;
  //float upperOffset;
  //float lowerOffset;

public:
  AnalogueIO();
  bool getValue();
  void setValue(bool);
  bool readValue();
  bool hasNewValue();
  //void setOnMillis(long onMillis);
  //void setOffMillis(long offMillis);
};

#endif
