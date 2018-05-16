#include <Arduino.h>
#include <WiFi.h>
#include <NewRemoteTransmitter.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include "DHT.h"
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

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
void printO(int x, int y, char *text);

//OLED display stuff
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping

//DHT22 stuff
#define DHTPIN 23 // what digital pin we're connected to
DHT dht;

//WiFi settings
char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
int status = WL_IDLE_STATUS;

//MQTT stuff
IPAddress mqttserver(192, 168, 0, 200);
#define subscribeTopic "433Bridge/cmnd/#"
WiFiClient WiFiEClient;
PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);

//433Mhz settings
#define TX433PIN 32
//282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, TX433PIN); // tx address, pin for tx
byte socket = 3;
bool state = false;
uint8_t socketNumber = 0;

//misc stuff
#define LEDPIN 2 //esp32 devkit on board blue LED
#define CR Serial.println()
#define SW_VERSION "V1.0"

//Global vars
unsigned long currentMillis = 0;
//conection check timer
unsigned long previousConnCheckMillis = 0;
unsigned long intervalConCheckMillis = 30000;
//unsigned long currentMillis = 0;
unsigned long previousTempDisplayMillis = 0;
unsigned long intervalTempDisplayMillis = 20000;
char tempStr[17]; // buffer for 16 chars and eos

void setup()
{ //Initialize serial monitor port to PC and wait for port to open:
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    u8g2.drawStr(30, 20, "MQTT ESP32");
    u8g2.drawStr(15, 32, "433Mhz Bridge");
    u8g2.drawStr(45, 44, SW_VERSION);
    u8g2.sendBuffer();
    delay(5000);

    //printO("Trying to connect..", 1, 20);
    WiFi.begin();
    // attempt to connect to Wifi network:
    reconnectWiFi();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    reconnectPSClient();
    dht.setup(DHTPIN);
}

void loop()
{
    psclient.loop();     //process any MQTT stuff
    checkConnections();  // reconnect if reqd
    updateTempDisplay(); //get and display temp
}

void checkConnections()
{
    currentMillis = millis();
    if (currentMillis - previousConnCheckMillis > intervalConCheckMillis)
    {
        Serial.println("Checking if WiFi or MQTT PSClient needs reconnecting"); // save the last time looped
        if (status != WL_CONNECTED)
        {
            Serial.println("Wifi Needs reconnecting");
            reconnectWiFi();
        }
        else
        {
            Serial.println("OK - WiFi still connected");
            //Serial.println(millis());
        }

        //Serial.println("Checking if psclient needs reconnect"); // save the last time looped
        if (!psclient.connected())
        {
            Serial.println("PSClient Needs reconnecting");
            reconnectPSClient();
        }
        else
        {
            Serial.println("OK - MQTT PS client still connected");
            //Serial.println(millis());
        }

        previousConnCheckMillis = currentMillis;
    }
}

void updateTempDisplay()
{
    currentMillis = millis();
    if (currentMillis - previousTempDisplayMillis > intervalTempDisplayMillis)
    {

        // Read temperature as Celsius (the default)
        float t = dht.getTemperature();
        //char tempStr[9]; //="12345678";buffer
        // Check if any reads failed and exit early (to try again).
        char msgStr[]="Message String long enough?";
        strcpy(msgStr, "Temp: ");
        if (isnan(t))
        {
            Serial.println("Failed to read from DHT sensor!");
            strcpy(tempStr, "-=NaN=-");
        }
        else // is a number
        {
            dtostrf(t, 4, 1, tempStr);
            strcat(tempStr, "*C");
        }

        // u8g2.clearBuffer();
        // u8g2.setCursor(20, 40);
        // printO2Str("Temp: ", tempStr);
        // delay(10);
        // u8g2.sendBuffer();

        printO(20,40, strcat(msgStr,tempStr));

        Serial.print("Temperature: ");
        Serial.println(tempStr);
        //Serial.println(" *C ");

        previousTempDisplayMillis = currentMillis;
    }
}

//psclient call back if mqtt messsage rxed
void callback(char *topic, byte *payload, unsigned int length)
{
    // Power<x> 		Show current power state of relay<x> as On or Off
    // Power<x> 	0 / off 	Turn relay<x> power Off
    // Power<x> 	1 / on 	Turn relay<x> power On
    // handle message arrived mqtt pubsub

    // Serial.println("Rxed a mesage from broker : ");

    //do some extra checking on rxed topic and payload
    payload[length] = '\0';
    //String s = String((char *)payload);
    // //float f = s.toFloat();
    // Serial.print(s);
    // Serial.println("--EOP");

    CR;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] : ");
    for (uint8_t i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    CR;
    Serial.println();
    //e.g topic = "433Bridge/cmnd/Power1", and payload = 1 or 0
    // either match whole topic string or trim off last 1or 2 chars and convert to a number
    //convert last 1-2 chars to socket number
    //get last char
    char lastChar = topic[strlen(topic) - 1]; //lst char will always be a digit char
    //see if last but 1 is also a digit char - ie number has two digits - 10 to 16
    char lastButOneChar = topic[strlen(topic) - 2];

    if ((lastButOneChar >= '0') && (lastButOneChar <= '9'))
    {                                                                    // is it a 2 digit number
        socketNumber = ((lastButOneChar - '0') * 10) + (lastChar - '0'); // calc actual int
    }
    else
    {
        socketNumber = (lastChar - '0');
    }

    socketNumber--;       // convert from 1-16 range to 0-15 range sendUnit uses
    uint8_t newState = 0; // default to off
    if ((payload[0] - '1') == 0)
    {
        newState = 1;
    }

    digitalWrite(LEDPIN, newState);
    operateSocket(socketNumber, newState);
}

//0-15, 0,1
void operateSocket(uint8_t socketID, uint8_t state)
{
    //this is a blocking routine so need to keep checking messages and

    u8g2.clearBuffer();
    u8g2.setCursor(20, 20);
    delay(10);
    printOWithVal("Socket : ", socketNumber + 1); //physical socket number

    u8g2.setCursor(55, 40);
    if (state == 0)
    {
        u8g2.print("OFF");
    }
    else
    {
        u8g2.print("ON");
    }
    delay(10);
    u8g2.sendBuffer();

    for (int i = 0; i < 2; i++)
    { // turn socket off
        //	processMessage();
        //	updateDisplay();
        transmitter.sendUnit(socketID, state);
    }

    Serial.println(F("OK - TX socket state updated"));
}

void reconnectWiFi()
{
    // Loop until we're reconnected
    //check is psclient is connected first
    // attempt to connect to Wifi network:
    printO(1, 20, "Connect WiFi..");

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(10000);
    }
}

void reconnectPSClient()
{
    // Loop until we're reconnected
    //check is psclient is connected first
    while (!psclient.connected())
    {
        printO(1, 20, "Connect MQTT..");
        Serial.println("Attempting MQTT connection...");
        // Attempt to connect
        //        if (psclient.connect("ESP32Client","",""))
        if (psclient.connect("ESP32Client"))
        {
            Serial.println("connected to MQTT server");
            // Once connected, publish an announcement...
            psclient.publish("outTopic", "hello world");
            // ... and resubscribe
            //psclient.subscribe("inTopic");
            psclient.subscribe(subscribeTopic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(psclient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
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

void printO(int x, int y, char *text)
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