#include "WebSocketLib.h"

bool MQTTNewData = false;
int MQTTNewState = 0;     // 0 or 1
int MQTTSocketNumber = 1; // 1-16

// MQTT stuff
//IPAddress mqttBroker(192, 168, 0, 200);
char subscribeTopic[] = "433Bridge/cmnd/#";
char publishTempTopic[] = "433Bridge/Temperature";
char publishHumiTopic[] = "433Bridge/Humidity";

#include "socketIDFS.h"



                
// strcpy(socketIDFunctionStrings[0], "blah");
void MQTTLibSetup(void)
{
    // socketIDFunctionStrings[0] = "X Lights";
    // socketIDFunctionStrings[1] = "SKT 2";
    // socketIDFunctionStrings[2] = "L Lights";
    // socketIDFunctionStrings[3] = "D Lights";
    // socketIDFunctionStrings[4] = "C Lights";
    // socketIDFunctionStrings[5] = "DAB";
    // socketIDFunctionStrings[6] = "Amp";
    // socketIDFunctionStrings[7] = "TV";
    // socketIDFunctionStrings[8] = "CSV Rads";
    // socketIDFunctionStrings[9] = "Fan";
    // socketIDFunctionStrings[10] = "SKT 11";
    // socketIDFunctionStrings[11] = "SKT 12";
    // socketIDFunctionStrings[12] = "SKT 13";
    // socketIDFunctionStrings[13] = "Zone 1";
    // socketIDFunctionStrings[14] = "Zone 3";
    // socketIDFunctionStrings[15] = "Outer Sensor";
}
#include "WebSerial.h"
extern WebSerial myWebSerial;
#include "My433Transmitter.h"
extern My433Transmitter transmitter;
void processMQTTMessage(void)
{

    //char msg[40] = "SSS == Operate Socket: ";
    char buff[10];

    // strcpy(buff, "Socket : ");
    sprintf(buff, "%d", (MQTTSocketNumber));
    //strcat(msg, buff);
    //strcat(msg, "-");

    if (MQTTNewData)
    {
        digitalWrite(ESP32_ONBOARD_BLUE_LED_PIN, MQTTNewState);
        //Serial
            // myWebSerial.println("process MQTT - MQTTSocketNumber...");

            // myWebSerial.println(buff);

    //                     myWebSerial.println("process MQTT - MQTTNewState...");
    // sprintf(buff, "%d", (MQTTNewState));

           // myWebSerial.println(buff);

        transmitter.operateSocket(MQTTSocketNumber - 1, MQTTNewState);
        MQTTNewData = false; // indicate not new data now, processed
    }
}

char* getMQTTDisplayString(char *MQTTStatus)
{
    char msg[] = "This is a message placeholder";
    char socketNumber[] = "This is a also message placeh";


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


// MQTTclient call back if mqtt messsage rxed
void MQTTRxcallback(char *topic, byte *payload, unsigned int length)
{
    uint8_t socketNumber = 0;

    // Power<x> 		Show current power state of relay<x> as On or
    // Off Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
    // 1 / on 	Turn relay<x> power On handle message arrived mqtt
    //! do some extra checking on rxed topic and payload?
    payload[length] = '\0';

    char message[] = "MQTT rxed [this/is/the/topic/for/this/mesage] : and finally the payload, and a bit extra to make sure there is room in the string";
    strcpy(message, "MQTT Rx [");
    strcat(message, topic);
    strcat(message, "]: ");
    //append payload and add \o terminator
    strncat(message, (char *)payload, length);
    //Serial.println(message);
    myWebSerial.println(message);

    // e.g incoming topic = "433Bridge/cmnd/Power1" to "...Power16", and payload = 1 or 0
    // either match whole topic string or trim off last 1or 2 chars and
    // convert to a number, convert last 1-2 chars to socket number
    char lastChar =
        topic[strlen(topic) - 1]; // lst char will always be a digit char
    // see if last but 1 is also a digit char - ie number has two digits - 10 to 16
    char lastButOneChar = topic[strlen(topic) - 2];

    // if ((lastButOneChar >= '0') &&
    //     (lastButOneChar <= '9'))
    socketNumber = lastChar - '0';
    if ( (lastButOneChar == '1') )
    { // it is a 2 digit number
        socketNumber = socketNumber + 10; // calc actual int
    }
    // else
    // {
    //     socketNumber = (lastChar - '0');
    // }
    // // convert from 1-16 range to 0-15 range sendUnit uses
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


#include <PubSubClient.h>
extern PubSubClient MQTTclient;


// set so ensures initial connect attempt, assume now gives 0
static long lastReconnectAttemptMillis = -6000;

void connectMQTT()
{
    bool MQTTConnectTimeout = false;
    u16_t checkPeriodMillis = 10000;
    u16_t timeOutMillis = 3000;
    MQTTConnectTimeout = false;
    u16_t startMillis = millis();

    if (startMillis - lastReconnectAttemptMillis > checkPeriodMillis)
    {
        myWebSerial.println("ready to try MQTT reconnectMQTT...");
        while (!MQTTclient.connected() && !MQTTConnectTimeout)//loop till connected or timed out
        {
            myWebSerial.println("Attempting MQTT connection...");
            if (MQTTclient.connect("433BridgeMQTTClient"))
            {
                myWebSerial.println("connected to MQTT server");
                MQTTclient.subscribe(subscribeTopic);
            }
            else
            {
                myWebSerial.println("MQTT connection failed, rc=");
                Serial.println(MQTTclient.state());
                myWebSerial.println(" try again ..");
            }
            lastReconnectAttemptMillis = startMillis;
            MQTTConnectTimeout = ((millis() - startMillis) > timeOutMillis) ? true : false;
        }
    }
}
    // (!MQTTConnectTimeout) ? myWebSerial.println("MQTT Connection made!")
    //                       : myWebSerial.println("MQTT Connection attempt Time Out!");