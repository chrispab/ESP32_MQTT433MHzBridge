#pragma once

#include <Arduino.h>

class LedFader
{
public:
  // constructor
  LedFader(const byte pin,
           const byte pwmChannel,
           const byte minValue,
           const byte maxValue,
           const unsigned long msPerCycle,
           const bool active = true,
           const bool stopWhenOn = false);

  void begin();
  void update();
  void on();
  void off();
  void fullOn();
  void fullOff();

  bool isOn() const;

private:
  const byte pin_;
  const byte pwmChannel_;
  const byte minValue_;
  const byte maxValue_;
  const unsigned long msPerCycle_;

  bool active_;
  bool stopWhenOn_;

  unsigned long startTime_; // when we last changed state
  bool forwards_;           // true is going up, false is going down

}; // end of LedFader class
