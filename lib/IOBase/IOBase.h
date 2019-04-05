#ifndef __IOBASE_H
#define __IOBASE_H

#include <Arduino.h>

class IOBase
{
protected:
  bool state;
  bool newStateFlag;
  bool defaultState;
  uint8_t pin;
  long prevStateChangeMillis;
  long onMillis;
  long offMillis;
  float upperOffset;
  float lowerOffset;

public:
  IOBase();
  bool getState();
  void setState(bool);
  bool clearNewStateFlag();
//  int getLevel();

  bool hasNewState();
  void setOnMillis(long onMillis);
  void setOffMillis(long offMillis);
};

#endif
