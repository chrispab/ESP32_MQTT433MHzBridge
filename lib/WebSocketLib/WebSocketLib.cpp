// #include <RF24Lib.h>
#include "WebSocketLib.h"

#include "WebSerial.h"
//#include <WebSocketsServer.h>
#include "../../src/version.h"

extern WebSocketsServer webSocket;

extern WebSerial myWebSerial;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] client ws Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendTXT(num, "You are connected to esp32 webSocket server<br>");
        webSocket.sendTXT(num, SW_VERSION);
        webSocket.sendTXT(num, "<br>");
        break;
    }
    case WStype_TEXT:
        //Serial.printf("[%u] get Text: %s\n", num, payload);

        //send message to client
        //webSocket.sendTXT(num, "from ESP32 - got your message");

        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
        break;
    case WStype_BIN:
        Serial.printf("[%u] get binary length: %u\n", num, length);
        hexdump(payload, length,16);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        Serial.printf("UUUUUUU Unhandled WS message in handler UUUUU");

        break;
    }
}

#include <NTPClient.h>

extern NTPClient timeClient;

void broadcastWS()
{
    static unsigned long lastResetMillis = millis();
    unsigned long resetInterval = 10000;

    //if websockets buffer has content then send to client and empty the buffer
    if (myWebSerial.hasData())
    {
        String statusString;
        statusString = timeClient.getFormattedTime() + ": " + myWebSerial.getBuffer();
        //broadcast to all clients
        if (webSocket.connectedClients() > 0)
            webSocket.broadcastTXT(statusString);
        myWebSerial.clearBuffer();
    }

    if ((millis() - lastResetMillis) >= resetInterval)
    {
        lastResetMillis = millis();
    }
}

void hexdump(const void *mem, uint32_t len, uint8_t cols)
{
    const uint8_t *src = (const uint8_t *)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++)
    {
        if (i % cols == 0)
        {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}

