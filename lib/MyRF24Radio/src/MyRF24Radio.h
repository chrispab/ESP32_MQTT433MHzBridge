#ifndef MyRF24Radio_h
#define MyRF24Radio_h
//#include <PubSubClient.h>
//#include <WiFi.h>
#include <RF24.h>

#define CR Serial.println()

// #define ce_pin 5
// #define cs_pin 4

class MyRF24Radio : public RF24 {
  public:
    //MyRF24Radio();
    // MyRF24Radio(IPAddress address, uint16_t num1,
    //             void(char *topic, byte *payload, unsigned int length),
    //             WiFiClient wfc);

    MyRF24Radio(int CEPin, int CSPin);

    char *getDisplayString(char *MQTTStatus);
    void setPipes(uint8_t *writingPipe, uint8_t *readingPipe);
    void checkForMessages(void);
    char *getPayload(void);
    boolean hasNewData(void);

    void clearNewDataFlag();

    // void connectMQTT(void);
    // boolean hasNewData(void);
    // void clearNewDataFlag(void);
    // static void MQTTRxcallback(char *topic, byte *payload, unsigned int
    // length); int getSocketNumber(void); int getState(void);

  private:
    unsigned long currentMillis;
    char displayString[20];

    uint8_t writePipeLocS[6] = "NodeS";
    uint8_t readPipeLocS[6] = "Node0";
    uint8_t writePipeLocC[6] = "NodeC";
    uint8_t readPipeLocC[6] = "Node0";
    uint8_t writePipeLocG[6] = "NodeG";
    uint8_t readPipeLocG[6] = "Node0";
    // Payload
    static int max_payload_size;
    // +1 to allow room for a terminating NULL char
    //    char receive_payload[max_payload_size + 1];
    char receive_payload[32];
    uint8_t newDataFlag;
};

#endif // MyRF24Radio_h
