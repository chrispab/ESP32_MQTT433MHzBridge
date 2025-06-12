#include "SupportLib.h"

#include "debug.h"
// extern displayModes displayMode;
// enum displayModes displayMode;

#include "TempSensor.h"
#include "pins.h"
extern TempSensor DHT22Sensor;

#include "ZoneController.h"
extern ZoneController ZCs[];

#include "WebSerial.h"
extern WebSerial myWebSerial;

#include "MQTTLib.h"
extern char *getMQTTDisplayString(char *MQTTDisplayString);

#include <NTPClient.h>
extern NTPClient timeClient;

// #include "Display.h"
// extern Display myDisplay;

boolean touchedFlag = false;

// /**
//  * @brief Returns the current formatted time as a string with a suffix.
//  *
//  * This function retrieves the current time from the global `timeClient` object,
//  * formats it as a string, and appends ": " to the end. The result is stored in
//  * a static buffer, so each call will overwrite the previous value.
//  *
//  * @note The returned pointer refers to a static buffer which is overwritten on each call.
//  * @note The function ensures that the buffer is not overrun by carefully managing string lengths.
//  *
//  * @return A pointer to a static character array containing the formatted time string with a ": " suffix.
//  */
// char *getTimeStr() {
//     static char timeStr[20];
//     char formatted[8]; 
//     strcpy(formatted, timeClient.getFormattedTime().c_str());
//     // DEBUG_PRINT("Formatted time: ");
//     // DEBUG_PRINTLN(formatted);
//     // Defensive: ensure buffer is not overrun
//     strncpy(timeStr, formatted, sizeof(timeStr) - 3); // leave space for ": " and null
//     // DEBUG_PRINT("Time string: ");
//     // DEBUG_PRINTLN(timeStr);
//     timeStr[sizeof(timeStr) - 3] = '\0';
//     strncat(timeStr, ": ", sizeof(timeStr) - strlen(timeStr) - 1);
//     return timeStr;
// }


/**
 * @brief Get the Elapsed Time since power on Str pointer
 *
 * @return char*
 */
// char *getElapsedTimeStr() {
//     static char elapsedTimeStr[20] = "Test Time";
//     static unsigned long startMillis = millis();

//     unsigned long rawTime = (millis() - startMillis) / 1000;
//     unsigned long hours = (rawTime) / 3600;
//     unsigned long minutes = (rawTime % 3600) / 60;
//     unsigned long seconds = rawTime % 60;
//     // Format with snprintf for buffer safety
//     snprintf(elapsedTimeStr, sizeof(elapsedTimeStr), "%02lu:%02lu:%02lu", hours, minutes, seconds);
//     return elapsedTimeStr;
// }

extern bool touchedFlag;

/**
 * @brief
 *
 */
#include "LedFader.h"
extern LedFader warnLED;
char *RF24getDisplayString(char *statusMessage);

// // static unsigned long lastDisplayUpdateMillis = 0;
// static char tempDisplayString[] = "12345678901234567890";
// static char humiDisplayString[] = "12345678901234567890";
// static char zone1DisplayString[] = "12345678901234567890";
// static char zone3DisplayString[] = "12345678901234567890";
// static char MQTTDisplayString[] = "12345678901234567890123456789";
// static char RF24DisplayString[] = "12345678901234567890";

// static char newTempDisplayString[] = "12345678901234567890";
// static char newHumiDisplayString[] = "12345678901234567890";
// static char newZone1DisplayString[] = "12345678901234567890";
// static char newZone3DisplayString[] = "12345678901234567890";
// static char newMQTTDisplayString[] = "12345678901234567890123456789";
// static char newRF24DisplayString[] = "12345678901234567890";

// void updateDisplayData() {
//     char justTempString[20];
//     char strx[30];
//     // blip red led if zones display has changed
//     if (
//         strcmp(zone1DisplayString, ZCs[0].getDisplayString(newZone1DisplayString)) ||
//         strcmp(zone3DisplayString, ZCs[2].getDisplayString(newZone3DisplayString))) {
//         warnLED.fullOn();
//         delay(1);
//         warnLED.fullOff();
//     }

//     // update the status strings
//     DHT22Sensor.getTempDisplayString(newTempDisplayString);
//     DHT22Sensor.getHumiDisplayString(newHumiDisplayString);  // get current humi reading
//     getMQTTDisplayString(newMQTTDisplayString);
//     RF24getDisplayString(newRF24DisplayString);

//     // only send data to webserial if any strings have changed

//     if (
//         strcmp(tempDisplayString, newTempDisplayString) || strcmp(zone1DisplayString, newZone1DisplayString) || strcmp(zone3DisplayString, newZone3DisplayString) || strcmp(MQTTDisplayString, newMQTTDisplayString)
//         //|| strcmp(RF24DisplayString, newRF24DisplayString)
//         //|| touchedFlag
//     ) {
//         // myWebSerial.print("Up Time : ");
//         // myWebSerial.println(getElapsedTimeStr());
//         // myWebSerial.print("T:");
//         // myWebSerial.print(timeClient.getFormattedTime().c_str());
//         // myWebSerial.print(":");
//         // myWebSerial.println("");
//         // myWebSerial.println("++ Changes START ++");

//         if (strcmp(tempDisplayString, newTempDisplayString)) {
//             myWebSerial.print(getTimeStr());

//             myWebSerial.print(newTempDisplayString);
//             myWebSerial.println("");
//         }

//         if (strcmp(MQTTDisplayString, newMQTTDisplayString)) {
//             // myWebSerial.println("MQTT DISP STRING CHANGED");
//             myWebSerial.print(getTimeStr());
//             myWebSerial.print("-> Tx 433MHz: ");

//             myWebSerial.print(newMQTTDisplayString);
//             myWebSerial.println("");
//         }

//         // if (strcmp(RF24DisplayString, newRF24DisplayString)){
//         //     myWebSerial.print(newRF24DisplayString);
//         // }

//         if (strcmp(zone1DisplayString, newZone1DisplayString)) {
//             myWebSerial.print(getTimeStr());

//             myWebSerial.print(newZone1DisplayString);
//             myWebSerial.println("");
//         }
//         if (strcmp(zone3DisplayString, newZone3DisplayString)) {
//             myWebSerial.print(getTimeStr());

//             myWebSerial.print(newZone3DisplayString);
//             myWebSerial.println("");
//         }

//         // copy new data to old vars
//         strcpy(tempDisplayString, newTempDisplayString);
//         strcpy(humiDisplayString, newHumiDisplayString);
//         strcpy(zone1DisplayString, newZone1DisplayString);
//         strcpy(zone3DisplayString, newZone3DisplayString);
//         strcpy(MQTTDisplayString, newMQTTDisplayString);
//         strcpy(RF24DisplayString, newRF24DisplayString);

//         // myWebSerial.println(tempDisplayString);
//         // myWebSerial.println(MQTTDisplayString);
//         // myWebSerial.println(RF24DisplayString);

//         // myWebSerial.println(getElapsedTimeStr());
//         // myWebSerial.println(timeClient.getFormattedTime().c_str());
//         // myWebSerial.println(zone1DisplayString);
//         // myWebSerial.println(zone3DisplayString);
//         // myWebSerial.println("++ Changes END ++");

//         if ((displayMode == BIG_TEMP) || (displayMode == NORMAL)) {
//             myDisplay.clearBuffer();
//             myDisplay.setFont(BIG_TEMP_FONT);

//             // just get the temp bit of displaystring
//             // end of string is 'C', need to get string from that pos
//             strcpy(justTempString, &tempDisplayString[6]);
//             // myDisplay.writeLine(4, justTempString);
//             // change the 'o'C to a proper 'degrees C' character
//             //  strcat(messageString, "\xb0"); // degree symbol
//             justTempString[4] = '\xb0';
//             // x,y
//             myDisplay.drawStr(0, 38, justTempString);
//             // myDisplay.refresh();

//             myDisplay.setFont(SYS_FONT);

//             // myDisplay.drawStr(0, 47, MQTTDisplayString);
//             strcpy(strx, MQTTDisplayString);
//             strcat(MQTTDisplayString, "::.");
//             strcat(MQTTDisplayString, RF24DisplayString);
//             myDisplay.drawStr(0, 47, MQTTDisplayString);
//             // timeClient.update();
//             strcpy(MQTTDisplayString, strx);

//             // Serial.println(timeClient.getFormattedTime());

//             myDisplay.drawStr(0, 55, zone1DisplayString);
//             myDisplay.drawStr(80, 55, getElapsedTimeStr());
//             myDisplay.drawStr(0, 63, zone3DisplayString);
//             myDisplay.drawStr(80, 63, timeClient.getFormattedTime().c_str());
//             myDisplay.sendBuffer();
//         } else if (displayMode == MULTI) {
//             myDisplay.setFont(SYS_FONT);
//             myDisplay.writeLine(1, tempDisplayString);
//             myDisplay.writeLine(2, humiDisplayString);
//             myDisplay.writeLine(3, MQTTDisplayString);
//             myDisplay.writeLine(5, zone1DisplayString);
//             myDisplay.writeLine(6, zone3DisplayString);
//             myDisplay.refresh();
//             myWebSerial.println("");

//             DEBUG_PRINTLN("!----------! MULTI Display Refresh");
//             DEBUG_PRINTLN(tempDisplayString);
//             DEBUG_PRINTLN(humiDisplayString);
//             DEBUG_PRINTLN(MQTTDisplayString);
//             DEBUG_PRINTLN(zone1DisplayString);
//             DEBUG_PRINTLN(zone3DisplayString);
//             DEBUG_PRINTLN("^----------^");
//         }
//     }
// }

extern unsigned long currentMillis;
extern unsigned long previousConnCheckMillis;
extern unsigned long intervalConnCheckMillis;
// extern char* publishLWTTopic;

#include <PubSubClient.h>

#include "WiFiLib.h"
extern PubSubClient MQTTclient;

char* stringToPrint(const char* literal, const char* value) {
    static char buffer[64]; // Adjust size as needed
    buffer[0] = '\0'; // Ensure buffer is empty
    strncat(buffer, literal, sizeof(buffer) - 1);
    strncat(buffer, value, sizeof(buffer) - strlen(buffer) - 1);
    return buffer;
}

