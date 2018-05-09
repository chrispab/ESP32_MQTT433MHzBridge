//work only using wifiesp lib as connection to net

//set in software serial.h
//#define _SS_MAX_RX_BUFF 256 // RX buffer size

//#include <SPI.h>
//#include <WiFiEsp.h>
#include <WiFi.h>
#include <NewRemoteTransmitter.h>
//#include "WiFiEspUdp.h"
#include <TimeLib.h>

#include <PubSubClient.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping

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

char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
//int keyIndex = 0;                                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

//#include "SoftwareSerial.h"
//SoftwareSerial ESPSerial(7, 6); // RX, TX

//WiFiEspServer server(80);

IPAddress mqttserver(192, 168, 0, 200);

//WiFiEspClient client = server.available();
//EthernetClient ethClient;

//282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, 4); // tx address, pin for tx
byte socket = 3;
bool state = false;

uint8_t socketNumber = 0;

#define LEDPIN 2 //esp32 on board blue LED

#define ESP_BAUD 9600
#define CR Serial.println()

#define subscribeTopic "433Bridge/cmnd/#"

WiFiClient WiFiEClient;
PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);
#define SW_VERSION "V0.1"

////////////////////ntp ///////////////////////////////////

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
unsigned long interval = 60000;

void setup()
{ //Initialize serial monitor port to PC and wait for port to open:
    Serial.begin(115200);
    //Serial.begin(9600);

    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    u8g2.begin();

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_8x13_tf);

    // 17 chars by 3 lines at this font size
    u8g2.drawStr(30, 10, "MQTT ESP32");
    u8g2.drawStr(15, 21, "433Mhz Bridge");
    u8g2.drawStr(45, 32, SW_VERSION);

    u8g2.sendBuffer();

    delay(4000);

    // initialize ESP module
    WiFi.begin();

    printWifiStatus();

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(5000);
    }
    //server.begin();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    //delay(5000);
    reconnectPSClient();

    //Udp.begin(localPort);
    //Serial.println("prepre");
    //doNtp();
}

// send an NTP request to the time server at the given address
void sendNTPpacket(char *ntpSrv)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)

    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(ntpSrv, 123); //NTP requests are to port 123

    Udp.write(packetBuffer, NTP_PACKET_SIZE);

    Udp.endPacket();
}

void doNtp()
{
    Serial.println("pre");

    sendNTPpacket(timeServer); // send an NTP packet to a time server
    Serial.println("post");
    // wait for a reply for UDP_TIMEOUT miliseconds
    unsigned long startMs = millis();
    while (!Udp.available() && (millis() - startMs) < UDP_TIMEOUT)
    {
    }

    Serial.println(Udp.parsePacket());
    if (Udp.parsePacket())
    {
        Serial.println("packet received");
        // We've received a packet, read the data from it into the buffer
        Udp.read(packetBuffer, NTP_PACKET_SIZE);

        // the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        Serial.print("Seconds since Jan 1 1900 = ");
        Serial.println(secsSince1900);

        // now convert NTP time into everyday time:
        Serial.print("Unix time = ");
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        // print Unix time:
        Serial.println(epoch);

        // print the hour, minute and second:
        Serial.print("The UTC time is ");      // UTC is the time at Greenwich Meridian (GMT)
        Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
        Serial.print(':');
        if (((epoch % 3600) / 60) < 10)
        {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
        Serial.print(':');
        if ((epoch % 60) < 10)
        {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.println(epoch % 60); // print the second
    }
    // wait ten seconds before asking for the time again
    delay(10000);
}

void loop()
{
    psclient.loop();

    //maybe check every n secs to see if client is connected and reconnect if reqd

    currentMillis = millis();
    if (currentMillis - previousMillis > interval)
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

        // Serial.println("Checking if psclient needs reconnect"); // save the last time looped
        // if (!psclient.connected())
        // {
        //     Serial.println("Needs reconnect");
        //     reconnectPSClient();
        // }
        // else
        // {
        //     Serial.println("OK - still connected");
        //     Serial.println(millis());
        // }

        previousMillis = currentMillis;
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

    transmitter.sendUnit(socketNumber, newState); // default 4 tx to socket
    //operateSocket(socketNumber, newState);
    //LEDBlink(LEDPIN, socketNumber); // blink socketNumber times

    digitalWrite(LEDPIN, newState);
}

void operateSocket(uint8_t socketID, uint8_t state)
{
    //this is a blocking routine so need to keep checking messages and
    //updating vars etc
    // but do NOT do manage restarts as could be recursive and call this routine again.

    //if (transmitEnable == 1)
    //{
    Serial.println(F("txing to socket"));
    //badLED();
    //beep(1, 2, 1);
    for (int i = 0; i < 2; i++)
    { // turn socket off
        //	processMessage();
        //	updateDisplay();
        transmitter.sendUnit(socketID, state);
    }

    Serial.println(F("socket state updated"));
    //printD2Str("Power on :", devices[deviceID].name);

    // for (int i = 0; i < 3; i++)
    // { // turn socket back on
    // //	processMessage();
    // //	updateDisplay();
    // 	transmitter.sendUnit(socketID, true);
    // }
    // processMessage();
    // updateDisplay();
    // LEDsOff();
    // //beep(1, 2, 1);
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
        delay(5000);
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


