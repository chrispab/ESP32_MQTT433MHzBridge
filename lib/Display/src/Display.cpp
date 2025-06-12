// #include <Arduino.h>
// #include <WiFi.h>
#include <Display.h>
// #include <PubSubClient.h>
#include <U8g2lib.h>
// #include "DHT.h"
#include <stdlib.h>  // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
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
char displayLine[DISPLAY_LINES][31];  // 6 lines of n chars +terminator for dispaly store

#include <LedFader.h>
#include <NTPClient.h>
#include <TempSensor.h>
#include <WebSerial.h>
#include <ZoneController.h>

#include "Display.h"
#include <MQTTLib.h>
#include <RF24Lib.h>



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
    // delay(50);
    sendBuffer();
    // delay(50);
}
// clearbufffer and sendto Disply
// redraw the display with contents of displayLine array
void Display::wipe(void) {
    // u8g2.begin();
    clearBuffer();
    // setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++) {
        strcpy(displayLine[i], " ");
        drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    // delay(20);
    sendBuffer();
}
// add-update a line of text in the display text buffer
void Display::writeLine(int lineNumber, const char *lineText) {
    // update a line in the diaplay text buffer
    strcpy(displayLine[lineNumber - 1], lineText);
}

// / static unsigned long lastDisplayUpdateMillis = 0;
static char tempDisplayString[] = "12345678901234567890";
static char humiDisplayString[] = "12345678901234567890";
static char zone1DisplayString[] = "12345678901234567890";
static char zone3DisplayString[] = "12345678901234567890";
static char MQTTDisplayString[] = "12345678901234567890123456789";
static char RF24DisplayString[] = "12345678901234567890";

static char newTempDisplayString[] = "12345678901234567890";
static char newHumiDisplayString[] = "12345678901234567890";
static char newZone1DisplayString[] = "12345678901234567890";
static char newZone3DisplayString[] = "12345678901234567890";
static char newMQTTDisplayString[] = "12345678901234567890123456789";
static char newRF24DisplayString[] = "12345678901234567890";

void Display::updateDisplayData(ZoneController* ZCs, TempSensor &DHT22Sensor,WebSerial &myWebSerial, LedFader &warnLED, NTPClient &timeClient, int displayMode) {
    char justTempString[20];
    char strx[30];
    // blip red led if zones display has changed
    if (
        strcmp(zone1DisplayString, ZCs[0].getDisplayString(newZone1DisplayString)) ||
        strcmp(zone3DisplayString, ZCs[2].getDisplayString(newZone3DisplayString))) {
        warnLED.fullOn();
        delay(1);
        warnLED.fullOff();
    }

    // update the status strings
    DHT22Sensor.getTempDisplayString(newTempDisplayString);
    DHT22Sensor.getHumiDisplayString(newHumiDisplayString);  // get current humi reading
    getMQTTDisplayString(newMQTTDisplayString);
    RF24getDisplayString(newRF24DisplayString);

    // only send data to webserial if any strings have changed

    if (
        strcmp(tempDisplayString, newTempDisplayString) || strcmp(zone1DisplayString, newZone1DisplayString) || strcmp(zone3DisplayString, newZone3DisplayString) || strcmp(MQTTDisplayString, newMQTTDisplayString)
        //|| strcmp(RF24DisplayString, newRF24DisplayString)
        //|| touchedFlag
    ) {
        // myWebSerial.print("Up Time : ");
        // myWebSerial.println(getElapsedTimeStr());
        // myWebSerial.print("T:");
        // myWebSerial.print(timeClient.getFormattedTime().c_str());
        // myWebSerial.print(":");
        // myWebSerial.println("");
        // myWebSerial.println("++ Changes START ++");

        if (strcmp(tempDisplayString, newTempDisplayString)) {

            myWebSerial.print(timeClient.getTimeStr());
            myWebSerial.print(timeClient.getFormattedTime().c_str());

            myWebSerial.print(newTempDisplayString);
            myWebSerial.println("");
        }

        if (strcmp(MQTTDisplayString, newMQTTDisplayString)) {
            // myWebSerial.println("MQTT DISP STRING CHANGED");
            myWebSerial.print(timeClient.getTimeStr());
            myWebSerial.print("-> Tx 433MHz: ");

            myWebSerial.print(newMQTTDisplayString);
            myWebSerial.println("");
        }

        // if (strcmp(RF24DisplayString, newRF24DisplayString)){
        //     myWebSerial.print(newRF24DisplayString);
        // }

        if (strcmp(zone1DisplayString, newZone1DisplayString)) {
            myWebSerial.print(timeClient.getTimeStr());

            myWebSerial.print(newZone1DisplayString);
            myWebSerial.println("");
        }
        if (strcmp(zone3DisplayString, newZone3DisplayString)) {
            myWebSerial.print(timeClient.getTimeStr());

            myWebSerial.print(newZone3DisplayString);
            myWebSerial.println("");
        }

        // copy new data to old vars
        strcpy(tempDisplayString, newTempDisplayString);
        strcpy(humiDisplayString, newHumiDisplayString);
        strcpy(zone1DisplayString, newZone1DisplayString);
        strcpy(zone3DisplayString, newZone3DisplayString);
        strcpy(MQTTDisplayString, newMQTTDisplayString);
        strcpy(RF24DisplayString, newRF24DisplayString);

        // myWebSerial.println(tempDisplayString);
        // myWebSerial.println(MQTTDisplayString);
        // myWebSerial.println(RF24DisplayString);

        // myWebSerial.println(getElapsedTimeStr());
        // myWebSerial.println(timeClient.getFormattedTime().c_str());
        // myWebSerial.println(zone1DisplayString);
        // myWebSerial.println(zone3DisplayString);
        // myWebSerial.println("++ Changes END ++");

        if ((displayMode == BIG_TEMP) || (displayMode == NORMAL)) {
            clearBuffer();
            setFont(BIG_TEMP_FONT);

            // just get the temp bit of displaystring
            // end of string is 'C', need to get string from that pos
            strcpy(justTempString, &tempDisplayString[6]);
            // myDisplay.writeLine(4, justTempString);
            // change the 'o'C to a proper 'degrees C' character
            //  strcat(messageString, "\xb0"); // degree symbol
            justTempString[4] = '\xb0';
            // x,y
            drawStr(0, 38, justTempString);
            // myDisplay.refresh();

            setFont(SYS_FONT);

            // myDisplay.drawStr(0, 47, MQTTDisplayString);
            strcpy(strx, MQTTDisplayString);
            strcat(MQTTDisplayString, "::.");
            strcat(MQTTDisplayString, RF24DisplayString);
            drawStr(0, 47, MQTTDisplayString);
            // timeClient.update();
            strcpy(MQTTDisplayString, strx);

            // Serial.println(timeClient.getFormattedTime());

            drawStr(0, 55, zone1DisplayString);
            drawStr(80, 55, timeClient.getElapsedTimeStr());
            drawStr(0, 63, zone3DisplayString);
            drawStr(80, 63, timeClient.getFormattedTime().c_str());
            sendBuffer();
        } else if (displayMode == MULTI) {
            setFont(SYS_FONT);
            writeLine(1, tempDisplayString);
            writeLine(2, humiDisplayString);
            writeLine(3, MQTTDisplayString);
            writeLine(5, zone1DisplayString);
            writeLine(6, zone3DisplayString);
            refresh();
            myWebSerial.println("");

            DEBUG_PRINTLN("!----------! MULTI Display Refresh");
            DEBUG_PRINTLN(tempDisplayString);
            DEBUG_PRINTLN(humiDisplayString);
            DEBUG_PRINTLN(MQTTDisplayString);
            DEBUG_PRINTLN(zone1DisplayString);
            DEBUG_PRINTLN(zone3DisplayString);
            DEBUG_PRINTLN("^----------^");
        }
    }
}