/*

 */

#ifndef WebSerial_h
#define WebSerial_h

#include <Arduino.h>
//#include <U8g2lib.h>

#define BUFF_SIZE 512

/**
 */
class WebSerial {
  public:

    WebSerial();

    void println(const char *text);
    char* getBuffer(void);

    void refresh(void);
    void wipe(void);

    char consoleBuffer[BUFF_SIZE];
};

#endif
