#include "WebSocketLib.h"
#include "config.h"
#include "debug.h"
extern char *stringToPrint(const char *literal, const char *value);

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
extern NTPClient timeClient;
// extern void storeREST(char *, char *, char *);
#include "WebSerial.h"
extern WebSerial myWebSerial;

// support for hearbeat from MQTT message
#include "ZoneController.h"
extern ZoneController ZCs[];
// #include "SupportLib.h"
//  static char messageText[21];
// extern char *getTimeStr();

// MQTTclient call back handler if mqtt messsage rxed (cos has been subscribed  to)

/**
 * @brief Callback function to handle incoming MQTT messages.
 *
 * This function is called whenever an MQTT message is received. It processes the topic and payload,
 * logs the full message, and performs specific actions based on the topic content:
 *   - Logs the received topic and payload for debugging.
 *   - Handles heartbeat messages for specific zones (e.g., "Zone1/HeartBeat", "Zone3/HeartBeat") by resetting zone devices and logging the event.
 *   - Processes commands for relay power control if the topic matches "433Bridge/cmnd/Power", extracting the socket number and desired state ("ON"/"OFF" or "1"/"0").
 *   - Sets global flags and variables (MQTTNewState, MQTTSocketNumber, MQTTNewData) to signal that a new command has been received.
 *
 * @param topic   The topic string of the received MQTT message.
 * @param payload The payload of the MQTT message as a byte array.
 * @param length  The length of the payload.
 */
void MQTTRxcallback(char *topic, byte *payload, unsigned int length) {
    uint8_t socketNumber = 0;

    // Power<x> 		Show current power state of relay<x> as On or
    // Off Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
    // 1 / on 	Turn relay<x> power On handle message arrived mqtt
    //! TODO do some extra checking on rxed topic and payload?

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

    DEBUG_PRINTLN(stringToPrint("fullMQTTmessage: ", fullMQTTmessage));

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
        myWebSerial.print(timeClient.getTimeStr());
        myWebSerial.println("+> GGG MQTT HeartBeat Rxed");
        // strcpy(messageText, ZCs[0].heartBeatText);
    }
    if (strstr(topic, "Zone3/HeartBeat") != NULL) {
        ZCs[2].resetZoneDevice();
        myWebSerial.print(timeClient.getTimeStr());
        myWebSerial.println("+> SSS MQTT HeartBeat Rxed");
        // strcpy(messageText, ZCs[0].heartBeatText);
    }

    // only proces if topic starts with "433Bridge/cmnd/Power"
    if (strstr(topic, "433Bridge/cmnd/Power") != NULL) {
        // payload will be "ON" or "OFF"
        //  convert to a number, convert last 1-2 chars to socket number

        // lst char will always be a digit char
        char lastChar = topic[strlen(topic) - 1];

        // see if last but 1 is also a digit char -
        // ie number has two digits - 10 to 16
        char lastButOneChar = topic[strlen(topic) - 2];

        socketNumber = lastChar - '0';         // get actual numeric value
        if ((lastButOneChar == '1')) {         // it is a 2 digit number
            socketNumber = socketNumber + 10;  // calc actual int
        }

        String payloadStr = String((char *)payload, length);
        // convert payload to a string
        DEBUG_PRINTLN(stringToPrint("payloadStr: ", payloadStr.c_str()));

        uint8_t newState = 0;  // default to off
        if (payloadStr.equalsIgnoreCase("ON")) {
            newState = 1;
        }
        // check for "OFF"
        if (payloadStr.equalsIgnoreCase("OFF")) {
            newState = 0;
        }

        // or with 0 or 1 integers
        if ((payload[0] - '1') == 0) {
            newState = 1;
        }
        // if 0
        if ((payload[0] - '0') == 0) {
            newState = 0;
        }

        DEBUG_PRINTLN(stringToPrint("MQTT Rxed - newState: ", String(newState).c_str()));
        // signal a new command has been rxed and
        // topic and payload also available
        MQTTNewState = newState;          // 0 or 1
        MQTTSocketNumber = socketNumber;  // 1-16
        MQTTNewData = true;
        // Exit early as a valid command has been processed
        // Exit early after processing a valid command to avoid unnecessary checks
        return;
    }
    MQTTNewData = false;
}

void MQTTLibSetup(void) {}
#include "WebSerial.h"
extern WebSerial myWebSerial;
#include "My433Transmitter.h"
extern My433Transmitter transmitter;

/**
 * @brief Processes a newly received MQTT message and performs the corresponding action.
 *
 * This function checks if new MQTT data has been received (indicated by the MQTTNewData flag).
 * If so, it performs the following actions:
 *   - Operates the specified socket (indexed by MQTTSocketNumber - 1) with the new state.
 *   - Prints debug information about the socket number and new state.
 *   - Resets the MQTTNewData flag to indicate the data has been processed.
 *
 * This function should be called regularly to handle incoming MQTT messages and trigger hardware actions.
 */
void processMQTTRecievedMessageAction(void) {

    if (MQTTNewData) {
        transmitter.operateSocket(MQTTSocketNumber - 1, MQTTNewState);
        DEBUG_PRINTLN(stringToPrint("MQTTSocketNumber: ", String(MQTTSocketNumber).c_str()));
        DEBUG_PRINTLN(stringToPrint("MQTTNewState: ", String(MQTTNewState).c_str()));
        MQTTNewData = false;  // indicate not new data now, processed
    }
}

/**
 * @brief Constructs a display string representing the current MQTT socket state.
 *
 * This function builds a human-readable status string based on the current MQTT socket number,
 * its associated function, and its state (ON/OFF). The resulting string is copied into the
 * provided MQTTStatus buffer and a pointer to this buffer is returned.
 *
 * @param MQTTStatus A character buffer where the resulting display string will be stored.
 *                   The buffer must be large enough to hold the resulting string.
 * @return char* Pointer to the MQTTStatus buffer containing the formatted display string.
 *
 * @note This function relies on the global variables MQTTSocketNumber, MQTTNewState, and
 *       socketIDFunctionStrings. Ensure these are properly initialized before calling.
 */
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

/**
 * @brief Attempts to connect to the MQTT server with a timeout and retry mechanism.
 *
 * This function manages the MQTT connection process. It checks if enough time has passed
 * since the last connection attempt before trying to reconnect. If not already connected,
 * it repeatedly attempts to connect to the MQTT server within a specified timeout period.
 * Upon successful connection, it publishes an "Online" message to the LWT topic and subscribes
 * to predefined topics. If the connection fails, it logs the failure and retries until the
 * timeout is reached. Connection status and debug information are output via myWebSerial.
 *
 * @note This function should be called periodically (e.g., in the main loop) to maintain
 *       a persistent MQTT connection.
 */
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
int getWiFiSignalQuality() {
    if (WiFi.status() != WL_CONNECTED)
        return -1;
    int dBm = WiFi.RSSI();  // Get the RSSI value in dBm
    if (dBm <= -100)        // RSSI less than or equal to -100 dBm corresponds to 0% quality
        return 0;
    if (dBm >= -50)  // RSSI greater than or equal to -50 dBm corresponds to 100% quality
        return 100;
    return 2 * (dBm + 100);  // Scale RSSI linearly between -100 and -50 to a percentage (0-100)
}
//     return 100;
//   return 2 * (dBm - RSSI_MIN);
// }

// unsigned long telePeriodMs = 240000;
unsigned long telePeriodMs = 30000;

//! publish telemetry every 5 mins , e.g. rssi info
unsigned long lastTelemetryPublish = 0 - telePeriodMs;
void publishTelemetryIfDue() {
    unsigned long now = millis();
    if (now - lastTelemetryPublish > telePeriodMs) {
        lastTelemetryPublish = now;

        // Attempt to reconnect
        // if (MQTTclient.connect("433BridgeMQTTClient", "433Bridge/LWT", 1, true, "Offline")) {
        //     myWebSerial.println("connected to MQTT server");

        String pubString = String(getWiFiSignalQuality());
        // char message_buff[10];
        // long rssi = WiFi.RSSI();
        // String pubString = String(getQuality());
        // pubString.toCharArray(message_buff, pubString.length() + 1);

        MQTTclient.publish("433Bridge/rssi", pubString.c_str());  // ensure send online
                                                                  // MQTTclient.publish(publishLWTTopic, "OnlWiFi.RSSI()ine");
                                                                  // MQTTclient.subscribe(subscribeTopic);
                                                                  // MQTTclient.subscribe(subscribeTopic2);
                                                                  // MQTTclient.subscribe(subscribeTopic3);

        DEBUG_PRINTLN(stringToPrint("Published telemetry: (WiFi Signal Quality) 433Bridge/rssi = ", pubString.c_str()));
    }
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
