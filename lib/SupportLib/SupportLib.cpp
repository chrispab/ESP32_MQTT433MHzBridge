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
char *RF24getDisplayString(char *statusMessage);

// static unsigned long lastDisplayUpdateMillis = 0;
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
static char newMQTTDisplayString[] = "12345678901234567890";
static char newRF24DisplayString[] = "12345678901234567890";

void updateDisplayData()
{

    char justTempString[20];
    char strx[30];
    //blip red led if zones display has changed
    if (
        strcmp(zone1DisplayString, ZCs[0].getDisplayString(newZone1DisplayString)) ||
        strcmp(zone3DisplayString, ZCs[2].getDisplayString(newZone3DisplayString)))
    {

        warnLED.fullOn();
        delay(1);
        warnLED.fullOff();
    }

    //update the status strings
    DHT22Sensor.getTempDisplayString(newTempDisplayString);
    DHT22Sensor.getHumiDisplayString(newHumiDisplayString); //get current humi reading
    getMQTTDisplayString(newMQTTDisplayString);
    RF24getDisplayString(newRF24DisplayString);

    if (
        strcmp(tempDisplayString, newTempDisplayString)
    || strcmp(zone1DisplayString, newZone1DisplayString)
    || strcmp(zone3DisplayString, newZone3DisplayString)
    || strcmp(MQTTDisplayString, newMQTTDisplayString)
    || strcmp(RF24DisplayString, newRF24DisplayString)
        //|| touchedFlag
    )
    {
        if (strcmp(MQTTDisplayString, newMQTTDisplayString)){
                    myWebSerial.println("MQTT DISP STRING CHANGED");

        }
        
        // copy new data to old vars
        strcpy(tempDisplayString, newTempDisplayString);
        strcpy(humiDisplayString, newHumiDisplayString);
        strcpy(zone1DisplayString, newZone1DisplayString);
        strcpy(zone3DisplayString, newZone3DisplayString);
        strcpy(MQTTDisplayString, newMQTTDisplayString);

        //strcpy(newRF24DisplayString, RF24getDisplayString(newRF24DisplayString));
        strcpy(RF24DisplayString, newRF24DisplayString);

        myWebSerial.println("");
        myWebSerial.println("!----------! BIG_TEMP Display Refresh");
        myWebSerial.println(tempDisplayString);
        myWebSerial.println(MQTTDisplayString);
        myWebSerial.println(RF24DisplayString);

        myWebSerial.println(getElapsedTimeStr());
        myWebSerial.println(timeClient.getFormattedTime().c_str());
        myWebSerial.println(zone1DisplayString);
        myWebSerial.println(zone3DisplayString);
        myWebSerial.println("^----------^");

        if ((displayMode == BIG_TEMP) || (displayMode == NORMAL))
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
            //x,y
            myDisplay.drawStr(0, 38, justTempString);
            //myDisplay.refresh();

            myDisplay.setFont(SYS_FONT);
            

            //myDisplay.drawStr(0, 47, MQTTDisplayString);
            strcpy(strx,MQTTDisplayString);
            strcat(MQTTDisplayString, "::.");
            strcat(MQTTDisplayString, RF24DisplayString);
            myDisplay.drawStr(0, 47, MQTTDisplayString );
            timeClient.update();
            strcpy(MQTTDisplayString, strx);




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
            myWebSerial.println("");

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

#include "WiFiLib.h"

#include <PubSubClient.h>
extern PubSubClient MQTTclient;

void checkConnections()
{
    currentMillis = millis();
    if ((currentMillis - previousConnCheckMillis) > intervalConnCheckMillis)
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


#include <LightSensor.h>
extern LightSensor myLightSensor;
#include <PubSubClient.h>
extern PubSubClient MQTTclient;
char publishLightStateTopic[] = "433Bridge/LightState";
char publishLightLevelTopic[] = "433Bridge/LightLevel";
void checkLightSensor()
{
    char str[21];

    myLightSensor.getLevel(); // trigger sampling if due
    if (myLightSensor.hasNewLevel())
    {
        //MQTTclient.publish(publishLightStateTopic, myLightSensor.getState() ? "true" : "false");
        sprintf(str, "%d", myLightSensor.getLevel());
        MQTTclient.publish(publishLightLevelTopic, str);
    }
    myLightSensor.getState();
    if (myLightSensor.hasNewState())
    {
        MQTTclient.publish(publishLightStateTopic, myLightSensor.getState() ? "true" : "false");
        //sprintf(str, "%d", myLightSensor.getLevel());
        //MQTTclient.publish(publishLightLevelTopic, str);
    }
            myLightSensor.clearNewLevelFlag();
        myLightSensor.clearNewStateFlag();

};


#include <PIRSensor.h>
extern PIRSensor myPIRSensor;

#include <PubSubClient.h>
extern PubSubClient MQTTclient;
char publishPIRStateTopic[] = "433Bridge/PIRState";
//char publishPIRLevelTopic[] = "433Bridge/PIRLevel";
void checkPIRSensor()
{
    //char str[21];
    myPIRSensor.getState(); // trigger sampling if due
    if (myPIRSensor.hasNewState())
    {
        MQTTclient.publish(publishPIRStateTopic, myPIRSensor.getState() ? "true" : "false");
        //sprintf(str, "%d", myPIRSensor.getLevel());
        //MQTTclient.publish(publishPIRLevelTopic, str);

        myPIRSensor.clearNewStateFlag();
    }
};
