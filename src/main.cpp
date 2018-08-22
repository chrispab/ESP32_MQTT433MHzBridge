// note : for I2C problem use ;
// https://desire.giesecke.tk/index.php/2018/04/20/how-to-use-stickbreakers-i2c-improved-code/
//
// https://github.com/espressif/arduino-esp32/issues/1352
//
// also fix for u8g2lib using wire lib
// https://community.particle.io/t/i2c-lcd-display-getting-corrupted-solved/9767/78
// edit
// /home/chris/Projects/git/ESP32_MQTT433MHzBridge/.piolibdeps/U8g2_ID942/src/U8x8lib.cpp
// and add 2us delays - 3 off, start write, end transmission
//! for led pwm
// example here : https://github.com/kriswiner/ESP32/tree/master/PWM
// use GPIO13 , phys pin 3 up on LHS
// mod of gammon ledfader

/* I2C slave Address Scanner
for 5V bus
 * Connect a 4.7k resistor between SDA and Vcc
 * Connect a 4.7k resistor between SCL and Vcc
for 3.3V bus
 * Connect a 2.4k resistor between SDA and Vcc
 * Connect a 2.4k resistor between SCL and Vcc

A resistor value of 4.7KΩ is recommended
It is strongly recommended that you place a 10 micro farad capacitor
between +ve and -ve as close to your ESP32 as you can

Note that GPIO_NUM_34 – GPIO_NUM_39 are input mode only. You can not use these pins
for signal output. Also, pins 6 (SD_CLK), 7 (SD_DATA0), 8 (SD_DATA1), 9
(SD_DATA2), 10 (SD_DATA3), 11 (SD_CMD) 16 (CS) and 17(Q) are used to interact
with the SPI flash chip ... you can not use those for other purposes.
When using pSRAM,
Strapping pins are GPIO0, GPIO2 and GPIO12.
TX and RX (as used for flash) are GPIO1 and GPIO3.
The data type called gpio_num_t is a C language enumeration with values
corresponding to these names. It is recommended to use these values rather than
attempt to use numeric values.

// startup screen text
*/
#define SW_VERSION "V3.66 Br:\"master\""

#define TITLE_LINE1 "     ESP32"
#define TITLE_LINE2 "MQTT 433MhZ Bridge"
#define TITLE_LINE3 "Zone Wireless Dog"
#define TITLE_LINE4 "MQTT Temp Sensor"
#define TITLE_LINE5 ""
#define TITLE_LINE6 SW_VERSION
//#define SYS_FONT u8g2_font_8x13_tf
#define SYS_FONT u8g2_font_6x12_tf       // 7 px high
#define BIG_TEMP_FONT u8g2_font_fub30_tf //30px hieght
// 33 too big - #define BIG_TEMP_FONT u8g2_font_inb33_mf

//! Pin GPIO usage
//Note that GPIO_NUM_34 – GPIO_NUM_39 are input mode only
#define ESP32_ONBOARD_BLUE_LED_PIN GPIO_NUM_2 // RHS_P_4 esp32 devkit on board blue LED
#define GREEN_LED_PIN GPIO_NUM_33             //LHS_P_9
#define DHTPIN GPIO_NUM_25                    // LHS_P_8 what digital pin we're connected to
#define TX433PIN GPIO_NUM_32                  //LHS_P_10
#define RF24_CE_PIN GPIO_NUM_5                //RHS_P_8
#define RF24_CS_PIN GPIO_NUM_4                //RHS_P_5
#define RF24_SPI_CLK GPIO_NUM_18              //RHS_P_9 green wire
#define RF24_SPI_MISO GPIO_NUM_19             //RHS_P_10 purple wire
#define RF24_SPI_MOSI GPIO_NUM_23             //RHS_P_15 blue wire
#define OLED_CLOCK_PIN GPIO_NUM_22            //RHS_P_14 SCL
#define OLED_DATA_PIN GPIO_NUM_21             //RHS_P_11 SDA
#define RED_LED_PIN GPIO_NUM_26               //LHS_P_7  ??

#define TOUCH_SENSOR_1 GPIO_NUM_13 //LHS_P_3
#define TOUCH_SENSOR_2 GPIO_NUM_12 //LHS_P_4
#define TOUCH_SENSOR_3 GPIO_NUM_14 //LHS_P_5
#define TOUCH_SENSOR_4 GPIO_NUM_27 //LHS_P_6
#define TOUCH_SENSOR_5 GPIO_NUM_15 // RHS_P_3  !! also used by 433 Tx??? - resolve

#define TOUCH_PIN T4 // ESP32 Pin gpio13

#define CR Serial.println()

// todo add oled power control fet

#include <Arduino.h>
#include <NewRemoteTransmitter.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <WiFi.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
//#include <stdio.h>
//#include <string.h>

// classes code changes
#include "Display.h"
#include "ZoneController.h"

//#include
//"/home/chris/.platformio/packages/framework-espidf/components/driver/include/driver/periph_ctrl.h"
#include "/home/chris/.platformio/packages/framework-arduinoespressif32/tools/sdk/include/driver/driver/periph_ctrl.h"
#include "LedFader.h"
#include "TempSensor.h"
#include "sendemail.h"

//time stuff
#include <NTPClient.h>
//#include <WiFi.h>
#include <WiFiUdp.h>
#define NTP_OFFSET 60 * 60     // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "europe.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
//NTPClient timeClient(ntpUDP);

// forward decs
void printWifiStatus();
boolean processRequest(String &getLine);
void sendResponse(WiFiClient client);
void listenForClients(void);
void LEDBlink(int LEDPin, int repeatNum);
void MQTTRxcallback(char *topic, byte *payload, unsigned int length);
void connectMQTT();
void connectWiFi();
void operateSocket(uint8_t socketID, uint8_t state);
void checkConnections(void);
// void updateTempDisplay(void);
void setPipes(uint8_t *writingPipe, uint8_t *readingPipe);
void processZoneRF24Message(void);
int equalID(char *receive_payload, const char *targetID);

// void updateZoneDisplayLines(void);
void updateDisplayData(void);

int freeRam(void);
void printFreeRam(void);

// char *getsensorDisplayString(char *thistempStr);
void processMQTTMessage(void);
char *getMQTTDisplayString(char *MQTTStatus);

void resetI2C(void);
void resetWatchdog(void);
boolean processTouchPads(void);
void connectRF24(void);

void WiFiLocalWebPageCtrl(void);

// DHT22 stuff
TempSensor DHT22Sensor;

// WiFi settings
const char ssid[] = "notwork";
const char pass[] = "a new router can solve many problems";
int status = WL_IDLE_STATUS;

// MQTT stuff
IPAddress mqttserver(192, 168, 0, 200);
char subscribeTopic[] = "433Bridge/cmnd/#";
char publishTempTopic[] = "433Bridge/Temperature";
char publishHumiTopic[] = "433Bridge/Humidity";

WiFiClient WiFiEClient;
PubSubClient MQTTclient(mqttserver, 1883, MQTTRxcallback, WiFiEClient);

// 433Mhz settings
// 282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, TX433PIN, 256, 5);
// last param is num times control message  is txed

//// Set up nRF24L01 rf24Radio on SPI bus plus pins 7 & 8

RF24 rf24Radio(RF24_CE_PIN, RF24_CS_PIN);
uint8_t writePipeLocS[] = "NodeS";
uint8_t readPipeLocS[] = "Node0";
uint8_t writePipeLocC[] = "NodeC";
uint8_t readPipeLocC[] = "Node0";
uint8_t writePipeLocG[] = "NodeG";
uint8_t readPipeLocG[] = "Node0";
// Payload
const int max_payload_size = 32;
char receive_payload[max_payload_size + 1];
// +1 to allow room for a terminating NULL char
// static unsigned int goodSecsMax = 15; // 20

// Global vars
unsigned long currentMillis = 0;
unsigned long previousConnCheckMillis = 0;
unsigned long intervalConnCheckMillis = 63000;

unsigned long intervalTempDisplayMillis = 60000;
unsigned long previousTempDisplayMillis =
    millis() - intervalTempDisplayMillis; // trigger on start

char tempStr[17]; // buffer for 16 chars and eos
// static unsigned long UpdateInterval = 250; // 1000ms
// static unsigned long displayUpdateInterval = 250; // ms

// create system objects
// create the display object

Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/OLED_CLOCK_PIN,
                  /* data=*/OLED_DATA_PIN);
ZoneController ZCs[3] = {ZoneController(0, 14, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 15, "SHD", "SSS")};
// display vars
boolean bigTempDisplayEnabled = true;

WiFiServer server(80);

enum displayModes
{
    NORMAL,
    BIG_TEMP,
    MULTI
} displayMode;
// enum displayModes ;

int MQTTNewState = 0;     // 0 or 1
int MQTTSocketNumber = 0; // 1-16
boolean MQTTNewData = false;
// create object
SendEmail e("smtp.gmail.com", 465, "cbattisson@gmail.com", "fbmfbmqluzaakvso",
            5000, true);
// set parameters. pin 13, go from 0 to 255 every n milliseconds
LedFader heartBeatLED(GREEN_LED_PIN, 1, 0, 255, 700, true);
LedFader warnLED(RED_LED_PIN, 2, 0, 255, 451, true);

// array to enable translation from socket ID (0-15) to string representing
// socket function
const char *socketIDFunctionStrings[16];

//! WATCHDOG STUFF
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule()
{
    ets_printf("ESP32 Rebooted by Internal Watchdog\n");
    esp_restart_noos();
}

void setup()
{ // Initialize serial monitor port to PC and wait for port to

    // strcpy(socketIDFunctionStrings[0], "blah");
    socketIDFunctionStrings[0] = "blah";
    socketIDFunctionStrings[1] = "blah";
    socketIDFunctionStrings[2] = "L Lights";
    socketIDFunctionStrings[3] = "D Lights";
    socketIDFunctionStrings[4] = "C Lights";
    socketIDFunctionStrings[5] = "DAB";
    socketIDFunctionStrings[6] = "Amp";
    socketIDFunctionStrings[7] = "TV";
    socketIDFunctionStrings[8] = "CSV Rads";
    socketIDFunctionStrings[9] = "Fan";
    socketIDFunctionStrings[10] = "blah";
    socketIDFunctionStrings[11] = "blah";
    socketIDFunctionStrings[12] = "blah";
    socketIDFunctionStrings[13] = "blah";
    socketIDFunctionStrings[14] = "blah";
    socketIDFunctionStrings[15] = "blah";

    heartBeatLED.begin(); // initialize
    warnLED.begin();      // initialize

    // reset i2c bus controllerfrom IDF call
    periph_module_reset(PERIPH_I2C0_MODULE);

    Serial.begin(115200);
    Serial.println("==========running setup==========");

    pinMode(ESP32_ONBOARD_BLUE_LED_PIN, OUTPUT); // set the LED pin mode
    displayMode = NORMAL;
    displayMode = BIG_TEMP;
    //displayMode = MULTI;
    // setup OLED display
    myDisplay.begin();

    myDisplay.setFont(SYS_FONT);
    myDisplay.wipe();

    myDisplay.writeLine(1, TITLE_LINE1);
    myDisplay.writeLine(2, TITLE_LINE2);
    myDisplay.writeLine(3, TITLE_LINE3);
    myDisplay.writeLine(4, TITLE_LINE4);
    myDisplay.writeLine(5, TITLE_LINE5);
    myDisplay.writeLine(6, TITLE_LINE6);
    myDisplay.refresh();
    delay(3000);

    myDisplay.wipe();
    myDisplay.writeLine(1, "Connecting to Sensor..");
    myDisplay.refresh();
    DHT22Sensor.setup(DHTPIN, DHT22Sensor.AM2302);

    // rf24 stuff
    myDisplay.writeLine(2, "Connecting to RF24..");
    myDisplay.refresh();
    connectRF24();

    // attempt to connect to Wifi network:
    myDisplay.writeLine(3, "Connecting to WiFi..");
    myDisplay.refresh();
    connectWiFi();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    myDisplay.writeLine(4, "Connecting to MQTT..");
    myDisplay.refresh();
    connectMQTT();
    myDisplay.writeLine(5, "All Connected");
    myDisplay.refresh();

    timeClient.begin();
    timeClient.update();

    Serial.println(timeClient.getFormattedTime());

    delay(500);

    // MQTTclient.loop(); //process any MQTT stuff
    // checkConnections();

    // Send Email
    e.send("<cbattisson@gmail.com>", "<cbattisson@gmail.com>", "ESP32 Started",
           "programm started/restarted");

    //! watchdog setup
    timer = timerBegin(0, 80, true); // timer 0, div 80
    timerAttachInterrupt(timer, &resetModule, true);
    // 40 secs
    timerAlarmWrite(timer, 40000000, false); // set time in us
    timerAlarmEnable(timer);                 // enable interrupt
    myDisplay.wipe();

    //connectWiFi();
}

boolean touchedFlag = false;

void loop()
{
    resetWatchdog();
    heartBeatLED.update(); // initialize
    warnLED.update();      // initialize

    // updateDisplayData();
    // DHT22Sensor.takeReadings();
    DHT22Sensor.publishReadings(MQTTclient, publishTempTopic, publishHumiTopic);

    // myDisplay.refresh();
    // capture new sensor readings

    MQTTclient.loop(); // process any MQTT stuff, returned in callback
    //updateDisplayData();

    processMQTTMessage(); // check flags set above and act on
    touchedFlag = processTouchPads();
    updateDisplayData();

    // updateDisplayData();
    processZoneRF24Message(); // process any zone watchdog messages
    if (ZCs[0].manageRestarts(transmitter) == true)
    {
        e.send("<cbattisson@gmail.com>", "<cbattisson@gmail.com>",
               "ESP32 Watchdog: Zone 1 power cycled",
               "ESP32 Watchdog: Zone 1 power cycled");
    }
    // manageRestarts(1);
    // ZCs[1].manageRestarts(transmitter);
    ZCs[1].resetZoneDevice();
    // manageRestarts(2);
    if (ZCs[2].manageRestarts(transmitter) == true)
    {
        e.send("<cbattisson@gmail.com>", "<cbattisson@gmail.com>",
               "ESP32 Watchdog: Zone 3 power cycled",
               "ESP32 Watchdog: Zone 3 power cycled");
    }

    // myDisplay.refresh();
    checkConnections(); // reconnect if reqd
    resetI2C();         // not sure if this is reqd. maybe display at fault

    WiFiLocalWebPageCtrl();
}

/***************************************************
* Send and receive data from Local Page
****************************************************/
void WiFiLocalWebPageCtrl(void)
{
    static char tempDisplayString[20];
    static char humiDisplayString[20];
    static char zone1DisplayString[20];
    static char zone3DisplayString[20];
    static char MQTTDisplayString[20];

    WiFiClient client = server.available(); // listen for incoming clients
    //client = server.available();
    if (client)
    {                                  // if you get a client,
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println(timeClient.getFormattedTime());
                        client.print("<br>");

                        // the content of the HTTP response follows the header:
                        //WiFiLocalWebPageCtrl();
                        //client.print("Temperature now is: ");
                        //client.print(localTemp);
                        // client.print("  oC<br>");
                        // client.print("Humidity now is:     ");
                        //client.print(localHum);
                        // client.print(" % <br>");
                        // client.print("<br>");
                        // client.print("Analog Data:     ");
                        //client.print(analog_value);
                        // client.print("<br>");
                        // client.print("<br>");

                        // client.print("Click <a href=\"/H\">here</a> to turn the LED on.<br>");
                        // client.print("Click <a href=\"/L\">here</a> to turn the LED off.<br>");

                        //Serial.println("!----------! BIG_TEMP Display Refresh");
                        client.print(DHT22Sensor.getTempDisplayString(tempDisplayString));
                        client.print("<br>");
                        client.print(DHT22Sensor.getHumiDisplayString(humiDisplayString));
                        client.print("<br>");
                        client.print(getMQTTDisplayString(MQTTDisplayString));
                        client.print("<br>");

                        client.print(ZCs[0].getDisplayString(zone1DisplayString));
                        client.print("<br>");
                        client.print(ZCs[2].getDisplayString(zone3DisplayString));
                        client.print("<br>");
                        // Serial.println(zone3DisplayString);
                        //Serial.println("^----------^");

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /H"))
                {
                    //digitalWrite(LED_PIN, HIGH);               // GET /H turns the LED on
                }
                if (currentLine.endsWith("GET /L"))
                {
                    //digitalWrite(LED_PIN, LOW);                // GET /L turns the LED off
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}

boolean processTouchPads(void)
{
    static int lastFilteredVal = 54;
    static int filteredVal = 54;
    const int filterConstant = 3; // 2 ishalf, 4 is quarter etc
    int touchThreshold = 32;
    int newTouchValue = 100;

    //newTouchValue = touchRead(TOUCH_PIN);
    //Serial.println(newTouchValue);
    //delay(500);

    // only read touch sensors every 100ms
    static unsigned long lastTouchReadMillis = millis();
    unsigned long touchReadInterval = 25;

    if ((millis() - lastTouchReadMillis) >= touchReadInterval)
    {
        newTouchValue = touchRead(TOUCH_PIN);
        lastTouchReadMillis = millis();

        //! software addition filter here to even out spurious readings
        int diff;
        //filteredVal = filteredVal + (filteredVal )
        if (newTouchValue > filteredVal) //if going up - add onto filteredval
        {
            diff = (newTouchValue - filteredVal);
            filteredVal = filteredVal + (diff / filterConstant);
        }
        if (newTouchValue < filteredVal) //if going down - subtract  from filteredval
        {
            diff = (filteredVal - newTouchValue);
            filteredVal = filteredVal - (diff / filterConstant);
        }

        // !! detect edges
        if ((lastFilteredVal > touchThreshold) && (filteredVal < touchThreshold))
        { //high to low edge finger on
            Serial.print("$$$]_ Touch Val = ");
            Serial.println(newTouchValue);
            Serial.print("filtered Val = ");
            Serial.println(filteredVal);

            displayMode = MULTI;
            lastFilteredVal = filteredVal;
            //lastTouchReadMillis = millis();

            return true;
        }
        if ((lastFilteredVal < touchThreshold) && (filteredVal > touchThreshold))
        { //low to high edge finger off
            Serial.print("$$$_[ Touch Val = ");
            Serial.println(newTouchValue);
            Serial.print("filtered Val = ");
            Serial.println(filteredVal);
            displayMode = BIG_TEMP;
            lastFilteredVal = filteredVal;
            //lastTouchReadMillis = millis();

            return true;
        }
        lastFilteredVal = filteredVal;

        return false; // no edge detected
    }
    return false; // no edge detected
}

void resetWatchdog(void)
{
    static unsigned long lastResetWatchdogMillis = millis();
    unsigned long resetWatchdogInterval = 10000;

    if ((millis() - lastResetWatchdogMillis) >= resetWatchdogInterval)
    {

        timerWrite(timer, 0); // reset timer (feed watchdog)
        Serial.println("Reset Module Watchdog......");
        lastResetWatchdogMillis = millis();
    }
}
void resetI2C(void)
{
    static unsigned long lastResetI2CMillis = millis();
    unsigned long resetI2CInterval = 360000;
    // do only every few hours
    // u_long is 0to 4,294,967,295
    // 3600000 ms is 1hr
    if ((millis() - lastResetI2CMillis) >= resetI2CInterval)
    {
        // reset i2c bus controllerfrom IDF call
        periph_module_reset(PERIPH_I2C0_MODULE);
        // delay(100);
        lastResetI2CMillis = millis();
        Serial.println("I2C RESET.......");

        // ESP.restart();
    }
}
void processMQTTMessage(void)
{
    if (MQTTNewData)
    {
        digitalWrite(ESP32_ONBOARD_BLUE_LED_PIN, MQTTNewState);
        //Serial
        operateSocket(MQTTSocketNumber - 1, MQTTNewState);
        MQTTNewData = false; // indicate not new data now, processed
    }
}

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

    // only update screen if status messages has changed
    // compare new display data to new data
    // if different - update the actual OLED display
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

        if (displayMode == NORMAL)
        {
            // updateTempDisplay(); // get and display temp
            // updateZoneDisplayLines();
        }
        else if (displayMode == BIG_TEMP)
        {
            myDisplay.clearBuffer();
            myDisplay.setFont(BIG_TEMP_FONT);
            //just get the temp bit of displaystring
            //end of string is 'C', need to get string from that pos
            strcpy(justTempString, &tempDisplayString[6]);
            //myDisplay.writeLine(4, justTempString);
            myDisplay.drawStr(0, 38, justTempString);
            //myDisplay.refresh();

            myDisplay.setFont(SYS_FONT);
            myDisplay.drawStr(0, 47, MQTTDisplayString);
            timeClient.update();
            //Serial.println(timeClient.getFormattedTime());

            myDisplay.drawStr(0, 55, zone1DisplayString);
            myDisplay.drawStr(0, 63, zone3DisplayString);
            myDisplay.drawStr(80, 63, timeClient.getFormattedTime().c_str());

            myDisplay.sendBuffer();
            Serial.println("!----------! BIG_TEMP Display Refresh");
            Serial.println(tempDisplayString);
            Serial.println(MQTTDisplayString);
            Serial.println(timeClient.getFormattedTime().c_str());
            Serial.println(zone1DisplayString);
            Serial.println(zone3DisplayString);
            Serial.println("^----------^");
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

char *getMQTTDisplayString(char *MQTTStatus)
{
    char msg[20] = "Socket:";
    char buff[10];

    sprintf(buff, "%d", (MQTTSocketNumber));

    // strcpy()
    strcpy(msg, socketIDFunctionStrings[MQTTSocketNumber - 1]);
    strcat(msg, "(");

    strcat(msg, buff);
    strcat(msg, "):");

    if (MQTTNewState == 0)
    {
        strcat(msg, " OFF");
    }
    else
    {
        strcat(msg, " ON");
    }
    // Serial.println(msg);
    strcpy(MQTTStatus, msg);

    return MQTTStatus;
}

void checkConnections()
{
    currentMillis = millis();
    if (currentMillis - previousConnCheckMillis > intervalConnCheckMillis)
    {

        if (!WiFi.isConnected())
        { //!= WL_CONNECTED)
            Serial.println("Wifi Needs reconnecting");
            connectWiFi();
        }
        else
        {
            Serial.println("OK - WiFi is connected");
        }

        if (!MQTTclient.connected())
        {
            Serial.println("MQTTClient Needs reconnecting");
            connectMQTT();
        }
        else
        {
            Serial.println("OK - MQTT is connected");
        }
        previousConnCheckMillis = currentMillis;
    }
}
void connectRF24()
{
    rf24Radio.begin();
    // enable dynamic payloads
    rf24Radio.enableDynamicPayloads();
    // optionally, increase the delay between retries & # of retries
    // rf24Radio.setRetries(15, 15);
    rf24Radio.setPALevel(RF24_PA_MAX);
    rf24Radio.setDataRate(RF24_250KBPS);
    rf24Radio.setChannel(124);
    rf24Radio.startListening();
    rf24Radio.printDetails();
    // autoACK enabled by default
    setPipes(writePipeLocC,
             readPipeLocC); // SHOULD NEVER NEED TO CHANGE PIPES
    rf24Radio.startListening();
}
void connectWiFi()
{
    bool wifiConnectTimeout = false;
    u16_t startMillis;
    u16_t timeOutMillis = 20000;

    // Loop until we're reconnected
    // check is MQTTclient is connected first
    // attempt to connect to Wifi network:
    // printO(1, 20, "Connect WiFi..");

    // IMPLEMNT TIME OUT TO ALLOW RF24 WIRLESS DOG FUNC TO CONTINUE
    // while (!WiFiEClient.connected())
    // while (status != WL_CONNECTED)
    wifiConnectTimeout = false;

    startMillis = millis();
    while (!WiFi.isConnected() && !wifiConnectTimeout)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open
        // or WEP network:
        WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(5000);
        // enable jump out if connection attempt has timed out
        wifiConnectTimeout =
            ((millis() - startMillis) > timeOutMillis) ? true : false;
    }
    (wifiConnectTimeout) ? Serial.println("WiFi Connection attempt Timed Out!")
                         : Serial.println("Wifi Connection made!");

    server.begin();
}

void connectMQTT()
{
    bool MQTTConnectTimeout = false;
    u16_t startMillis;
    u16_t timeOutMillis = 20000;
    // Loop until we're reconnected
    // check is MQTTclient is connected first
    MQTTConnectTimeout = false;

    startMillis = millis();
    while (!MQTTclient.connected() && !MQTTConnectTimeout)
    {
        // printO(1, 20, "Connect MQTT..");

        Serial.println(F("Attempting MQTT connection..."));
        // Attempt to connect
        //        if (MQTTclient.connect("ESP32Client","",""))
        if (MQTTclient.connect("ESP32Client"))
        {
            Serial.println(F("connected to MQTT server"));
            // Once connected, publish an announcement...
            // MQTTclient.publish("outTopic", "hello world");
            // ... and resubscribe
            // MQTTclient.subscribe("inTopic");
            MQTTclient.subscribe(subscribeTopic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(MQTTclient.state());
            Serial.println(" try again ..");
            // Wait 5 seconds before retrying
            // delay(5000);
        }
        MQTTConnectTimeout =
            ((millis() - startMillis) > timeOutMillis) ? true : false;
    }
    (!MQTTConnectTimeout) ? Serial.println("MQTT Connection made!")
                          : Serial.println("MQTT Connection attempt Time Out!");
}

// MQTTclient call back if mqtt messsage rxed
void MQTTRxcallback(char *topic, byte *payload, unsigned int length)
{
    uint8_t socketNumber = 0;

    // Power<x> 		Show current power state of relay<x> as On or
    // Off Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
    // 1 / on 	Turn relay<x> power On handle message arrived mqtt
    // pubsub

    // Serial.println("Rxed a mesage from broker : ");

    // do some extra checking on rxed topic and payload
    payload[length] = '\0';
    // String s = String((char *)payload);
    // //float f = s.toFloat();
    // Serial.print(s);
    // Serial.println("--EOP");

    // CR;
    Serial.print("MQTT rxed [");
    Serial.print(topic);
    Serial.print("] : ");
    for (uint8_t i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    CR;
    // e.g topic = "433Bridge/cmnd/Power1" to "...Power16", and payload = 1 or 0
    // either match whole topic string or trim off last 1or 2 chars and
    // convert to a number, convert last 1-2 chars to socket number
    char lastChar =
        topic[strlen(topic) - 1]; // lst char will always be a digit char
    // see if last but 1 is also a digit char - ie number has two digits -
    // 10 to 16
    char lastButOneChar = topic[strlen(topic) - 2];

    if ((lastButOneChar >= '0') &&
        (lastButOneChar <= '9'))
    { // is it a 2 digit number
        socketNumber =
            ((lastButOneChar - '0') * 10) + (lastChar - '0'); // calc actual int
    }
    else
    {
        socketNumber = (lastChar - '0');
    }
    // convert from 1-16 range to 0-15 range sendUnit uses
    // int socketID = socketNumber - 1;
    uint8_t newState = 0; // default to off
    if ((payload[0] - '1') == 0)
    {
        newState = 1;
    }
    // signal a new command has been rxed
    // and
    // topic and payload also available
    MQTTNewState = newState;         // 0 or 1
    MQTTSocketNumber = socketNumber; // 1-16
    MQTTNewData = true;
    // then leave main control loop to turn of/onn sockets
    // and reset sigmals

    // digitalWrite(ESP32_ONBOARD_BLUE_LED_PIN, newState);
    // operateSocket(socketID, newState);
}

// 0-15, 0,1
// mqtt funct to operate a remote power socket
// only used by mqtt function
void operateSocket(uint8_t socketID, uint8_t state)
{
    // this is a blocking routine !!!!!!!
    char msg[30] = "Operate Socket:";
    char buff[10];

    // strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (socketID + 1));
    strcat(msg, buff);

    // u8g2.setCursor(55, 40);
    if (state == 0)
    {
        strcat(msg, " - OFF");
    }
    else
    {
        strcat(msg, " - ON");
    }
    Serial.println(msg);
    transmitter.sendUnit(socketID, state);
    //}

    // Serial.println("OK - TX socket state updated");
}

void LEDBlink(int LPin, int repeatNum)
{
    for (int i = 0; i < repeatNum; i++)
    {
        digitalWrite(LPin, 1); // GET /H turns the LED on
        delay(50);
        digitalWrite(LPin, 0); // GET /H turns the LED on
        delay(50);
    }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void setPipes(uint8_t *writingPipe, uint8_t *readingPipe)
{
    // config rf24Radio to comm with a node
    rf24Radio.stopListening();
    rf24Radio.openWritingPipe(writingPipe);
    rf24Radio.openReadingPipe(1, readingPipe);
}

void processZoneRF24Message(void)
{
    char messageText[17];
    while (rf24Radio.available())
    { // Read all available payloads

        // Grab the message and process
        uint8_t len = rf24Radio.getDynamicPayloadSize();

        // If a corrupt dynamic payload is received, it will be flushed
        if (!len)
        {
            return;
        }

        rf24Radio.read(receive_payload, len);

        // Put a zero at the end for easy printing etc
        receive_payload[len] = 0;

        // who was it from?
        // reset that timer

        if (equalID(receive_payload, ZCs[0].heartBeatText))
        {
            ZCs[0].resetZoneDevice();
            Serial.println("RESET G Watchdog");
            strcpy(messageText, ZCs[0].heartBeatText);
        }
        else if (equalID(receive_payload, ZCs[1].heartBeatText))
        {
            ZCs[1].resetZoneDevice();
            Serial.println("RESET C Watchdog");
            strcpy(messageText, ZCs[1].heartBeatText);
        }
        else if (equalID(receive_payload, ZCs[2].heartBeatText))
        {
            ZCs[2].resetZoneDevice();
            Serial.println("RESET S Watchdog");
            strcpy(messageText, ZCs[2].heartBeatText);
        }
        else
        {
            Serial.println("NO MATCH");
            strcpy(messageText, "NO MATCH");
        }
    }
}

int equalID(char *receive_payload, const char *targetID)
{
    // check if same 1st 3 chars
    if ((receive_payload[0] == targetID[0]) &&
        (receive_payload[1] == targetID[1]) &&
        (receive_payload[2] == targetID[2]))
    {
        return true;
    }
    else
    {
        return false;
    }
}
