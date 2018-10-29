// #include <RF24.h>

#include <WebSocketsServer.h>

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length); 

void hexdump(const void *mem, uint32_t len, uint8_t cols);
void broadcastWS(void);
void MQTTRxcallback(char *topic, byte *payload, unsigned int length);
void connectMQTT();
char *getMQTTDisplayString(char *MQTTStatus);
void processMQTTMessage(void);




