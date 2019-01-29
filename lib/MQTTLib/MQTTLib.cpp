#include "WebSocketLib.h"

bool MQTTNewData = false;
int MQTTNewState = 0;     // 0 or 1
int MQTTSocketNumber = 0; // 1-16

// MQTT stuff
//IPAddress mqttBroker(192, 168, 0, 200);
char subscribeTopic[] = "433Bridge/cmnd/#";
char publishTempTopic[] = "433Bridge/Temperature";
char publishHumiTopic[] = "433Bridge/Humidity";

// array to enable translation from socket ID (0-15) to string representing
// socket function
const char *socketIDFunctionStrings[16];
// strcpy(socketIDFunctionStrings[0], "blah");
void MQTTLibSetup(void)
{
    socketIDFunctionStrings[0] = "X Lights";
    socketIDFunctionStrings[1] = "SKT 2";
    socketIDFunctionStrings[2] = "L Lights";
    socketIDFunctionStrings[3] = "D Lights";
    socketIDFunctionStrings[4] = "C Lights";
    socketIDFunctionStrings[5] = "DAB";
    socketIDFunctionStrings[6] = "Amp";
    socketIDFunctionStrings[7] = "TV";
    socketIDFunctionStrings[8] = "CSV Rads";
    socketIDFunctionStrings[9] = "Fan";
    socketIDFunctionStrings[10] = "SKT 11";
    socketIDFunctionStrings[11] = "SKT 12";
    socketIDFunctionStrings[12] = "SKT 13";
    socketIDFunctionStrings[13] = "Zone 1";
    socketIDFunctionStrings[14] = "Zone 3";
    socketIDFunctionStrings[15] = "Zone X-16";
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

char *getMQTTDisplayString(char *MQTTStatus)
{
    char msg[] = "This is a message placeholder";
    char socketNumber[10];

    sprintf(socketNumber, "%d", (MQTTSocketNumber));
    strcpy(msg, socketNumber);
    strcat(msg, "-");
    strcat(msg, socketIDFunctionStrings[MQTTSocketNumber - 1]);
    strcat(msg, ":");

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

#include "WebSerial.h"
extern WebSerial myWebSerial;
// MQTTclient call back if mqtt messsage rxed
void MQTTRxcallback(char *topic, byte *payload, unsigned int length)
{
    uint8_t socketNumber = 0;

    // Power<x> 		Show current power state of relay<x> as On or
    // Off Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
    // 1 / on 	Turn relay<x> power On handle message arrived mqtt
    // do some extra checking on rxed topic and payload?
    payload[length] = '\0';

    char message[] = "MQTT rxed [this/is/the/topic/for/this/mesage] : and finally the payload, and a bit extra to make sure there is room in the string";
    strcpy(message, "MQTT Recieved [");
    strcat(message, topic);
    strcat(message, "] : ");
    //copy on payload and add \o terminator
    strncat(message, (char *)payload, length);
    //Serial.println(message);
    myWebSerial.println(message);

    // e.g topic = "433Bridge/cmnd/Power1" to "...Power16", and payload = 1 or 0
    // either match whole topic string or trim off last 1or 2 chars and
    // convert to a number, convert last 1-2 chars to socket number
    char lastChar =
        topic[strlen(topic) - 1]; // lst char will always be a digit char
    // see if last but 1 is also a digit char - ie number has two digits - 10 to 16
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
    // signal a new command has been rxed and
    // topic and payload also available
    MQTTNewState = newState;         // 0 or 1
    MQTTSocketNumber = socketNumber; // 1-16
    MQTTNewData = true;
}

#include "LedFader.h"
extern LedFader warnLED;
#include "NewRemoteTransmitter.h"
extern NewRemoteTransmitter transmitter;
// 0-15, 0,1
// mqtt funct to operate a remote power socket
// only used by mqtt function
void operateSocket(uint8_t socketID, uint8_t state)
{
    // this is a blocking routine !!!!!!!
    char msg[40] = "SSS == Operate Socket: ";
    char buff[10];

    // strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (socketID + 1));
    strcat(msg, buff);
    strcat(msg, "-");

    //add socket item name
    strcat(msg, socketIDFunctionStrings[socketID]);

    // u8g2.setCursor(55, 40);
    if (state == 0)
    {
        strcat(msg, " - OFF");
    }
    else
    {
        strcat(msg, " - ON");
    }
    //Serial.println(msg);
    myWebSerial.println(msg);

    warnLED.fullOn();
    transmitter.sendUnit(socketID, state);
    warnLED.fullOff(); //}
}

#include <PubSubClient.h>
extern PubSubClient MQTTclient;

boolean reconnect()
{
    if (MQTTclient.connect("ESP32Client"))
    {
        // Once connected, publish an announcement...
        //MQTTclient.publish("outTopic", "hello world");
        // ... and resubscribe
        MQTTclient.connect("ESP32Client");
    }
    return MQTTclient.connected();
}



// set so ensures initial connect attempt, assume now gives 0
long lastReconnectAttempt = -6000;

void connectMQTT()
{
    bool MQTTConnectTimeout = false;
    u16_t startMillis = millis();
    u16_t timeOutMillis = 1000;
    // Loop until we're reconnected
    // check is MQTTclient is connected first

    // if (!MQTTclient.connected())
    // {
    //     long now = millis();
    //     if (now - lastReconnectAttempt > 5000)
    //     {
    //         lastReconnectAttempt = now;
    //         // Attempt to reconnect
    //         if (MQTTclient.connect("ESP32Client"))
    //         {
    //   MQTTclient.connect("ESP32Client")
    //             lastReconnectAttempt = 0;
    //         }
    //     }
    // }
    // return;

    startMillis = millis();
    MQTTConnectTimeout = false;
    long now = millis();
    myWebSerial.println("In MQTT connectMQTT...");

    if (now - lastReconnectAttempt > 5000)
    {
        myWebSerial.println("ready to try MQTT reconnectMQTT...");

        //loop till connected or timed out
        while (!MQTTclient.connected() && !MQTTConnectTimeout)
        {
            myWebSerial.println("Attempting MQTT connection...");

            lastReconnectAttempt = now;
            if (MQTTclient.connect("ESP32Client"))
            {
                myWebSerial.println("connected to MQTT server");
                MQTTclient.subscribe(subscribeTopic);
                lastReconnectAttempt = 0;
            }
            else
            {
                myWebSerial.println("failed, rc=");
                Serial.println(MQTTclient.state());
                myWebSerial.println(" try again ..");
            }
        }
        MQTTConnectTimeout =
            ((millis() - startMillis) > timeOutMillis) ? true : false;
    }

    (!MQTTConnectTimeout) ? myWebSerial.println("MQTT Connection made!")
                          : myWebSerial.println("MQTT Connection attempt Time Out!");
}
