#include <Arduino.h>
#include <WiFi.h>
#include <Display.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include "DHT.h"
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
#include <RF24.h>

//forward decs
void printWifiStatus();
boolean processRequest(String &getLine);
void sendResponse(WiFiClient client);
void listenForClients(void);
void LEDBlink(int LEDPin, int repeatNum);
void callback(char *topic, byte *payload, unsigned int length);
void reconnectPSClient();
void reconnectWiFi();
void operateSocket(uint8_t socketID, uint8_t state);
void printO(const char *message);
void printOWithVal(const char *message, int value);
void printO2Str(const char *str1, const char *str2);
void checkConnections(void);
void updateTempDisplay(void);
void printO(int x, int y, const char *text);
void setPipes(uint8_t *writingPipe, uint8_t *readingPipe);
void processZoneMessage(void);
int equalID(char *receive_payload, const char *targetID);
void displayRefresh(void);
void displayWriteLine(int lineNumber, const char *lineText);
void resetZoneDevice(int deviceID);
void updateZoneDisplayLines(void);
int freeRam(void);
void printFreeRam(void);
void displayWipe(void);
void manageRestarts(int deviceID);
void powerCycle(int deviceID);

//OLED display stuff
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping
#define LINE_HIEGHT 10
#define XPIX 128
#define YPIX 64
#define DISPLAY_LINES 6
char displayLine[6][17];  //6 lines of 16 chars +eol for dispaly store
//int dispUpdateFreq = 1.5; // how many updates per sec

//DHT22 stuff
//#define DHTPIN 23 // what digital pin we're connected to
//#define DHTPIN 33 // what digital pin we're connected to
//DHT dht;

//WiFi settings
const char ssid[] PROGMEM = "notwork";                              // your network SSID (name)
const char pass[] PROGMEM = "a new router can solve many problems"; // your network password
////int status = WL_IDLE_STATUS;

//MQTT stuff
// IPAddress mqttserver(192, 168, 0, 200);
// const char subscribeTopic[] PROGMEM = "433Bridge/cmnd/#";
// WiFiClient WiFiEClient;
// PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);

//433Mhz settings
#define TX433PIN 32
//282830 addr of 16ch remote
//NewRemoteTransmitter transmitter(282830, TX433PIN); // tx address, pin for tx
//byte socket = 3;
//bool state = false;
//uint8_t socketNumber = 0;

//// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
// #define ce_pin 5
// #define cs_pin 4
// RF24 radio(ce_pin, cs_pin);
// uint8_t writePipeLocS[] PROGMEM = "NodeS";
// uint8_t readPipeLocS[] PROGMEM = "Node0";
// uint8_t writePipeLocC[] PROGMEM = "NodeC";
// uint8_t readPipeLocC[] PROGMEM = "Node0";
// uint8_t writePipeLocG[] PROGMEM = "NodeG";
// uint8_t readPipeLocG[] PROGMEM = "Node0";
// Payload
// const int max_payload_size = 32;
// char receive_payload[max_payload_size + 1];                   // +1 to allow room for a terminating NULL char
// static unsigned int goodSecsMax = 15;                         //20
// static unsigned long maxMillisNoAckFromPi = 1000UL * 300UL;   //300 max millisces to wait if no ack from pi before power cycling pi
// static unsigned long waitForPiPowerUpMillis = 1000UL * 120UL; //120

// define a struct for each controller instance with related vars
// struct controller
// {
//     uint8_t id_number;
//     uint8_t zone;
//     uint8_t socketID;
//     unsigned long lastGoodAckMillis;
//     char name[4];
//     char heartBeatText[4];   // allow enough space for text plus 1 extra for null terminator
//     char badStatusMess[10];  // enough for 1 line on display
//     char goodStatusMess[10]; // enough for 1 line on display
//     bool isRebooting;        //indicate if booting up
//     bool isPowerCycling;     //indicate if power cycling
//     unsigned long rebootMillisLeft;
//     unsigned long lastRebootMillisLeftUpdate;
//     uint8_t powerCyclesSincePowerOn;
// };
// struct controller devices[3]; //create an array of 3 controller structures

//misc stuff
#define LEDPIN 2 //esp32 devkit on board blue LED
#define CR Serial.println()
#define TITLE_LINE1 "MQTT ESP32"
#define TITLE_LINE2 "433Mhz Bridge"
#define SW_VERSION "V1.3"

//Global vars
// unsigned long currentMillis = 0;
// //conection check timer
// unsigned long previousConnCheckMillis = 0;
// unsigned long intervalConCheckMillis = 30000;
// //unsigned long currentMillis = 0;
// unsigned long previousTempDisplayMillis = 0;
// unsigned long intervalTempDisplayMillis = 20000;
// char tempStr[17]; // buffer for 16 chars and eos

#include "Display.h"

Display::Display(void)
{
    //NOP;
    // _address = address;
    // _pin = pin;
    // _periodusec = periodusec;
    // _repeats = (1 << repeats) - 1; // I.e. _repeats = 2^repeats - 1

    // pinMode(_pin, OUTPUT);
}

void Display::setup()
{ //Initialize serial monitor port to PC and wait for port to open:

    begin();

    displayWriteLine(1, TITLE_LINE1);
    displayWriteLine(2, TITLE_LINE2);
    displayWriteLine(3, SW_VERSION);
    displayRefresh();

    delay(5000);

    //psclient.loop(); //process any MQTT stuff
    //checkConnections();
    displayWipe();
}

//redraw the display with contents of displayLine array
void Display::displayRefresh(void)
{
    // u8g2.begin();
    clearBuffer();
    setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++)
    {
        drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    delay(10);
    sendBuffer();
}
//redraw the display with contents of displayLine array
void Display::displayWipe(void)
{
    // u8g2.begin();
    clearBuffer();
    setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++)
    {
        strcpy(displayLine[i], " ");
        drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    delay(10);
    sendBuffer();
}
//add-update a line of text in the display text buffer
void Display::displayWriteLine(int lineNumber, const char *lineText)
{
    //update a line in the diaplay text buffer
    strcpy(displayLine[lineNumber - 1], lineText);
}

void printO(int x, int y, const char *text)
{
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    u8g2.drawStr(x, y, text);
    u8g2.sendBuffer();
    delay(10);
}

void printO(const char *message)
{
    u8g2.print(message);
}

void printOWithVal(const char *message, int value)
{
    u8g2.print(message);
    u8g2.print(value);
}

void printO2Str(const char *str1, const char *str2)
{
    u8g2.print(str1);
    u8g2.print(str2);
}
