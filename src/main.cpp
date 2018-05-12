#include <Arduino.h>
//#include <SPI.h>
//#include <WiFiEsp.h>
#include <WiFi.h>
#include <NewRemoteTransmitter.h>
//#include "WiFiEspUdp.h"
//#include <TimeLib.h>

#include <PubSubClient.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping

////////DHT22 stuff////////////////////////////
#include "DHT.h"

#define DHTPIN 23 // what digital pin we're connected to

DHT dht;
/////////////////////////////////////////////////

//forward decs
void printWifiStatus();
boolean processRequest(String &getLine);
void sendResponse(WiFiClient client);
void listenForClients(void);
void LEDBlink(int LEDPin, int repeatNum);
void callback(char *topic, byte *payload, unsigned int length);
void reconnectPSClient();
void reconnectWiFiEsp();
void operateSocket(uint8_t socketID, uint8_t state);

void printD(const char *message);
void printDWithVal(const char *message, int value);
void printD2Str(const char *str1, const char *str2);

char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
//int keyIndex = 0;                                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

IPAddress mqttserver(192, 168, 0, 200);

#define TX433PIN 32
//282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, TX433PIN); // tx address, pin for tx
byte socket = 3;
bool state = false;

uint8_t socketNumber = 0;

#define LEDPIN 2 //esp32 on board blue LED

//#define ESP_BAUD 9600
#define CR Serial.println()

#define subscribeTopic "433Bridge/cmnd/#"

WiFiClient WiFiEClient;
PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);
#define SW_VERSION "V0.5"

////////////////////ntp ///////////////////////////////////

#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

char timeServer[] = "time.nist.gov"; // NTP server
//unsigned int localPort = 2390;        // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT = 2000;   // timeout in miliseconds to wait for an UDP packet to arrive

byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";

const int timeZone = 1; // Central European Time

unsigned int localPort = 8888; // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

///////////////////////////////////////////////

void sendNTPpacket(char *ntpSrv);
void doNtp();

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long intervalMillis = 30000;

void setup()
{ //Initialize serial monitor port to PC and wait for port to open:
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    u8g2.drawStr(30, 10, "MQTT ESP32");
    u8g2.drawStr(15, 21, "433Mhz Bridge");
    u8g2.drawStr(45, 32, SW_VERSION);
    u8g2.sendBuffer();
    delay(4000);

    // initialize ESP module
    WiFi.begin();
    //    printWifiStatus();
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
        // wait 5 seconds for connection:
        delay(10000);
    }
    //server.begin();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    reconnectPSClient();

    dht.setup(DHTPIN);
}

void loop()
{
    psclient.loop();

    //maybe check every n secs to see if client is connected and reconnect if reqd

    currentMillis = millis();
    if (currentMillis - previousMillis > intervalMillis)
    {
        Serial.println("Checking if WiFi needs reconnect"); // save the last time looped
        if (status != WL_CONNECTED)
        {
            Serial.println("Wifi Needs reconnect");
            reconnectWiFiEsp();
        }
        else
        {
            Serial.println("OK - WiFi still connected");
            Serial.println(millis());
        }

        Serial.println("Checking if psclient needs reconnect"); // save the last time looped
        if (!psclient.connected())
        {
            Serial.println("Needs reconnect");
            reconnectPSClient();
        }
        else
        {
            Serial.println("OK - ps client still connected");
            Serial.println(millis());
        }

        previousMillis = currentMillis;

        // Read temperature as Celsius (the default)
        float t = dht.getTemperature();
        char tempStr[9]; //="12345678";buffer
        // Check if any reads failed and exit early (to try again).
        if (isnan(t))
        {
            Serial.println("Failed to read from DHT sensor!");
            //return;
        }

        if (!isnan(t))
        {
            dtostrf(t, 5, 2, tempStr);
            strcat(tempStr,"*C");
        }
        else
        {
            strcpy(tempStr, "-=NaN=-");
        }

        u8g2.clearBuffer();
        u8g2.setCursor(20, 40);
        //delay(10);
        printD2Str("Temp: ", tempStr); 
        //u8g2.print("*C");
        delay(10);
        u8g2.sendBuffer();

        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C ");
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
    Serial.print("] ");
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

    Serial.println(millis());

    //maybe call a few times
    Serial.println(F("txing to socket"));
    //u8g2.clearBuffer();

    Serial.println("txing state to socket");

    digitalWrite(LEDPIN, newState);
    //transmitter.sendUnit(socketNumber, newState); // default 4 tx to socket
    operateSocket(socketNumber, newState);
    //LEDBlink(LEDPIN, socketNumber); // blink socketNumber times
}

//0-15, 0,1
void operateSocket(uint8_t socketID, uint8_t state)
{
    //this is a blocking routine so need to keep checking messages and
    //updating vars etc

    u8g2.clearBuffer();
    u8g2.setCursor(20, 20);
    //delay(10);
    printDWithVal("Socket : ", socketNumber + 1); //physical socket number

    u8g2.setCursor(55, 40);
    if (state == 0)
    {
        u8g2.print("OFF");
    }
    else
    {
        u8g2.print("ON");
    }

    //delay(10);
    //printDWithVal("Socket : ", socketNumber + 1); //physical socket number
    delay(10);
    u8g2.sendBuffer();

    for (int i = 0; i < 2; i++)
    { // turn socket off
        //	processMessage();
        //	updateDisplay();
        transmitter.sendUnit(socketID, state);
    }

    Serial.println(F("socket state updated"));

    Serial.println(F("OK"));
    // }
    // else
    // {
    // 	Serial.println(F("not transmitting"));
    // }
}

void reconnectWiFiEsp()
{
    // Loop until we're reconnected
    //check is psclient is connected first
    // attempt to connect to Wifi network:
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
        Serial.println("Attempting MQTT connection...");
        // Attempt to connect
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
void printD(const char *message)
{
    u8g2.print(message);
}

void printDWithVal(const char *message, int value)
{
    u8g2.print(message);
    u8g2.print(value);
}

void printD2Str(const char *str1, const char *str2)
{
    u8g2.print(str1);
    u8g2.print(str2);
}