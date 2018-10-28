// #include <RF24.h>

#include <WebSocketsServer.h>

// void connectRF24(void);
// void processZoneRF24Message(void);


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length); 

void hexdump(const void *mem, uint32_t len, uint8_t cols);
void broadcastWS(void);

