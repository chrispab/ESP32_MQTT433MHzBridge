#include "MyRF24Radio.h"
//#include <NewRemoteTransmitter.h>

// define static class vars - singles
// uint8_t MyRF24Radio::MQTTState = 0;
// uint8_t MyRF24Radio::MQTTSocketNumber = 0;
// boolean MyRF24Radio::MQTTNewDataFlag = false;
// char MyRF24Radio::subscribeTopic[] = "433Bridge/cmnd/#";

//int MyRF24Radio::max_payload_size = 32;

// MyRF24Radio::MyRF24Radio() {
//     //max_payload_size = 32;
//     setPipes(writePipeLocC,
//              readPipeLocC); // SHOULD NEVER NEED TO CHANGE PIPES
//     this->startListening();
// }

// MyRF24Radio::MyRF24Radio(IPAddress the_address, uint16_t the_port,
//                          void (*callback)(char *topic, byte *payload,
//                                           unsigned int length),
//                          WiFiClient wfc)
//     : PubSubClient(the_address, the_port, *callback, wfc) {
//     // extra initialisation here now base cla ss done
// }

MyRF24Radio::MyRF24Radio(int CEPin, int CSPinZZZ)
: RF24(CEPin, CSPinZZZ) {
    // extra initialisation here now base cla ss done
    // MyRF24Radio::MQTTState = 0;
    // MyRF24Radio::MQTTSocketNumber = 0;
    // MyRF24Radio::MQTTNewDataFlag = false;
    // strcpy(MyRF24Radio::subscribeTopic, "433Bridge/cmnd/#");
    // // now set internal method callback handler
    // this->setCallback(this->MQTTRxcallback);
    //     setPipes(writePipeLocC,
    //          readPipeLocC); // SHOULD NEVER NEED TO CHANGE PIPES
    this->startListening();
}

void MyRF24Radio::setPipes(uint8_t *writingPipe, uint8_t *readingPipe) {
    // config rf24Radio to comm with a node
    this->stopListening();
    this->openWritingPipe(writingPipe);
    this->openReadingPipe(1, readingPipe);
}
void MyRF24Radio::checkForMessages() {
    if (this->available()) { // Read all available payloads

        // Grab the message and process
        uint8_t len = this->getDynamicPayloadSize();

        // If a corrupt dynamic payload is received, it will be flushed
        if (!len) {
            return;
        }
        //! put back just debugging
        this->read(receive_payload, len);

        // Put a zero at the end for easy printing etc
        receive_payload[len] = 0;

        // who was it from?
        // reset that timer
    }
}

// boolean MyRF24Radio::hasNewData(void) { return
// MyRF24Radio::MQTTNewDataFlag;
// }

// void MyRF24Radio::clearNewDataFlag() { MyRF24Radio::MQTTNewDataFlag =
// false;
// }

// int MyRF24Radio::getSocketNumber(void) { return
// MyRF24Radio::MQTTSocketNumber; }

// int MyRF24Radio::getState(void) { return MyRF24Radio::MQTTState; }

//
// void MyRF24Radio::MQTTRxcallback(char *topic, byte *payload,
//                                  unsigned int length) {
//     // Power<x> Show current power state of relay<x> as On or Off
//     // Power<x> 	0 / off 	Turn relay<x> power Off Power<x>
//     // 1 / on 	Turn relay<x> power On handle message arrived mqtt
//     int socketNumber;

//     payload[length] = '\0';
//     Serial.print("MQTT rxed [");
//     Serial.print(topic);
//     Serial.print("] : ");
//     for (uint8_t i = 0; i < length; i++) {
//         Serial.print((char)payload[i]);
//     }
//     CR;
//     // e.g topic = "433Bridge/cmnd/Power1" to "...Power16", and payload =
//     1 or 0
//     // either match whole topic string or trim off last 1or 2 chars and
//     // convert to a number, convert last 1-2 chars to socket number
//     // lst char will always be a digit char
//     char lastChar = topic[strlen(topic) - 1];
//     // see if last but 1 is also a digit char - ie number has two digits
//     -
//     // 10 to 16
//     char lastButOneChar = topic[strlen(topic) - 2];

//     if ((lastButOneChar >= '0') &&
//         (lastButOneChar <= '9')) { // is it a 2 digit number
//         socketNumber =
//             ((lastButOneChar - '0') * 10) + (lastChar - '0'); // calc
//             actual int
//     } else {
//         socketNumber = (lastChar - '0');
//     }
//     // convert from 1-16 range to 0-15 range sendUnit uses
//     // int socketID = socketNumber - 1;
//     uint8_t newState = 0; // default to off
//     if ((payload[0] - '1') == 0) {
//         newState = 1;
//     }
//     // signal a new command has been rxed
//     // and
//     // topic and payload also available
//     MyRF24Radio::MQTTState = newState;            // 0 or 1
//     MyRF24Radio::MQTTSocketNumber = socketNumber; // 1-16
//     MyRF24Radio::MQTTNewDataFlag = true;
//     // then leave main control loop to turn of/onn sockets
//     // and reset sigmals

//     // digitalWrite(LEDPIN, newState);
//     // operateSocket(socketID, newState);
// }

boolean MyRF24Radio::hasNewData(void) { return MyRF24Radio::newDataFlag; }

void MyRF24Radio::clearNewDataFlag() { MyRF24Radio::newDataFlag = false; }

char *MyRF24Radio::getPayload(void) {
    // char msg[17] = "Socket:";
    // char buff[10];

    // sprintf(buff, "%d", (MyRF24Radio::MQTTSocketNumber));
    // strcat(msg, buff);

    // if (MyRF24Radio::MQTTState == 0) {
    //     strcat(msg, " OFF");
    // } else {
    //     strcat(msg, " ON");
    // }
    // // Serial.println(msg);
    // strcpy(MQTTStatus, msg);

    return receive_payload;
}
char *MyRF24Radio::getDisplayString(char *MQTTStatus) {
    // char msg[17] = "Socket:";
    // char buff[10];

    // sprintf(buff, "%d", (MyRF24Radio::MQTTSocketNumber));
    // strcat(msg, buff);

    // if (MyRF24Radio::MQTTState == 0) {
    //     strcat(msg, " OFF");
    // } else {
    //     strcat(msg, " ON");
    // }
    // // Serial.println(msg);
    // strcpy(MQTTStatus, msg);

    return MQTTStatus;
}
// void MyRF24Radio::connectMQTT() {
//     bool MQTTConnectTimeout = false;
//     u16_t startMillis;
//     u16_t timeOutMillis = 20000;
//     // Loop until we're reconnected
//     // check is MQTTclient is connected first
//     MQTTConnectTimeout = false;

//     startMillis = millis();
//     while (!connected() && !MQTTConnectTimeout) {
//         // printO(1, 20, "Connect MQTT..");
//         // myDisplay.writeLine(6, "Connect MQTT..");
//         // myDisplay.refresh();
//         Serial.println("Attempting MQTT connection...");
//         // Attempt to connect
//         //        if (MQTTclient.connect("ESP32Client","",""))
//         if (connect("ESP32Client")) {
//             Serial.println(F("connected to MQTT server"));
//             // Once connected, publish an announcement...
//             // MQTTclient.publish("outTopic", "hello world");
//             // ... and resubscribe
//             // MQTTclient.subscribe("inTopic");
//             this->subscribe(subscribeTopic);
//             // myDisplay.writeLine(6, "Connected MQTT!");
//             // myDisplay.refresh();
//         } else {
//             Serial.print("failed, rc=");
//             Serial.print(this->state());
//             Serial.println(" try again ..");
//             // Wait 5 seconds before retrying
//             // delay(5000);
//         }
//         MQTTConnectTimeout =
//             ((millis() - startMillis) > timeOutMillis) ? true : false;
//     }
//     (!MQTTConnectTimeout) ? Serial.println("MQTT Connection made!")
//                           : Serial.println("MQTT Connection attempt Time
//                           Out!");
// }