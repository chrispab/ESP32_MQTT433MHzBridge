#include <Arduino.h>
#include <DHT.h>
#include <NewRemoteTransmitter.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <WiFi.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

// classes code changes
#include "Display.h"
#include "ZoneController.h"

#include <sendemail.h>

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
void updateTempDisplay(void);
void setPipes(uint8_t *writingPipe, uint8_t *readingPipe);
void processZoneRF24Message(void);
int equalID(char *receive_payload, const char *targetID);

void updateZoneDisplayLines(void);
void updateDisplayData(void);

int freeRam(void);
void printFreeRam(void);

// DHT22 stuff
//#define DHTPIN 23 // what digital pin we're connected to
#define DHTPIN 33 // what digital pin we're connected to
DHT dht;

// WiFi settings
const char ssid[] = "notwork"; // your network SSID (name)
const char pass[] =
    "a new router can solve many problems"; // your network password
int status = WL_IDLE_STATUS;

// MQTT stuff
IPAddress mqttserver(192, 168, 0, 200);
const char subscribeTopic[] = "433Bridge/cmnd/#";
const char publishTempTopic[] = "433Bridge/Temperature";
const char publishHumiTopic[] = "433Bridge/Humidity";

WiFiClient WiFiEClient;
PubSubClient MQTTclient(mqttserver, 1883, MQTTRxcallback, WiFiEClient);

// 433Mhz settings
#define TX433PIN 32
// 282830 addr of 16ch remote
NewRemoteTransmitter transmitter(282830, TX433PIN); // tx address, pin for tx

byte socket = 3;
bool state = false;
uint8_t socketNumber = 0;

//// Set up nRF24L01 rf24Radio on SPI bus plus pins 7 & 8
#define ce_pin 5
#define cs_pin 4
RF24 rf24Radio(ce_pin, cs_pin);
uint8_t writePipeLocS[] = "NodeS";
uint8_t readPipeLocS[] = "Node0";
uint8_t writePipeLocC[] = "NodeC";
uint8_t readPipeLocC[] = "Node0";
uint8_t writePipeLocG[] = "NodeG";
uint8_t readPipeLocG[] = "Node0";
// Payload
const int max_payload_size = 32;
char receive_payload[max_payload_size +
                     1]; // +1 to allow room for a terminating NULL char
static unsigned int goodSecsMax = 15; // 20

// misc stuff
#define LEDPIN 2 // esp32 devkit on board blue LED
#define CR Serial.println()
#define TITLE_LINE1 "ESP32 MQTT"
#define TITLE_LINE2 "433Mhz Bridge"
#define TITLE_LINE3 "Wireless Dog"
#define SW_VERSION "V2.97 Br:\"OO\""

// Global vars
unsigned long currentMillis = 0;
unsigned long previousConnCheckMillis = 0;
unsigned long intervalConnCheckMillis = 63000;
unsigned long intervalTempDisplayMillis = 60000;
unsigned long previousTempDisplayMillis =
    millis() - intervalTempDisplayMillis; // trigger on start

char tempStr[17];                               // buffer for 16 chars and eos
static unsigned long dispUpdateInterval = 1000; // 1000ms

// create system objects
Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22,
                  /* data=*/21); // create the display object
ZoneController ZCs[3] = {ZoneController(0, 14, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 15, "SHD", "SSS")};
//display vars
boolean bigTempDisplayEnabled=true;

void setup() { // Initialize serial monitor port to PC and wait for port to
               // open:
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    // setup OLED display
    myDisplay.begin();
    myDisplay.setFont(u8g2_font_8x13_tf);
    myDisplay.writeLine(1, TITLE_LINE1);
    myDisplay.writeLine(3, TITLE_LINE2);
    myDisplay.writeLine(4, TITLE_LINE3);
    // myDisplay.writeLine(5, "________________");

    myDisplay.writeLine(6, SW_VERSION);
    myDisplay.refresh();
    delay(3000);
    myDisplay.wipe();

    dht.setup(DHTPIN, dht.AM2302);

    // rf24 stuff
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

    // WiFi.begin();
    // attempt to connect to Wifi network:
    connectWiFi();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    connectMQTT();

    // MQTTclient.loop(); //process any MQTT stuff
    // checkConnections();
    myDisplay.wipe();

    // create object
    SendEmail e("smtp.gmail.com", 465, "cbattisson@gmail.com",
                "fbmfbmqluzaakvso", 5000, true);
    // Send Email
    e.send("<cbattisson@gmail.com>", "<cbattisson@gmail.com>", "ESP32 Started",
           "programm started/restarted");
}

void loop() {
    updateDisplayData();

    myDisplay.refresh();

    MQTTclient.loop();   // process any MQTT stuff returned in callback
    //myDisplay.refresh();

    processZoneRF24Message(); // process any zone messages
    ZCs[0].manageRestarts(transmitter);
    // manageRestarts(1);
    // ZCs[1].manageRestarts(transmitter);
    ZCs[1].resetZoneDevice();
    // manageRestarts(2);
    ZCs[2].manageRestarts(transmitter);

    //myDisplay.refresh();
    checkConnections(); // reconnect if reqd
}

void updateDisplayData(){

    if (bigTempDisplayEnabled){

    }

    updateTempDisplay(); // get and display temp
    updateZoneDisplayLines();

}

void checkConnections() {
    currentMillis = millis();
    if (currentMillis - previousConnCheckMillis > intervalConnCheckMillis) {

        if (!WiFi.isConnected()) { //!= WL_CONNECTED)
            Serial.println("Wifi Needs reconnecting");
            connectWiFi();
        } else {
            Serial.println("OK - WiFi still connected");
        }

        if (!MQTTclient.connected()) {
            Serial.println("MQTTClient Needs reconnecting");
            connectMQTT();
        } else {
            Serial.println("OK - MQTT still connected");
        }
        previousConnCheckMillis = currentMillis;
    }
}

void connectWiFi() {
    bool wifiConnectTimeout = false;
    u16_t startMillis;
    u16_t timeOutMillis = 20000;

    // Loop until we're reconnected
    // check is MQTTclient is connected first
    // attempt to connect to Wifi network:
    // printO(1, 20, "Connect WiFi..");
    myDisplay.writeLine(5, "Connect WiFi..");
    myDisplay.refresh();
    // IMPLEMNT TIME OUT TO ALLOW RF24 WIRLESS DOG FUNC TO CONTINUE
    // while (!WiFiEClient.connected())
    // while (status != WL_CONNECTED)
    wifiConnectTimeout = false;

    startMillis = millis();
    while (!WiFi.isConnected() && !wifiConnectTimeout) {
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

    // myDisplay.writeLine(5, "Connected WiFi!");
    // myDisplay.refresh();
}

void connectMQTT() {
    bool MQTTConnectTimeout = false;
    u16_t startMillis;
    u16_t timeOutMillis = 20000;
    // Loop until we're reconnected
    // check is MQTTclient is connected first
    MQTTConnectTimeout = false;

    startMillis = millis();
    while (!MQTTclient.connected() && !MQTTConnectTimeout) {
        // printO(1, 20, "Connect MQTT..");
        myDisplay.writeLine(6, "Connect MQTT..");
        myDisplay.refresh();
        Serial.println(F("Attempting MQTT connection..."));
        // Attempt to connect
        //        if (MQTTclient.connect("ESP32Client","",""))
        if (MQTTclient.connect("ESP32Client")) {
            Serial.println(F("connected to MQTT server"));
            // Once connected, publish an announcement...
            // MQTTclient.publish("outTopic", "hello world");
            // ... and resubscribe
            // MQTTclient.subscribe("inTopic");
            MQTTclient.subscribe(subscribeTopic);
            myDisplay.writeLine(6, "Connected MQTT!");
            myDisplay.refresh();
        } else {
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

void updateTempDisplay() {
    currentMillis = millis();
    if (currentMillis - previousTempDisplayMillis > intervalTempDisplayMillis) {

        // Read temperature as Celsius (the default)
        float t = dht.getTemperature();
        float h = dht.getHumidity();
        char humiStr[] = "string to hold humidity";
        char msgStr[] = "Message String long enough?";
        strcpy(msgStr, "Temp: ");
        if (isnan(t)) {
            Serial.println("Failed to read from DHT sensor!");
            strcpy(tempStr, "-=NaN=-");
        } else { // is a number
            dtostrf(t, 4, 1, tempStr);
            MQTTclient.publish(publishTempTopic, tempStr);
            dtostrf(h, 4, 1, humiStr);
            MQTTclient.publish(publishHumiTopic, humiStr);

            strcat(tempStr, "*C");
            Serial.print("MQTT publish Temp: ");
            Serial.println(tempStr);
        }
        myDisplay.writeLine(1, strcat(msgStr, tempStr));

        Serial.print("Temp reading: ");
        Serial.println(tempStr);

        previousTempDisplayMillis = currentMillis;
    }
}

// MQTTclient call back if mqtt messsage rxed
void MQTTRxcallback(char *topic, byte *payload, unsigned int length) {
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
    for (uint8_t i = 0; i < length; i++) {
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
        (lastButOneChar <= '9')) { // is it a 2 digit number
        socketNumber =
            ((lastButOneChar - '0') * 10) + (lastChar - '0'); // calc actual int
    } else {
        socketNumber = (lastChar - '0');
    }

    int socketID =
        socketNumber - 1; // convert from 1-16 range to 0-15 range sendUnit uses
    uint8_t newState = 0; // default to off
    if ((payload[0] - '1') == 0) {
        newState = 1;
    }

    digitalWrite(LEDPIN, newState);
    operateSocket(socketID, newState);
}

// 0-15, 0,1
// mqtt funct to operate a remote power socket
// only used by mqtt function
void operateSocket(uint8_t socketID, uint8_t state) {
    // this is a blocking routine !!!!!!!
    char msg[17] = "Socket : ";
    char buff[10];

    // strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (socketID + 1));
    strcat(msg, buff);

    // u8g2.setCursor(55, 40);
    if (state == 0) {
        strcat(msg, " OFF");
    } else {
        strcat(msg, " ON");
    }
    myDisplay.writeLine(2, msg);
    myDisplay.refresh();

    // for (int i = 0; i < 2; i++)
    // { // turn socket off
    //	processZoneRF24Message();
    //	refresh();
    transmitter.sendUnit(socketID, state);
    // }
    Serial.println(msg);

    // Serial.println("OK - TX socket state updated");
}

void LEDBlink(int LPin, int repeatNum) {
    for (int i = 0; i < repeatNum; i++) {
        digitalWrite(LPin, 1); // GET /H turns the LED on
        delay(50);
        digitalWrite(LPin, 0); // GET /H turns the LED on
        delay(50);
    }
}

void printWifiStatus() {
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

void setPipes(uint8_t *writingPipe, uint8_t *readingPipe) {
    // config rf24Radio to comm with a node
    rf24Radio.stopListening();
    rf24Radio.openWritingPipe(writingPipe);
    rf24Radio.openReadingPipe(1, readingPipe);
}

void processZoneRF24Message(void) {
    char messageText[17];
    while (rf24Radio.available()) { // Read all available payloads

        // Grab the message and process
        uint8_t len = rf24Radio.getDynamicPayloadSize();

        // If a corrupt dynamic payload is received, it will be flushed
        if (!len) {
            return;
        }

        rf24Radio.read(receive_payload, len);

        // Put a zero at the end for easy printing etc
        receive_payload[len] = 0;

        // who was it from?
        // reset that timer

        if (equalID(receive_payload, ZCs[0].heartBeatText)) {
            ZCs[0].resetZoneDevice();
            Serial.println("RESET GGG");
            strcpy(messageText, ZCs[0].heartBeatText);
        } else if (equalID(receive_payload, ZCs[1].heartBeatText)) {
            ZCs[1].resetZoneDevice();
            // ZCs[1].lastGoodAckMillis = millis();
            Serial.println("RESET CCC");
            strcpy(messageText, ZCs[1].heartBeatText);
        } else if (equalID(receive_payload, ZCs[2].heartBeatText)) {
            ZCs[2].resetZoneDevice();
            // ZCs[2].lastGoodAckMillis = millis();
            Serial.println("RESET SSS");
            strcpy(messageText, ZCs[2].heartBeatText);
        } else {
            Serial.println("NO MATCH");
            // badLED();
            // LEDsOff();
            strcpy(messageText, "NO MATCH");
        }
        // writeLine(6, messageText);
    }
}

int equalID(char *receive_payload, const char *targetID) {
    // check if same 1st 3 chars
    if ((receive_payload[0] == targetID[0]) &&
        (receive_payload[1] == targetID[1]) &&
        (receive_payload[2] == targetID[2])) {
        return true;
    } else {
        return false;
    }
}

void updateZoneDisplayLines(void) {
    // all three lines can be displayed at once
    const char rebootMsg[] = {"Reboot: "};
    const char powerCycleMsg[] = {"Power Cycle"};
    int zoneID; // only initialised once at start
    static unsigned long lastDispUpdateTimeMillis = 0;

    // static unsigned long dispUpdateInterval = 1000 / dispUpdateFreq;

    unsigned int secsSinceAck = 0;
    // max secs out considered good

    char str_output[20] = {0};
    unsigned int secsLeft;
    char buf[17];
    // Serial.println(zoneID);
    // this loop
    // create info string for each zone then display it
    // setup disp
    // u8g2.clearBuffer();
    // u8g2.setFont(u8g2_font_7x14_tf);

    // check if time to display a new message updates
    if ((millis() - lastDispUpdateTimeMillis) >=
        dispUpdateInterval) { // ready to update?
        // printFreeRam();
        for (zoneID = 0; zoneID < 3; zoneID++) {
            // Serial.println(zoneID);
            secsSinceAck = (millis() - ZCs[zoneID].lastGoodAckMillis) / 1000;

            // u8g2.setCursor(0, ((zoneID + 1) * 10) + (1 *
            // zoneID));
            // make sure check for restarting device
            // if so display current secs in wait for reboot cycle
            if (ZCs[zoneID].isRebooting) {
                secsLeft = (ZCs[zoneID].rebootMillisLeft) / 1000UL;

                Serial.print("--rebootMillisLeft: ");
                Serial.println((ZCs[zoneID].rebootMillisLeft));

                Serial.print("--secsLeft var: ");
                Serial.println(secsLeft);

                // build string to show if cycling or coming
                // back up char str_output[20] = { 0 }; //,
                // str_two[]="two"; start with device name
                strcpy(str_output, ZCs[zoneID].name);
                strcat(str_output, ": ");
                // char message[] = " Reboot: ";
                if (ZCs[zoneID].isPowerCycling) {
                    strcat(str_output, powerCycleMsg);
                    // printD(str_output);
                    myDisplay.writeLine(zoneID + 4, str_output);
                    // secsLeft = '';
                } else {
                    strcat(str_output, rebootMsg);
                    // printDWithVal(str_output, secsLeft);
                    sprintf(buf, "%d", secsLeft);
                    strcat(str_output, buf);

                    myDisplay.writeLine(zoneID + 4, str_output);
                }
            } else if ((secsSinceAck > goodSecsMax)) {
                strcpy(str_output, ZCs[zoneID].name);
                strcat(str_output, ": ");
                strcat(str_output, ZCs[zoneID].badStatusMess);
                strcat(str_output, ": ");

                // printDWithVal(str_output, secsSinceAck);

                sprintf(buf, "%d", secsSinceAck);
                strcat(str_output, buf);
                myDisplay.writeLine(zoneID + 4, str_output);
                // badLED();
                // LEDsOff();
            } else {
                // u8g2.print("u");
                strcpy(str_output, ZCs[zoneID].name);
                strcat(str_output, ": ");
                strcat(str_output, ZCs[zoneID].goodStatusMess);
                // add restarts soince power on
                strcat(str_output, " (");

                sprintf(buf, "%i", ZCs[zoneID].powerCyclesSincePowerOn);

                strcat(str_output, buf);
                strcat(str_output, ")");
                myDisplay.writeLine(zoneID + 4, str_output);
                // goodLED();
            }
        }
        lastDispUpdateTimeMillis = millis();
        myDisplay.refresh();
    }
}
