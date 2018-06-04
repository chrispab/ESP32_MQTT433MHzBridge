#include <Arduino.h>
#include <WiFi.h>
#include <NewRemoteTransmitter.h>
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
void resetDevice(int deviceID);


//OLED display stuff
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21); // ESP32 Thing, HW I2C with pin remapping
#define LINE_HIEGHT 10
#define XPIX 128
#define YPIX 64
#define DISPLAY_LINES 6
char displayLine[6][17];  //6 lines of 16 chars +eol for dispaly store
int dispUpdateFreq = 1.5; // how many updates per sec

//DHT22 stuff
//#define DHTPIN 23 // what digital pin we're connected to
#define DHTPIN 33 // what digital pin we're connected to
DHT dht;

//WiFi settings
const char ssid[] PROGMEM = "notwork";                              // your network SSID (name)
const char pass[] PROGMEM = "a new router can solve many problems"; // your network password
int status = WL_IDLE_STATUS;

//MQTT stuff
IPAddress mqttserver(192, 168, 0, 200);
const char subscribeTopic[] PROGMEM = "433Bridge/cmnd/#";
WiFiClient WiFiEClient;
PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);

//433Mhz settings
#define TX433PIN 32
//282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, TX433PIN); // tx address, pin for tx
byte socket = 3;
bool state = false;
uint8_t socketNumber = 0;

//// Set up nRF24L01 radio on SPI bus plus pins 7 & 8
#define ce_pin 5
#define cs_pin 4
RF24 radio(ce_pin, cs_pin);
uint8_t writePipeLocS[] PROGMEM = "NodeS";
uint8_t readPipeLocS[] PROGMEM = "Node0";
uint8_t writePipeLocC[] PROGMEM = "NodeC";
uint8_t readPipeLocC[] PROGMEM = "Node0";
uint8_t writePipeLocG[] PROGMEM = "NodeG";
uint8_t readPipeLocG[] PROGMEM = "Node0";
// Payload
const int max_payload_size = 32;
char receive_payload[max_payload_size + 1];                   // +1 to allow room for a terminating NULL char
static unsigned int goodSecsMax = 15;                         //20
static unsigned long maxMillisNoAckFromPi = 1000UL * 300UL;   //300 max millisces to wait if no ack from pi before power cycling pi
static unsigned long waitForPiPowerUpMillis = 1000UL * 120UL; //120

// define a struct for each controller instance with related vars
struct controller
{
    uint8_t id_number;
    uint8_t zone;
    uint8_t socketID;
    unsigned long lastGoodAckMillis;
    char name[4];
    char heartBeatText[4];   // allow enough space for text plus 1 extra for null terminator
    char badStatusMess[10];  // enough for 1 line on display
    char goodStatusMess[10]; // enough for 1 line on display
    bool isRebooting;        //indicate if booting up
    bool isPowerCycling;     //indicate if power cycling
    unsigned long rebootMillisLeft;
    unsigned long lastRebootMillisLeftUpdate;
    uint8_t powerCyclesSincePowerOn;
};
struct controller devices[3]; //create an array of 3 controller structures

//misc stuff
#define LEDPIN 2 //esp32 devkit on board blue LED
#define CR Serial.println()
#define TITLE_LINE1 "ESP32 MQTT"
#define TITLE_LINE2 "433Mhz Bridge"
#define SW_VERSION "V1.4rf24"

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

    displayWriteLine(1, TITLE_LINE1);
    displayWriteLine(2, TITLE_LINE2);
    displayWriteLine(3, SW_VERSION);
    displayRefresh();

    delay(5000);

    //WiFi.begin();
    // attempt to connect to Wifi network:
    reconnectWiFi();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    reconnectPSClient();
    dht.setup(DHTPIN);

    //rf24 stuff
    radio.begin();
    // enable dynamic payloads
    radio.enableDynamicPayloads();
    // optionally, increase the delay between retries & # of retries
    //radio.setRetries(15, 15);
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(124);
    radio.startListening();
    radio.printDetails();
    // autoACK enabled by default
    setPipes(writePipeLocC, readPipeLocC); // SHOULD NEVER NEED TO CHANGE PIPES
    radio.startListening();

    //populate the array of controller structs
    for (int i = 0; i < 3; i++)
    {
        devices[i].id_number = i; //0 to 2
        devices[i].zone = i + 1;  //1 to 3
        devices[i].lastGoodAckMillis = millis();
        devices[i].isRebooting = 0;
        devices[i].isPowerCycling = 0;
        devices[i].rebootMillisLeft = 0;
        devices[i].lastRebootMillisLeftUpdate = millis();
        devices[i].powerCyclesSincePowerOn = 0;
        strcpy(devices[i].badStatusMess, "Away");
        strcpy(devices[i].goodStatusMess, "OK");
    }
    // individual stuff
    devices[0].socketID = 14;
    strcpy(devices[0].name, "GRG");
    strcpy(devices[0].heartBeatText, "GGG");

    devices[1].socketID = 4;
    strcpy(devices[1].name, "CNV");
    strcpy(devices[1].heartBeatText, "CCC");

    devices[2].socketID = 15;
    strcpy(devices[2].name, "SHD");
    strcpy(devices[2].heartBeatText, "SSS");

    psclient.loop(); //process any MQTT stuff
    checkConnections();
    displayWipe();
}

void loop()
{
    psclient.loop();      //process any MQTT stuff
    checkConnections();   // reconnect if reqd
    updateTempDisplay();  //get and display temp
    processZoneMessage(); //process any zone messages
    manageRestarts(0);
    resetDevice(1);
    manageRestarts(2);
    updateZoneDisplayLines();
    displayRefresh();
}
void resetDevice(int deviceID)
{
	devices[deviceID].lastGoodAckMillis = millis();
	devices[deviceID].isRebooting = 0;
	devices[deviceID].rebootMillisLeft = 0;
}
void powerCycle(int deviceID)
{
    //this is a blocking routine so need to keep checking messages and
    //updating vars etc
    // but do NOT do manage restarts as could be recursive and call this routine again.
    int transmitEnable = 1;
    if (transmitEnable == 1)
    {
        Serial.println(F("sending off"));
        //badLED();
        //beep(1, 2, 1);
        for (int i = 0; i < 3; i++)
        { // turn socket off
            processZoneMessage();
            displayRefresh();
            transmitter.sendUnit(devices[deviceID].socketID, false);
        }
        processZoneMessage();
        displayRefresh();
        delay(1000);
        processZoneMessage();
        displayRefresh();
        delay(1000);
        processZoneMessage();
        displayRefresh();
        delay(1000);
        processZoneMessage();
        displayRefresh();
        // Switch Rxunit on
        Serial.println(F("sending on"));
        //printD2Str("Power on :", devices[deviceID].name);

        for (int i = 0; i < 3; i++)
        { // turn socket back on
            processZoneMessage();
            displayRefresh();
            transmitter.sendUnit(devices[deviceID].socketID, true);
        }
        processZoneMessage();
        displayRefresh();
        //LEDsOff();
        //beep(1, 2, 1);
        Serial.println(F("complete"));
    }
    else
    {
        Serial.println(F("not transmitting"));
    }
}
//check a unit and see if restart reqd
void manageRestarts(int deviceID)
{
    // now check if need to reboot a device
    if ((millis() - devices[deviceID].lastGoodAckMillis) > maxMillisNoAckFromPi)
    { // over time limit so reboot first time in then just upadte time each other time

        //printFreeRam();
        if (devices[deviceID].isRebooting == 0)
        { // all this done first time triggered
            //printD("Reboot : ");

            devices[deviceID].isRebooting = 1; //signal device is rebooting

            devices[deviceID].isPowerCycling = 1; // signal in power cycle

            powerCycle(deviceID);

            devices[deviceID].isPowerCycling = 0; // signal in power cycle
            devices[deviceID].powerCyclesSincePowerOn++;
            devices[deviceID].rebootMillisLeft = waitForPiPowerUpMillis;
            devices[deviceID].lastRebootMillisLeftUpdate = millis();

            Serial.println("triggered reboot in manage reboots");

            Serial.println(devices[deviceID].rebootMillisLeft);
            //delay(1000);
        }
        else
        { // this executes till end of reboot timer
            //device is rebooting now - do some stuff to update countdown timers
            //wait for pi to come back up - do nothing
            //millis since last update
            unsigned long millisLapsed = millis() - devices[deviceID].lastRebootMillisLeftUpdate;

            // next subtraction will take us to/over limit
            if (millisLapsed >= devices[deviceID].rebootMillisLeft)
            {
                //zero or neg reached
                devices[deviceID].rebootMillisLeft = 0;
                //timerDone = 1;
            }
            else
            { // ok to do timer subtraction

                devices[deviceID].rebootMillisLeft =
                    devices[deviceID].rebootMillisLeft - millisLapsed;
            }
            devices[deviceID].lastRebootMillisLeftUpdate = millis();

            // calc if next time subtraction takes it below zero
            //cant get a neg number from unsigned numbers used
            if (devices[deviceID].rebootMillisLeft == 0)
            { // reboot stuff completed here
                //if (timerDone == 1) { // reboot stuff completed here
                devices[deviceID].lastGoodAckMillis = millis();
                Serial.print(F("Assume Pi back up:"));
                Serial.println(deviceID);
                //printD("Assume pi back up");
                //printD2Str("Assume up:", devices[deviceID].name);
                devices[deviceID].isRebooting = 0; //signal device has stopped rebooting
            }
        }
    }
}
void checkConnections()
{
    currentMillis = millis();
    if (currentMillis - previousConnCheckMillis > intervalConCheckMillis)
    {
        Serial.println(F("Checking if WiFi or MQTT PSClient needs reconnecting")); // save the last time looped
        if (status != WL_CONNECTED)
        {
            Serial.println(F("Wifi Needs reconnecting"));
            reconnectWiFi();
        }
        else
        {
            Serial.println(F("OK - WiFi still connected"));
            //Serial.println(millis());
        }

        //Serial.println("Checking if psclient needs reconnect"); // save the last time looped
        if (!psclient.connected())
        {
            Serial.println(F("PSClient Needs reconnecting"));
            reconnectPSClient();
        }
        else
        {
            Serial.println(F("OK - MQTT PS client still connected"));
            //Serial.println(millis());
        }
        previousConnCheckMillis = currentMillis;
    }
}

//redraw the display with contents of displayLine array
void displayRefresh(void)
{
    // u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++)
    {
        u8g2.drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    delay(10);
    u8g2.sendBuffer();
}
//redraw the display with contents of displayLine array
void displayWipe(void)
{
    // u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    for (int i = 0; i < DISPLAY_LINES; i++)
    {
        strcpy(displayLine[i], " ");
        u8g2.drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    }
    delay(10);
    u8g2.sendBuffer();
}
//add-update a line of text in the display text buffer
void displayWriteLine(int lineNumber, const char *lineText)
{
    //update a line in the diaplay text buffer
    strcpy(displayLine[lineNumber - 1], lineText);
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
        char msgStr[] = "Message String long enough?";
        strcpy(msgStr, "Temp: ");
        if (isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
            strcpy(tempStr, "-=NaN=-");
        }
        else // is a number
        {
            dtostrf(t, 4, 1, tempStr);
            strcat(tempStr, "*C");
        }
        //printO(20, 40, strcat(msgStr, tempStr));

        displayWriteLine(1, strcat(msgStr, tempStr));

        Serial.print(F("Temperature: "));
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
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] : "));
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
    char msg[17] = "Socket : ";
    char buff[10];

    //strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (socketID + 1));
    strcat(msg, buff);

    //u8g2.setCursor(55, 40);
    if (state == 0)
    {
        //u8g2.print("OFF");
        //displayWriteLine(3, "OFF");
        strcat(msg, " OFF");
    }
    else
    {
        //u8g2.print("ON");
        //displayWriteLine(3, "ON");
        strcat(msg, " ON");
    }
    displayWriteLine(2, msg);
    displayRefresh();
    //delay(10);
    //u8g2.sendBuffer();

    for (int i = 0; i < 2; i++)
    { // turn socket off
        //	processZoneMessage();
        //	displayRefresh();
        transmitter.sendUnit(socketID, state);
    }

    Serial.println(F("OK - TX socket state updated"));
}

void reconnectWiFi()
{
    // Loop until we're reconnected
    //check is psclient is connected first
    // attempt to connect to Wifi network:
    //printO(1, 20, "Connect WiFi..");
    displayWriteLine(5, "Connect WiFi..");
    displayRefresh();

    while (status != WL_CONNECTED)
    {
        Serial.print(F("Attempting to connect to SSID: "));
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(10000);
    }
    displayWriteLine(5, "Connected WiFi!");
    displayRefresh();
}

void reconnectPSClient()
{
    // Loop until we're reconnected
    //check is psclient is connected first
    while (!psclient.connected())
    {
        //printO(1, 20, "Connect MQTT..");
        displayWriteLine(6, "Connect MQTT..");
        displayRefresh();
        Serial.println(F("Attempting MQTT connection..."));
        // Attempt to connect
        //        if (psclient.connect("ESP32Client","",""))
        if (psclient.connect("ESP32Client"))
        {
            Serial.println(F("connected to MQTT server"));
            // Once connected, publish an announcement...
            psclient.publish("outTopic", "hello world");
            // ... and resubscribe
            //psclient.subscribe("inTopic");
            psclient.subscribe(subscribeTopic);
            displayWriteLine(6, "Connected MQTT!");
            displayRefresh();
        }
        else
        {
            Serial.print(F("failed, rc="));
            Serial.print(psclient.state());
            Serial.println(F(" try again in 5 secs"));
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
    Serial.print(F("SSID: "));
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print(F("IP Address: "));
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print(F("signal strength (RSSI):"));
    Serial.print(rssi);
    Serial.println(F(" dBm"));
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

void setPipes(uint8_t *writingPipe, uint8_t *readingPipe)
{
    //config radio to comm with a node
    radio.stopListening();
    radio.openWritingPipe(writingPipe);
    radio.openReadingPipe(1, readingPipe);
}

void processZoneMessage(void)
{
    char messageText[17];
    while (radio.available())
    { // Read all available payloads

        // Grab the message and process
        uint8_t len = radio.getDynamicPayloadSize();

        // If a corrupt dynamic payload is received, it will be flushed
        if (!len)
        {
            return;
        }

        radio.read(receive_payload, len);

        // Put a zero at the end for easy printing
        receive_payload[len] = 0;

        //who was it from?
        //reset that timer

        if (equalID(receive_payload, devices[0].heartBeatText))
        {
            resetZoneDevice(0);
            Serial.println(F("RESET GGG"));
            strcpy(messageText, devices[0].heartBeatText);
        }
        else if (equalID(receive_payload, devices[1].heartBeatText))
        {
            resetZoneDevice(1);
            //devices[1].lastGoodAckMillis = millis();
            Serial.println(F("RESET CCC"));
            strcpy(messageText, devices[1].heartBeatText);
        }
        else if (equalID(receive_payload, devices[2].heartBeatText))
        {
            resetZoneDevice(2);
            //devices[2].lastGoodAckMillis = millis();
            Serial.println(F("RESET SSS"));
            strcpy(messageText, devices[2].heartBeatText);
        }
        else
        {
            Serial.println(F("NO MATCH"));
            //badLED();
            //LEDsOff();
            strcpy(messageText, "NO MATCH");
        }
        //displayWriteLine(6, messageText);
    }
}

void resetZoneDevice(int deviceID)
{
    devices[deviceID].lastGoodAckMillis = millis();
    devices[deviceID].isRebooting = 0;
    devices[deviceID].rebootMillisLeft = 0;
}

int equalID(char *receive_payload, const char *targetID)
{
    //check if same 1st 3 chars
    if ((receive_payload[0] == targetID[0]) && (receive_payload[1] == targetID[1]) && (receive_payload[2] == targetID[2]))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void updateZoneDisplayLines(void)
{
    //all three lines can be displayed at once
    const char rebootMsg[] PROGMEM = {"Reboot: "};
    const char powerCycleMsg[] PROGMEM = {"Power Cycle"};
    int stateCounter; // only initialised once at start
    static unsigned long lastDispUpdateTimeMillis = 0;

    static unsigned long dispUpdateInterval = 1000 / dispUpdateFreq;

    unsigned int secsSinceAck = 0;
    // max secs out considered good

    char str_output[20] = {0};
    unsigned int secsLeft;
    char buf[17];
    //Serial.println(stateCounter);
    // this loop
    //create info string for each zone then display it
    //setup disp
    //u8g2.clearBuffer();
    //u8g2.setFont(u8g2_font_7x14_tf);

    //check if time to display a new message updates
    if ((millis() - lastDispUpdateTimeMillis) >= dispUpdateInterval)
    { //ready to update?
        //printFreeRam();
        for (stateCounter = 0; stateCounter < 3; stateCounter++)
        {
            //Serial.println(stateCounter);
            secsSinceAck = (millis() - devices[stateCounter].lastGoodAckMillis) / 1000;

            //u8g2.setCursor(0, ((stateCounter + 1) * 10) + (1 * stateCounter));
            // make sure check for restarting device
            //if so display current secs in wait for reboot cycle
            if (devices[stateCounter].isRebooting)
            {
                secsLeft = (devices[stateCounter].rebootMillisLeft) / 1000UL;

                Serial.print(F("--rebootMillisLeft: "));
                Serial.println((devices[stateCounter].rebootMillisLeft));

                Serial.print(F("--secsLeft var: "));
                Serial.println(secsLeft);

                //build string to show if cycling or coming back up
                //char str_output[20] = { 0 }; //, str_two[]="two";
                //start with device name
                strcpy(str_output, devices[stateCounter].name);
                strcat(str_output, ": ");
                //char message[] = " Reboot: ";
                if (devices[stateCounter].isPowerCycling)
                {
                    strcat(str_output, powerCycleMsg);
                    //printD(str_output);
                    displayWriteLine(stateCounter + 4, str_output);
                    //secsLeft = '';
                }
                else
                {
                    strcat(str_output, rebootMsg);
                    //printDWithVal(str_output, secsLeft);
                    sprintf(buf, "%d", secsLeft);
                    strcat(str_output, buf);

                    displayWriteLine(stateCounter + 4, str_output);
                }
            }
            else if ((secsSinceAck > goodSecsMax))
            {
                strcpy(str_output, devices[stateCounter].name);
                strcat(str_output, ": ");
                strcat(str_output, devices[stateCounter].badStatusMess);
                strcat(str_output, ": ");

                //printDWithVal(str_output, secsSinceAck);

                sprintf(buf, "%d", secsSinceAck);
                strcat(str_output, buf);
                displayWriteLine(stateCounter + 4, str_output);
                //badLED();
                //LEDsOff();
            }
            else
            {
                //u8g2.print("u");
                strcpy(str_output, devices[stateCounter].name);
                strcat(str_output, ": ");
                strcat(str_output, devices[stateCounter].goodStatusMess);
                //add restarts soince power on
                strcat(str_output, " (");

                sprintf(buf, "%i",
                        devices[stateCounter].powerCyclesSincePowerOn);

                strcat(str_output, buf);
                strcat(str_output, ")");
                //printD(str_output);
                displayWriteLine(stateCounter + 4, str_output);
                //goodLED();
            }
        }
        lastDispUpdateTimeMillis = millis();
        //u8g2.sendBuffer();
    }
}

// int freeRam()
// {
// 	extern int __heap_start, *__brkval;
// 	int v;

// 	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
// }

// void printFreeRam(void)
// {
// 	Serial.print(F("FR: "));
// 	Serial.println(freeRam());
// }
