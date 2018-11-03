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
    bool processTouchPad(void);

  int getValue();

private:
  int _pin;
  bool state;
  int rawValue;
  int lastFilteredVal;
  int filteredVal;
  int filterConstant; // 2 ishalf, 4 is quarter etc
  int touchThreshold;
  int newTouchValue;
  unsigned long lastTouchReadMillis;
  unsigned long touchReadInterval;
};

#endif
