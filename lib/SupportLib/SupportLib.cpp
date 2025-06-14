#include "SupportLib.h"

#include "debug.h"


#include "TempSensor.h"
#include "pins.h"
extern TempSensor DHT22Sensor;

#include "ZoneController.h"
extern ZoneController ZCs[];

#include "WebSerial.h"
extern WebSerial myWebSerial;

// #include "MQTTLib.h"
// extern char *getMQTTDisplayString(char *MQTTDisplayString);

#include <NTPClient.h>
extern NTPClient timeClient;



boolean touchedFlag = false;



extern bool touchedFlag;


#include "LedFader.h"
extern LedFader warnLED;
char *RF24getDisplayString(char *statusMessage);



extern unsigned long currentMillis;
extern unsigned long previousConnCheckMillis;
extern unsigned long intervalConnCheckMillis;
// extern char* publishLWTTopic;

#include <PubSubClient.h>

#include "WiFiLib.h"
extern PubSubClient MQTTclient;



char* stringToPrint(const char* literal, const char* value) {
    static char buffer[64]; // Adjust size as needed
    buffer[0] = '\0'; // Ensure buffer is empty
    strncat(buffer, literal, sizeof(buffer) - 1);
    strncat(buffer, value, sizeof(buffer) - strlen(buffer) - 1);
    return buffer;
}

