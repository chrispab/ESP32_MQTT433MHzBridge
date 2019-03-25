/*

 */

#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <U8g2lib.h>


/**
 */
class Display : public U8G2 {
  public:
    Display(const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE,
            uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE);

    void writeLine(int lineNumber, const char *lineText);
    void refresh(void);
    void wipe(void);
};

#endif
