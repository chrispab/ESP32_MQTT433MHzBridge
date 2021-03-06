#include "WebSocketLib.h"
#include "config.h"
bool MQTTNewData = false;
int MQTTNewState = 0;      // 0 or 1
int MQTTSocketNumber = 1;  // 1-16

// MQTT stuff
// IPAddress mqttBroker(192, 168, 0, 200);
char subscribeTopic[] = "433Bridge/cmnd/#";
char subscribeTopic2[] = "Zone1/HeartBeat";
char subscribeTopic3[] = "Zone3/HeartBeat";
// char subscribeTopic[] = "#";

char publishTempTopic[] = "433Bridge/Temperature";
char publishHumiTopic[] = "433Bridge/Humidity";
// char publishLWTTopic[] = "433Bridge/LWT";

#include "socketIDFS.h"

// strcpy(socketIDFunctionStrings[0], "blah");

#include <NTPClient.h>
// extern NTPClient timeClient;
// extern void storeREST(char *, char *, char *);
#include "WebSerial.h"
extern WebSerial myWebSerial;

// support for hearbeat from MQTT message
#include "ZoneController.h"
extern ZoneController ZCs[];
//#include "SupportLib.h"
// static char messageText[21];
extern char *getTimeStr();

// MQTTclient call back if mqtt messsage rxed (cos has been subscribed  to)
void MQTTRxcallback(char *topic, byte *payload, unsigned int length) {
    uint8_t socketNumber = 0;

    // Power<x> 		Show current power state of relay<x> as On or
    // Off Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
    // 1 / on 	Turn relay<x> power On handle message arrived mqtt
    //! TODO do some extra checking on rxed topic and payload?
    // payload[length] = '\0';

    // format and display the whole MQTT message and payload
    char fullMQTTmessage[255];  // = "MQTT rxed thisisthetopicforthismesage and
                                // finally the payload, and a bit extra to make
                                // sure there is room in the string and even more
                                // chars";
    strcpy(fullMQTTmessage, "MQTT Rxed [");
    strcat(fullMQTTmessage, topic);
    strcat(fullMQTTmessage, "]:");
    // append payload and add \o terminator
    strcat(fullMQTTmessage, "[");
    strncat(fullMQTTmessage, (char *)payload, length);
    strcat(fullMQTTmessage, "]");

    Serial.print("fullMQTTmessage: ");
    Serial.println(fullMQTTmessage);
#ifdef DEBUG_WSERIAL

    myWebSerial.println(fullMQTTmessage);
    // Serial.print("1..");
#endif
    //! now store the topic and payload VIA REST POST to remote site DB
    // get the time mesage published - use now!//then add 3dp precision by
    // interrogating millis() for thousands of a sec (modulo????)

    // String published_at = timeClient.getFormattedDateTime();
    // TODO remote storage proven - remove it now
    // storeREST(topic, (char *)payload, (char *)published_at.c_str());

    // look for and process MQTT strings - if subscribed to, to act as additional
    // heartbeat from zones
    if (strstr(topic, "Zone1/HeartBeat") != NULL) {
        ZCs[0].resetZoneDevice();
        myWebSerial.print(getTimeStr());
        myWebSerial.println("+> GGG MQTT HeartBeat Rxed");
        // strcpy(messageText, ZCs[0].heartBeatText);
    }
    if (strstr(topic, "Zone3/HeartBeat") != NULL) {
        ZCs[2].resetZoneDevice();
        myWebSerial.print(getTimeStr());
        myWebSerial.println("+> SSS MQTT HeartBeat Rxed");
        // strcpy(messageText, ZCs[0].heartBeatText);
    }

    // only proces if topic starts with "433Bridge/cmnd/Power"
    if (strstr(topic, "433Bridge/cmnd/Power") != NULL) {
        // e.g incoming topic = "433Bridge/cmnd/Power1" to "...Power16", and payload
        // = 1 or 0 either match whole topic string or trim off last 1or 2 chars and
        //! now superceded, so payload will be "ON" or "OFF"
        // convert to a number, convert last 1-2 chars to socket number
        char lastChar =
            topic[strlen(topic) - 1];  // lst char will always be a digit char
        char lastButOneChar =
            topic[strlen(topic) - 2];  // see if last but 1 is also a digit char -
                                       // ie number has two digits - 10 to 16

        socketNumber = lastChar - '0';         // get actual numeric value
        if ((lastButOneChar == '1')) {         // it is a 2 digit number
            socketNumber = socketNumber + 10;  // calc actual int
        }

        // if ((payload[0] - '1') == 0) {
        //   newState = 1;
        // }

        //display payload
        Serial.print("......payload[");
        for (int i = 0; i < length; i++) {
            Serial.print((char)payload[i]);
        }
        Serial.println("]");

        uint8_t newState = 0;             // default to off
        if ((char)(payload[1]) == 'N') {  // the N in "ON"
            newState = 1;
        }
        //or with 0 or 1 integers
        if ((payload[0] - '1') == 0) {
            newState = 1;
        }

        Serial.print("new state: [");
        Serial.print(newState);
        Serial.println("]");

        // signal a new command has been rxed and
        // topic and payload also available
        MQTTNewState = newState;          // 0 or 1
        MQTTSocketNumber = socketNumber;  // 1-16
        MQTTNewData = true;
        return;
    }
    MQTTNewData = false;
}

void MQTTLibSetup(void) {}
#include "WebSerial.h"
extern WebSerial myWebSerial;
#include "My433Transmitter.h"
extern My433Transmitter transmitter;
void processMQTTMessage(void) {
    // char msg[40] = "SSS == Operate Socket: ";
    char buff[10];

    // strcpy(buff, "Socket : ");
    // if socket number is  valid one -
    sprintf(buff, "%d", (MQTTSocketNumber));
    // strcat(msg, buff);
    // strcat(msg, "-");

    if (MQTTNewData) {
        digitalWrite(ESP32_ONBOARD_BLUE_LED_PIN, MQTTNewState);
        // Serial
        // myWebSerial.println("process MQTT - MQTTSocketNumber...");

        // myWebSerial.println(buff);

        //                     myWebSerial.println("process MQTT -
        //                     MQTTNewState...");
        // sprintf(buff, "%d", (MQTTNewState));

        // myWebSerial.println(buff);

        transmitter.operateSocket(MQTTSocketNumber - 1, MQTTNewState);
        MQTTNewData = false;  // indicate not new data now, processed
    }
}

char *getMQTTDisplayString(char *MQTTStatus) {
    char msg[] = "This is a message placeholder with chars for space";
    char socketNumber[] = "This is a also message placeh";

    sprintf(socketNumber, "%d", (MQTTSocketNumber));

    strcpy(msg, socketNumber);

    strcat(msg, "-");
    strcat(msg, socketIDFunctionStrings[MQTTSocketNumber - 1]);
    strcat(msg, ":");

    if (MQTTNewState == 0) {
        strcat(msg, " OFF");
    } else {
        strcat(msg, " ON");
    }
    // Serial.println(msg);
    strcpy(MQTTStatus, msg);
    return MQTTStatus;
}

#include <PubSubClient.h>
extern PubSubClient MQTTclient;

// set so ensures initial connect attempt, assume now gives 0

void connectMQTT() {
    bool MQTTConnectTimeout = false;
    unsigned long checkPeriodMillis = 20000;
    unsigned long timeOutMillis = 5000;
    unsigned long now;
    unsigned long nowMillis = millis();
    static unsigned long lastReconnectAttemptMillis =
        nowMillis - checkPeriodMillis - 1000;

    myWebSerial.println("HELLO...");
    myWebSerial.println("nowMillis : ", nowMillis);
    myWebSerial.println("Last reconn attempt : ", lastReconnectAttemptMillis);
    myWebSerial.println("checkPeriodMillis : ", checkPeriodMillis);

    // do on start up
    // if (lastReconnectAttemptMillis == 0)
    // {
    //     nowMillis = checkPeriodMillis + 1;
    // }

    if ((nowMillis - lastReconnectAttemptMillis) > checkPeriodMillis) {
        myWebSerial.println("ready to try MQTT reconnectMQTT...");
        while (!MQTTclient.connected() &&
               !MQTTConnectTimeout)  // loop till connected or timed out
        {
            myWebSerial.println("Attempting MQTT connection...");
            // if (MQTTclient.connect("433BridgeMQTTClient"))  // failure will insert
            // a delay,poss 15 secs boolean connect(const char* id, const char*
            // willTopic, uint8_t willQos, boolean willRetain, const char*
            // willMessage);
            if (MQTTclient.connect("433BridgeMQTTClient", "433Bridge/LWT", 1, true, "Offline")) {
                myWebSerial.println("connected to MQTT server");
                MQTTclient.publish("433Bridge/LWT", "Online", true);  // ensure send online
                // MQTTclient.publish(publishLWTTopic, "Online");
                MQTTclient.subscribe(subscribeTopic);
                MQTTclient.subscribe(subscribeTopic2);
                MQTTclient.subscribe(subscribeTopic3);
            } else {
                myWebSerial.println("MQTT connection failed, rc=");
                Serial.println(MQTTclient.state());
                myWebSerial.println("MQTT STATE : ", MQTTclient.state());

                myWebSerial.println(" try again ..");
            }
            now = millis();
            lastReconnectAttemptMillis = now;
            MQTTConnectTimeout = ((now - nowMillis) > timeOutMillis) ? true : false;
        }
        (!MQTTConnectTimeout)
            ? myWebSerial.println("MQTT Connection made!")
            : myWebSerial.println("MQTT Connection attempt Time Out!");

        now = millis();
        lastReconnectAttemptMillis = now;
        // MQTTclient.publish(publishLWTTopic, "Online");//ensure send online
    }

    // MQTTclient.publish(publishLWTTopic, "Online");//ensure send online
}


/*
   Return the quality (Received Signal Strength Indicator)
   of the WiFi network.
   Returns a number between 0 and 100 if WiFi is connected.
   Returns -1 if WiFi is disconnected.
*/
int getQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}



unsigned long telePeriodMs = 240000;
//! publish telemetry every 5 mins , e.g. rssi info
unsigned long lastTelemetryPublish = 0-telePeriodMs;
void publishTelemetry() {
    unsigned long now = millis();
    if (now - lastTelemetryPublish > telePeriodMs) {
        lastTelemetryPublish = now;
        // Serial.println("MQTT is not connected.. trying to connect now");

        // Attempt to reconnect
        // if (MQTTclient.connect("433BridgeMQTTClient", "433Bridge/LWT", 1, true, "Offline")) {
        //     myWebSerial.println("connected to MQTT server");

        // String pubString = "{" report ":{" light ": " " + String(lightRead) + " "}}";
        char message_buff[10];
        // long rssi = WiFi.RSSI();
        String pubString = String(getQuality()); 
        pubString.toCharArray(message_buff, pubString.length() + 1);
        //Serial.println(pubString);
        //client.publish("io.m2m/arduino/lightsensor", message_buff);

        MQTTclient.publish("433Bridge/rssi", message_buff);  // ensure send online
                                                     // MQTTclient.publish(publishLWTTopic, "OnlWiFi.RSSI()ine");
                                                     // MQTTclient.subscribe(subscribeTopic);
                                                     // MQTTclient.subscribe(subscribeTopic2);
                                                     // MQTTclient.subscribe(subscribeTopic3);

        // Serial.println("MQTT is now connected....");
        // lastReconnectAttempt = 0;
        // }
    }
    // return MQTTclient.connected();
}

long lastReconnectAttempt = 0;

boolean reconnectMQTT() {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        Serial.println("MQTT is not connected.. trying to connect now");

        // Attempt to reconnect
        if (MQTTclient.connect("433BridgeMQTTClient", "433Bridge/LWT", 1, true, "Offline")) {
            myWebSerial.println("connected to MQTT server");
            MQTTclient.publish("433Bridge/LWT", "Online", true);  // ensure send online
            // MQTTclient.publish(publishLWTTopic, "Online");
            MQTTclient.subscribe(subscribeTopic);
            MQTTclient.subscribe(subscribeTopic2);
            MQTTclient.subscribe(subscribeTopic3);

            Serial.println("MQTT is now connected....");
            lastReconnectAttempt = 0;
        }
    }
    return MQTTclient.connected();
}
