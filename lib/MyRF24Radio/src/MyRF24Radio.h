#ifndef MyMQTTClient_h
#define MyMQTTClient_h
#include <PubSubClient.h>
#include <WiFi.h>
#define CR Serial.println()

class MyMQTTClient : public PubSubClient {
  public:
    MyMQTTClient(IPAddress address, uint16_t num1,
                 void(char *topic, byte *payload, unsigned int length),
                 WiFiClient wfc);

    MyMQTTClient(IPAddress IPaddress, uint16_t the_port, WiFiClient the_client);

    char *getDisplayString(char *MQTTStatus);
    void connectMQTT(void);
     boolean hasNewData(void);
     void clearNewDataFlag(void);
     static void MQTTRxcallback(char *topic, byte *payload, unsigned int length);
     int getSocketNumber(void);
     int getState(void);

  private:
    unsigned long currentMillis;
    char displayString[20];
    // singles
    static uint8_t MQTTState;        // 0 or 1
    static uint8_t MQTTSocketNumber; // 1-16
    static char subscribeTopic[20];
    static boolean MQTTNewDataFlag;
};
// uint8_t MyMQTTClient::MQTTState = 0;
// uint8_t MyMQTTClient::MQTTSocketNumber = 0;
// boolean MyMQTTClient::MQTTNewDataFlag = false;
// char MyMQTTClient::subscribeTopic[] = "433Bridge/cmnd/#";

#endif // MyMQTTClient_h
