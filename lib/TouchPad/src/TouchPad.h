#ifndef TOUCHPAD_H
#define TOUCHPAD_H

/**
 * @brief 
 * 
 */
class TouchPad
{
public:
  TouchPad(int pin);
  bool getState(void);

private:
  int _pin;
  int lastFilteredVal;
  int filteredVal;
  int filterConstant; // 2 ishalf, 4 is quarter etc
  int touchThreshold;
  int newTouchValue;
  unsigned long lastTouchReadMillis;
  unsigned long touchReadInterval;
};

#endif
