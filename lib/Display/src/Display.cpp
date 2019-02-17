// #include <Arduino.h>
// #include <WiFi.h>
#include <Display.h>
// #include <PubSubClient.h>
#include <U8g2lib.h>
// #include "DHT.h"
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
// #include <RF24.h>

// forward decs

// OLED display stuff
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
// /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping
#define LINE_HIEGHT 10
#define XPIX 128
#define YPIX 64
#define DISPLAY_LINES 6
#define CHAR_WIDTH 8
char displayLine[DISPLAY_LINES][31]; // 6 lines of n chars +terminator for dispaly store

#include "Display.h"

Display::Display(const u8g2_cb_t *rotation, uint8_t reset, uint8_t clock,
                 uint8_t data)
    : U8G2() {
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
    u8x8_SetPin_HW_I2C(getU8x8(), reset, clock, data);
}

// redraw the display with contents of displayLine array
void Display::refresh(void) {
    // u8g2.begin();
    clearBuffer();
    // setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++) {
        drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    //delay(50);
    sendBuffer();
    //delay(50);
}
// clearbufffer and sendto Disply
//redraw the display with contents of displayLine array
void Display::wipe(void) {
    // u8g2.begin();
    clearBuffer();
    // setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++) {
        strcpy(displayLine[i], " ");
        drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    //delay(20);
    sendBuffer();
}
// add-update a line of text in the display text buffer
void Display::writeLine(int lineNumber, const char *lineText) {
    // update a line in the diaplay text buffer
    strcpy(displayLine[lineNumber - 1], lineText);
}
