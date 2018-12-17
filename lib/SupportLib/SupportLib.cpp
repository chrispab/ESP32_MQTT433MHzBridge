#include "SupportLib.h"
//extern displayModes displayMode;
enum displayModes displayMode;

#include "pins.h"

#include "TempSensor.h"
extern TempSensor DHT22Sensor;

#include "ZoneController.h"
extern ZoneController ZCs[];

#include "WebSerial.h"
extern WebSerial myWebSerial;

#include "MQTTLib.h"
extern char *getMQTTDisplayString(char *MQTTDisplayString);

#include <NTPClient.h>
extern NTPClient timeClient;

#include "Display.h"
extern Display myDisplay;

boolean touchedFlag = false;

/**
 * @brief scan touchpins and check for a touch
 * 
 * @return boolean true if touch detected
 */
// boolean processTouchPads(void)
// {
//     static int lastFilteredVal = 54;
//     static int filteredVal = 54;
//     const int filterConstant = 3; // 2 ishalf, 4 is quarter etc
//     int touchThreshold = 45;
//     int newTouchValue = 100;

//     static unsigned long lastTouchReadMillis = millis();
//     unsigned long touchReadInterval = 25;

//     if ((millis() - lastTouchReadMillis) >= touchReadInterval)
//     {
//         //newTouchValue = touchRead(TOUCH_PIN);
//         newTouchValue = touchRead(TOUCH_SENSOR_2);

//         lastTouchReadMillis = millis();

//         //! software addition filter here to even out spurious readings
//         int diff;
//         //filteredVal = filteredVal + (filteredVal )
//         if (newTouchValue > filteredVal) //if going up - add onto filteredval
//         {
//             diff = (newTouchValue - filteredVal);
//             filteredVal = filteredVal + (diff / filterConstant);
//         }
//         if (newTouchValue < filteredVal) //if going down - subtract  from filteredval
//         {
//             diff = (filteredVal - newTouchValue);
//             filteredVal = filteredVal - (diff / filterConstant);
//         }

//         // !! detect edges
//         if ((lastFilteredVal > touchThreshold) && (filteredVal < touchThreshold))
//         { //high to low edge finger on
//             Serial.print("$$$]_ Touch Val = ");
//             Serial.println(newTouchValue);
//             Serial.print("filtered Val = ");
//             Serial.println(filteredVal);

//             //displayMode = MULTI;
//             lastFilteredVal = filteredVal;
//             //lastTouchReadMillis = millis();

//             return true;
//         }
//         if ((lastFilteredVal < touchThreshold) && (filteredVal > touchThreshold))
//         { //low to high edge finger off
//             Serial.print("$$$_[ Touch Val = ");
//             Serial.println(newTouchValue);
//             Serial.print("filtered Val = ");
//             Serial.println(filteredVal);
//             //displayMode = BIG_TEMP;
//             lastFilteredVal = filteredVal;
//             //lastTouchReadMillis = millis();

//             return false;
//         }
//         lastFilteredVal = filteredVal;

//         return false; // no edge detected
//     }
//     return false; // no edge detected
//}

/**
 * @brief Get the Elapsed Time Str pointer
 * 
 * @return char* 
 */
char *getElapsedTimeStr()
{
    static char elapsedTimeStr[20] = "Test Time";
    static unsigned long startMillis = millis();

    unsigned long rawTime = (millis() - startMillis) / 1000;

    unsigned long hours = (rawTime) / 3600;
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

    unsigned long minutes = (rawTime % 3600) / 60;
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    unsigned long seconds = rawTime % 60;
    String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

    strcpy(elapsedTimeStr, (hoursStr + ":" + minuteStr + ":" + secondStr).c_str());

    return elapsedTimeStr;
}

extern bool touchedFlag;

/**
 * @brief 
 * 
 */
#include "LedFader.h"
extern LedFader warnLED;

void updateDisplayData()
{
    // static unsigned long lastDisplayUpdateMillis = 0;
    static char tempDisplayString[20];
    static char humiDisplayString[20];
    static char zone1DisplayString[20];
    static char zone3DisplayString[20];
    static char MQTTDisplayString[20];

    static char newTempDisplayString[20];
    static char newHumiDisplayString[20];
    static char newZone1DisplayString[20];
    static char newZone3DisplayString[20];
    static char newMQTTDisplayString[20];

    char justTempString[20];

    //blip red led if zone display has changed
    if (
        strcmp(zone1DisplayString, ZCs[0].getDisplayString(newZone1DisplayString)) ||
        strcmp(zone3DisplayString, ZCs[2].getDisplayString(newZone3DisplayString))
        )
    {
        warnLED.fullOn();
        delay(10);
        warnLED.fullOff();
    }
    // only update screen if status messages or touch sensor is active/has changed
    // compare new display data to previous display data
    // if different - update the actual OLED display and previous
    // if any non zero then data has changed
    if (strcmp(tempDisplayString,
               DHT22Sensor.getTempDisplayString(newTempDisplayString)) ||
        touchedFlag ||
        strcmp(zone1DisplayString, ZCs[0].getDisplayString(newZone1DisplayString)) ||
        strcmp(zone3DisplayString,
               ZCs[2].getDisplayString(newZone3DisplayString)) ||
        strcmp(MQTTDisplayString, getMQTTDisplayString(newMQTTDisplayString)))
    {
        DHT22Sensor.getHumiDisplayString(newHumiDisplayString); //get current humi reading
        // copy new data to old vars
        strcpy(tempDisplayString, newTempDisplayString);
        strcpy(humiDisplayString, newHumiDisplayString);
        strcpy(zone1DisplayString, newZone1DisplayString);
        strcpy(zone3DisplayString, newZone3DisplayString);
        strcpy(MQTTDisplayString, newMQTTDisplayString);

        myWebSerial.println("!----------! BIG_TEMP Display Refresh");
        myWebSerial.println(tempDisplayString);
        myWebSerial.println(MQTTDisplayString);
        myWebSerial.println(getElapsedTimeStr());
        myWebSerial.println(timeClient.getFormattedTime().c_str());
        myWebSerial.println(zone1DisplayString);
        myWebSerial.println(zone3DisplayString);
        myWebSerial.println("^----------^");

        if (displayMode == NORMAL)
        {
            // TODO
        }
        else if (displayMode == BIG_TEMP)
        {
            myDisplay.clearBuffer();
            myDisplay.setFont(BIG_TEMP_FONT);

            //just get the temp bit of displaystring
            //end of string is 'C', need to get string from that pos
            strcpy(justTempString, &tempDisplayString[6]);
            //myDisplay.writeLine(4, justTempString);
            //change the 'o'C to a proper 'degrees C' character
            // strcat(messageString, "\xb0"); // degree symbol
            justTempString[4] = '\xb0';
            myDisplay.drawStr(0, 38, justTempString);
            //myDisplay.refresh();

            myDisplay.setFont(SYS_FONT);
            myDisplay.drawStr(0, 47, MQTTDisplayString);
            timeClient.update();
            //Serial.println(timeClient.getFormattedTime());

            myDisplay.drawStr(0, 55, zone1DisplayString);
            myDisplay.drawStr(80, 55, getElapsedTimeStr());
            myDisplay.drawStr(0, 63, zone3DisplayString);
            myDisplay.drawStr(80, 63, timeClient.getFormattedTime().c_str());
            myDisplay.sendBuffer();
        }
        else if (displayMode == MULTI)
        {
            myDisplay.setFont(SYS_FONT);
            myDisplay.writeLine(1, tempDisplayString);
            myDisplay.writeLine(2, humiDisplayString);
            myDisplay.writeLine(3, MQTTDisplayString);
            myDisplay.writeLine(5, zone1DisplayString);
            myDisplay.writeLine(6, zone3DisplayString);
            myDisplay.refresh();
            Serial.println("!----------! MULTI Display Refresh");
            Serial.println(tempDisplayString);
            Serial.println(humiDisplayString);
            Serial.println(MQTTDisplayString);
            Serial.println(zone1DisplayString);
            Serial.println(zone3DisplayString);
            Serial.println("^----------^");
        }
    }
}

extern unsigned long currentMillis;
extern unsigned long previousConnCheckMillis;
extern unsigned long intervalConnCheckMillis;
//extern unsigned long intervalConnCheckMillis;
//extern unsigned long intervalConnCheckMillis;

#include "WiFiLib.h"

#include <PubSubClient.h>
extern PubSubClient MQTTclient;

void checkConnections()
{
    currentMillis = millis();
    if (currentMillis - previousConnCheckMillis > intervalConnCheckMillis)
    {
        if (!WiFi.isConnected())
        { //!= WL_CONNECTED)
            myWebSerial.println("Wifi Needs reconnecting");
            connectWiFi();
        }
        else
        {
            myWebSerial.println("OK - WiFi is connected");
        }

        if (!MQTTclient.connected())
        {
            myWebSerial.println("MQTTClient Needs reconnecting");
            connectMQTT();
        }
        else
        {
            myWebSerial.println("OK - MQTT is connected");
        }
        previousConnCheckMillis = currentMillis;
    }
}