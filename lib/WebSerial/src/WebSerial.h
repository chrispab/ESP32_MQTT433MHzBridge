/*

 */

#ifndef WebSerial_h
#define WebSerial_h

#include <Arduino.h>
//#include <U8g2lib.h>

#define BUFF_SIZE 1024

/**
 */
class WebSerial
{
public:
  WebSerial();

  void println(const char *text);
  void print(const char *text);

  char *getBuffer(void);
  boolean hasData(void);
  void clearBuffer(void);
  String getString(void);

  void refresh(void);
  void wipe(void);

  char consoleBuffer[BUFF_SIZE];
};

#endif
