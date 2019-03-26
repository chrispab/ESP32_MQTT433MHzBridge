// #include <Arduino.h>
// #include <WiFi.h>
#include <NewRemoteTransmitter.h>

#include <My433Transmitter.h>
// #include <PubSubClient.h>
// #include <U8g2lib.h>
// #include "DHT.h"
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
#include "../../src/socketIDFS.h"

// #include <RF24.h>

// forward decs

// OLED display stuff
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
// /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping
// #define LINE_HIEGHT 10
// #define XPIX 128
// #define YPIX 64
// #define DISPLAY_LINES 6
// #define CHAR_WIDTH 8
//char displayLine[DISPLAY_LINES][31]; // 6 lines of n chars +terminator for dispaly store

//#include "My433Transmitter.h"

My433Transmitter::My433Transmitter(unsigned long address, byte pin, unsigned int periodusec, byte repeats) :
    NewRemoteTransmitter::NewRemoteTransmitter(address, pin, periodusec, repeats){;
}
/**
 * @brief 0-15, 0,1 mqtt funct to operate a remote power socket
 *          only used by mqtt function
 * 
 * @param socketID 0-15, correspond to socketNumber 1-16
 * @param state 
 */
#include "LedFader.h"
extern LedFader warnLED;
#include "My433Transmitter.h"
extern My433Transmitter transmitter;
#include "WebSerial.h"
extern WebSerial myWebSerial;
void My433Transmitter::operateSocket(uint8_t socketID, uint8_t state)
{
    // this is a blocking routine !!!!!!!
    char msg[60] = "=> Operate Socket: ";
    char buff[10];

    // strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (socketID + 1));
    strcat(msg, buff);
    strcat(msg, "-");

    //add socket item name
    strcat(msg, socketIDFunctionStrings[socketID]);

    // u8g2.setCursor(55, 40);
    if (state == 0)
    {
        strcat(msg, " - OFF");
    }
    else
    {
        strcat(msg, " - ON");
    }
    //Serial.println(msg);
    myWebSerial.println(msg);

    warnLED.fullOn();

    // myWebSerial.println("operate skt scktID..");
    // sprintf(buff, "%d", (socketID));
    // myWebSerial.println(buff);
    //     myWebSerial.println("operate skt newstate..");
    // sprintf(buff, "%d", (state));
    // myWebSerial.println(buff);

    this->sendUnit(socketID, state);
    warnLED.fullOff(); //}
}
// // redraw the display with contents of displayLine array
// void Display::refresh(void) {
//     // u8g2.begin();
//     clearBuffer();
//     // setFont(u8g2_font_8x13_tf);
//     for (int i = 0; i < DISPLAY_LINES; i++) {
//         drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
//     }
//     //delay(50);
//     sendBuffer();
//     //delay(50);
// }
// // clearbufffer and sendto Disply
// //redraw the display with contents of displayLine array
// void Display::wipe(void) {
//     // u8g2.begin();
//     clearBuffer();
//     // setFont(u8g2_font_8x13_tf);
//     for (int i = 0; i < DISPLAY_LINES; i++) {
//         strcpy(displayLine[i], " ");
//         drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
//     }
//     //delay(20);
//     sendBuffer();
// }
// // add-update a line of text in the display text buffer
// void Display::writeLine(int lineNumber, const char *lineText) {
//     // update a line in the diaplay text buffer
//     strcpy(displayLine[lineNumber - 1], lineText);
// }
