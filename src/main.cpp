#include "debug.h"
#include "config.h"
#include "version.h"

// #define RELEASE
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <NewRemoteTransmitter.h>
#include <RF24.h>
#include <WiFi.h>
// #include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <stdlib.h>  // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

#include "Display.h"
#include "LedFader.h"
#include "MyOTA.h"
#include "SupportLib.h"
#include "TempSensor.h"
#include "ZoneController.h"
#include "pins.h"
#include "secret.h"
#include "sendemail.h"

// time stuff
#include <NTPClient.h>
// #include <WiFiUdp.h>
#define NTP_OFFSET 0            // 60 * 60      // In seconds, 0 for GMT, 60*60 for BST
#define NTP_INTERVAL 60 * 1000  // In miliseconds
#define NTP_ADDRESS "europe.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
// NTPClient timeClient(ntpUDP);

#include "MQTTLib.h"
extern char subscribeTopic[];    // = "433Bridge/cmnd/#";
extern char publishTempTopic[];  // = "433Bridge/Temperature";
extern char publishHumiTopic[];  // = "433Bridge/Humidity";
extern bool MQTTNewData;
// forward decs
// void checkConnections(void);
// void updateDisplayData(void);
void resetWatchdog(void);
// boolean processTouchPads(void);

void IRAM_ATTR resetModule();

// DHT22 stuff
TempSensor DHT22Sensor;

// MQTT stuff
#include <PubSubClient.h>
IPAddress mqttBroker(192, 168, 0, MQTT_LAST_OCTET);
WiFiClient WiFiEClient;
PubSubClient MQTTclient(mqttBroker, 1883, MQTTRxcallback, WiFiEClient);

// 433Mhz settings
// 282830 addr of 16ch remote
// param 3 is pulse width, last param is num times control message  is txed
#include "My433Transmitter.h"
// NewRemoteTransmitter transmitter(282830, TX433PIN, 260, 4);
My433Transmitter transmitter(282830, TX433PIN, 260, 4);

#include "RF24Lib.h"  //// Set up nRF24L01 rf24Radio on SPI bus plus pins 7 & 8
RF24 rf24Radio(RF24_CE_PIN, RF24_CS_PIN);

#include "LightSensor.h"
LightSensor myLightSensor(LDR_PIN);

// Global vars
unsigned long currentMillis = 0;
// unsigned long previousConnCheckMillis = 0;
unsigned long intervalConnCheckMillis = 30000;

unsigned long intervalTempDisplayMillis = 60000;
unsigned long previousTempDisplayMillis =
    millis() - intervalTempDisplayMillis;  // trigger on start

// create the display object
Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, OLED_CLOCK_PIN,
                  OLED_DATA_PIN);
// def zone controllers, 2nd parm is socketID (0-15)
ZoneController ZCs[3] = {ZoneController(0, 13, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 14, "SHD", "SSS")};

WiFiServer server(80);

// create object
// SendEmail e("smtp.gmail.com", 465, EMAIL_ADDRESS, APP_PASSWORD,
// 2000, true);
// set parameters. pin 13, go from 0 to 255 every n milliseconds
#define HEART_BEAT_TIME 500
LedFader heartBeatLED(GREEN_LED_PIN, 1, 0, 50, HEART_BEAT_TIME, true);
LedFader warnLED(RED_LED_PIN, 2, 0, 255, 451, true);

#include <WebSerial.h>
WebSerial myWebSerial;

#include "WebSocketLib.h"

WebSocketsServer webSocket = WebSocketsServer(81);

// #define EMAIL_SUBJECT "ESP32 Bridge - REBOOTED"

// #include "WebSocketLib.h"

extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                           size_t length);

#include "WebPageLib.h"
#include "WiFiLib.h"

// extern boolean processTouchPads(void);
// extern char *getElapsedTimeStr();
// extern void updateDisplayData();
// extern void checkConnections();
// extern displayModes displayMode;
// extern boolean touchedFlag;  // = false;
// Remove any previous externs for processTouchPads and touchedFlag
extern bool processTouchPads(void);
extern char *getElapsedTimeStr();
extern void updateDisplayData();
extern void checkConnections();
#include "Display.h" // Ensure displayModes is defined before use
extern displayModes displayMode;
extern bool touchedFlag;  // = false;

#include "TouchPad.h"
TouchPad touchPad1 = TouchPad(TOUCH_SENSOR_1);
TouchPad touchPad2 = TouchPad(TOUCH_SENSOR_2);

// ! big issue - does not work when no internet connection - resolve
// hang on wifi connect etc
//!! poss fixed - !!RETEST


#include "PIRSensor.h"
PIRSensor myPIRSensor(PIR_PIN);

#include <RestClient.h>
// char restHost[]="homested.local";
// char restHost[]="192.168.0.40";
char restHost[] = "chrisiot.com";

// RestClient client = RestClient(restHost, 443);
RestClient client = RestClient(restHost, 80);

bool initit = true;
/**
 * @brief
 *
 */
//! WATCHDOG STUFF
#include "esp_system.h"
hw_timer_t *timer = NULL;
// const int wdtTimeoutS = ESP32_WATCHDOG_TIMEOUT_SECS;
const int wdtTimeoutMs =
    ESP32_WATCHDOG_TIMEOUT_SECS * 1000;  // time in ms to trigger the watchdog
unsigned long resetWatchdogIntervalMs =
    ESP32_WATCHDOG_RESET_INTERVAL_SECS * 1000;

void IRAM_ATTR resetModule() {
    ets_printf("ESP32 Rebooted by Internal Watchdog\n");
    esp_restart();
}

//------------------------------------------------------------
unsigned long previousAPIWriteMillis = 0;
/**
 * @brief Periodically sends a REST API POST request with the current time.
 *
 * Uses the RestClient to send a POST request to a configured endpoint every 20 seconds.
 * Handles buffer safety and prints debug output for status codes.
 */
void doRest() {
    char postParameter[79];
    char postMessage[255];

    String postValue = "";
    postValue.toCharArray(postParameter, sizeof(postParameter));
    unsigned long intervalAPIWriteMillis = 20000;
    unsigned long currentMillis = millis();
    String dateTimeStr = "";
    String postStrFull = "";

    String postStr1 = "";
    String postStr2 = "";
    // do every 20 secs
    if (currentMillis - previousAPIWriteMillis > intervalAPIWriteMillis) {
        client.setHeader("Accept: application/json");

        // local auth token
        // client.setHeader("Authorization: Bearer
        // eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjI1MjE2OWI3MTMxNjVlNTczNWU1MGUwMGY2NzZhYjdiOGYwYTUzMTY0YWEyZTdiYzdiMzAzMDMzNzE4ZmRlMmE5M2QwZDdlODEwMDI1NDMwIn0.eyJhdWQiOiIzIiwianRpIjoiMjUyMTY5YjcxMzE2NWU1NzM1ZTUwZTAwZjY3NmFiN2I4ZjBhNTMxNjRhYTJlN2JjN2IzMDMwMzM3MThmZGUyYTkzZDBkN2U4MTAwMjU0MzAiLCJpYXQiOjE1Njc0NDkyOTEsIm5iZiI6MTU2NzQ0OTI5MSwiZXhwIjoxNTk5MDcxNjkwLCJzdWIiOiIyIiwic2NvcGVzIjpbXX0.Q8i63MAVgbGjRTYilydHHb0ljHvKhkeSANJbJ-qD_8_uWhPC_vQUrAC67mL3DmHm3pZkOvNm5WTAx5zQpTfOq-nJkB4c6vUytjQmyQNG-eV8WF90q_ccO5jbljlHORvfUzDF7TJRgKwO4Dcl8lWSQYgta3g_MkgH42qJHg9HEbGOKvgAvGsMsmeouLKwYojN8Oh02gKCQ_T7hcUkcB3zWVH9_ltV3qiSqA66VMyT45NzMuz3yxOYbSwXWaJl4AgiMs96LBDnpqMZzJIIYJ2YMMkXdaYljJhHga6vsGgxwc9HrZM2ZdY4EJcRcokVc6S6TGIJLEeGuIgGet-qDXhTEN832ufwh8saETrH_D_isnDohMEOkHjwWHkfcF4kfoYvQyD5jTg7DP4zqMDIE7uQmdiWDES512nByqmpzWNenIIMKZ1e5nT2EqvLDT21mdHhF35JzL0FUWd341xXTqjJLV27lfX3HAcs0pn69kY5X7Wqb4GNnEKlU-BbV-d6tBMNQI6yDcnKFYE2eJADtauMzmcAr_nNRqf212jqjLjblrqH1Qaoh1ZGHHnITUPd6Ai5uZa_x-phv1sTK4IaWwdtLn4RTQEWfiR1wVYePkfVM9xl1eTuiRrTfwAmRu-flCTCC66_ZobhYqLLmOssImK-GrxOmqQFC15zgC6PxklihpE");

        // chrisiot auth token
        client.setHeader(
            "Authorization: Bearer "
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjZhOWExZTg1MTEwM2JlNDgxYW"
            "Q1Nzk0ZGMzNGM5NTkzYjk0NDk4YjQ3ZjA1YjBiOTU5ODg2YzM4YzRmMmRmODMyZmE1ODZj"
            "ZjE1NDFmZjBmIn0."
            "eyJhdWQiOiIxIiwianRpIjoiNmE5YTFlODUxMTAzYmU0ODFhZDU3OTRkYzM0Yzk1OTNiOT"
            "Q0OThiNDdmMDViMGI5NTk4ODZjMzhjNGYyZGY4MzJmYTU4NmNmMTU0MWZmMGYiLCJpYXQi"
            "OjE1Njc3ODQ3MDIsIm5iZiI6MTU2Nzc4NDcwMiwiZXhwIjoxNTk5NDA3MTAyLCJzdWIiOi"
            "IzIiwic2NvcGVzIjpbXX0."
            "lmMl9R3CHo9ODPwwYcLH3tDrqolIyJgVeQWBVJF41RYzHmZDkZzFG9oL_"
            "trt1ewwDqYWsx_G7_Ka6_rwrQoKvefR4KY7HzMXACc-"
            "iiKpjoYX4Ersd3tXqFuj0AdkM7xzLjzPWHJhFleHjrrMwNuITD2-"
            "YXHGqjznCr5mCsfgTxfW0h3sEpKTv3DBukGScPmPFzPn-hL0-"
            "tmDZHImuQAwT6aDVjdEMJfSgtrkGDmF1CaXPi27JL8TjbCvGA2cyuNp6wpuutsqi9UuKTt"
            "_gQbrH9hsVxOwgS3GST2GMhWlbGx9vWkrilUWnkOVpSR0RzLzRLb-8se4BPOsi3Jer_"
            "h1pXNSKlOYylpeRZm_9Qd_"
            "ooI6YI7PIuU0ZN9hj5QDeRbq2JVfXMnBgI9X9x9cJEpWu7vuWtJCntVTvSVJqaBVoh0SCQ"
            "n9yzpPfj5wJjuw1EZN-w8nphb5vgw-LTfjv4QeBdZ9Vo9VoUrUnNng6Ki3_"
            "uNzbvZOiCDQ8sKWBBHPQ421Rv-Z2UFghNAsG7_GL9_"
            "n6etFrKch34CGIAqPjcF1fJhTv3ERQh3ep5Ym_"
            "LsWxCYFv7WkKOAP1mxoPWNPtBvQ9ms5SHYZNnUtOzPTL6HDWuAllwZFOyVj7YmCZm1imN4"
            "d1dvUlhCAfkgd33ZNPREaGxjJyAyRMYE9D_Y7K0pnMNg");

        // build the POST string
        postStr1 =
            "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        dateTimeStr = timeClient.getFormattedDateTime(0);
        postStrFull = postStr1 + dateTimeStr;
        postStrFull.replace(" ", "%20");
        postStrFull.toCharArray(postMessage, sizeof(postMessage));
        int statusCode = client.post(postMessage, postParameter);
        DEBUG_PRINT("Status code from server: ");
        DEBUG_PRINTLN(statusCode);
        if (statusCode < 200 || statusCode >= 300) {
            DEBUG_PRINTLN("[REST] Warning: Non-success status code returned.");
        }
        previousAPIWriteMillis = currentMillis;
    }
}

/**
 * @brief Sends a REST API POST request with topic, payload, and timestamp.
 *
 * @param topic        The topic string for the REST API.
 * @param payload      The payload string for the REST API.
 * @param published_at The timestamp string for the REST API.
 *
 * Handles buffer safety and prints debug output for status codes.
 */
void storeREST(char *topic, char *payload, char *published_at) {
    char postParameter[79];
    char postMessage[255];

    String postValue = "";
    postValue.toCharArray(postParameter, sizeof(postParameter));
    unsigned long currentMillis = millis();
    String dateTimeStr = "";
    String postStrFull = "";

    String postStr1 = "";
    String postStr2 = "";
    String postTopic = "";
    String postPayload = "";
    String published_atStr = "";

    client.setHeader("Accept: application/json");

    // local auth token
    // client.setHeader("Authorization: Bearer
    // eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjI1MjE2OWI3MTMxNjVlNTczNWU1MGUwMGY2NzZhYjdiOGYwYTUzMTY0YWEyZTdiYzdiMzAzMDMzNzE4ZmRlMmE5M2QwZDdlODEwMDI1NDMwIn0.eyJhdWQiOiIzIiwianRpIjoiMjUyMTY5YjcxMzE2NWU1NzM1ZTUwZTAwZjY3NmFiN2I4ZjBhNTMxNjRhYTJlN2JjN2IzMDMwMzM3MThmZGUyYTkzZDBkN2U4MTAwMjU0MzAiLCJpYXQiOjE1Njc0NDkyOTEsIm5iZiI6MTU2NzQ0OTI5MSwiZXhwIjoxNTk5MDcxNjkwLCJzdWIiOiIyIiwic2NvcGVzIjpbXX0.Q8i63MAVgbGjRTYilydHHb0ljHvKhkeSANJbJ-qD_8_uWhPC_vQUrAC67mL3DmHm3pZkOvNm5WTAx5zQpTfOq-nJkB4c6vUytjQmyQNG-eV8WF90q_ccO5jbljlHORvfUzDF7TJRgKwO4Dcl8lWSQYgta3g_MkgH42qJHg9HEbGOKvgAvGsMsmeouLKwYojN8Oh02gKCQ_T7hcUkcB3zWVH9_ltV3qiSqA66VMyT45NzMuz3yxOYbSwXWaJl4AgiMs96LBDnpqMZzJIIYJ2YMMkXdaYljJhHga6vsGgxwc9HrZM2ZdY4EJcRcokVc6S6TGIJLEeGuIgGet-qDXhTEN832ufwh8saETrH_D_isnDohMEOkHjwWHkfcF4kfoYvQyD5jTg7DP4zqMDIE7uQmdiWDES512nByqmpzWNenIIMKZ1e5nT2EqvLDT21mdHhF35JzL0FUWd341xXTqjJLV27lfX3HAcs0pn69kY5X7Wqb4GNnEKlU-BbV-d6tBMNQI6yDcnKFYE2eJADtauMzmcAr_nNRqf212jqjLjblrqH1Qaoh1ZGHHnITUPd6Ai5uZa_x-phv1sTK4IaWwdtLn4RTQEWfiR1wVYePkfVM9xl1eTuiRrTfwAmRu-flCTCC66_ZobhYqLLmOssImK-GrxOmqQFC15zgC6PxklihpE");

    // chrisiot auth token
    client.setHeader(
        "Authorization: Bearer "
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjZhOWExZTg1MTEwM2JlNDgxYWQ1"
        "Nzk0ZGMzNGM5NTkzYjk0NDk4YjQ3ZjA1YjBiOTU5ODg2YzM4YzRmMmRmODMyZmE1ODZjZjE1"
        "NDFmZjBmIn0."
        "eyJhdWQiOiIxIiwianRpIjoiNmE5YTFlODUxMTAzYmU0ODFhZDU3OTRkYzM0Yzk1OTNiOTQ0"
        "OThiNDdmMDViMGI5NTk4ODZjMzhjNGYyZGY4MzJmYTU4NmNmMTU0MWZmMGYiLCJpYXQiOjE1"
        "Njc3ODQ3MDIsIm5iZiI6MTU2Nzc4NDcwMiwiZXhwIjoxNTk5NDA3MTAyLCJzdWIiOiIzIiwi"
        "c2NvcGVzIjpbXX0."
        "lmMl9R3CHo9ODPwwYcLH3tDrqolIyJgVeQWBVJF41RYzHmZDkZzFG9oL_trt1ewwDqYWsx_"
        "G7_Ka6_rwrQoKvefR4KY7HzMXACc-"
        "iiKpjoYX4Ersd3tXqFuj0AdkM7xzLjzPWHJhFleHjrrMwNuITD2-"
        "YXHGqjznCr5mCsfgTxfW0h3sEpKTv3DBukGScPmPFzPn-hL0-"
        "tmDZHImuQAwT6aDVjdEMJfSgtrkGDmF1CaXPi27JL8TjbCvGA2cyuNp6wpuutsqi9UuKTt_"
        "gQbrH9hsVxOwgS3GST2GMhWlbGx9vWkrilUWnkOVpSR0RzLzRLb-8se4BPOsi3Jer_h1pXN"
        "SKlOYylpeRZm_9Qd_ooI6YI7PIuU0ZN9hj5QDeRbq2JVfXMnBgI9X9x9cJEpWu7vuWtJCnt"
        "VTvSVJqaBVoh0SCQn9yzpPfj5wJjuw1EZN-w8nphb5vgw-LTfjv4QeBdZ9Vo9VoUrUnNng6"
        "Ki3_uNzbvZOiCDQ8sKWBBHPQ421Rv-Z2UFghNAsG7_GL9_n6etFrKch34CGIAqPjcF1fJhT"
        "v3ERQh3ep5Ym_LsWxCYFv7WkKOAP1mxoPWNPtBvQ9ms5SHYZNnUtOzPTL6HDWuAllwZFOyV"
        "j7YmCZm1imN4d1dvUlhCAfkgd33ZNPREaGxjJyAyRMYE9D_Y7K0pnMNg");

    // build the POST string
    //"/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
    postStr1 =
        "/api/todo?";  //"/test/topic""&content=%7Bcontent:body%7D&published_at=";
    postTopic = "topic=";
    postTopic += topic;
    postPayload = "&content=";
    postPayload += payload;
    dateTimeStr = timeClient.getFormattedDateTime(0);
    published_atStr = "&published_at=";
    published_atStr += String(published_at);
    // postStr2 =
    // "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
    postStrFull =
        postStr1 + postTopic + postPayload + published_atStr;  // dateTimeStr;
    // replace any spaces (esp the one bet date and time) with %20
    postStrFull.replace(" ", "%20");
    postStrFull.replace("{", "%7B");
    postStrFull.replace("}", "%7D");

    postStrFull.toCharArray(postMessage, sizeof(postMessage));
    int statusCode = client.post(postMessage, postParameter);
    DEBUG_PRINT("Status code from server: ");
    DEBUG_PRINTLN(statusCode);
    if (statusCode < 200 || statusCode >= 300) {
        DEBUG_PRINTLN("[REST] Warning: Non-success status code returned.");
    }
    previousAPIWriteMillis = currentMillis;
}

#define SENSOR_INTERVAL_MS 1000
#define WIFI_CHECK_INTERVAL_MS 5000
#define TELEMETRY_INTERVAL_MS 60000
#define DISPLAY_ON_TIME_MS 2500

displayModes displayMode = NORMAL;

void setup() {
    Serial.begin(115200);
    myWebSerial.println("==========running setup==========");
    heartBeatLED.begin();                         // initialize
    warnLED.begin();                              // initialize
    pinMode(ESP32_ONBOARD_BLUE_LED_PIN, OUTPUT);  // set the LED pin mode

    timer = timerBegin(0, 8000, true);  // timer 0, 80mhz div 8000
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, wdtTimeoutMs * 10, false);  // set time in us
    timerAlarmEnable(timer);                           // enable interrupt

    // setup OLED display
    displayMode = NORMAL;
    displayMode = BIG_TEMP;
    // displayMode = MULTI;
    myDisplay.begin();
    myDisplay.setFont(SYS_FONT);
    myDisplay.wipe();
    myDisplay.writeLine(1, TITLE_LINE1);
    myDisplay.writeLine(2, TITLE_LINE2);
    myDisplay.writeLine(3, TITLE_LINE3);
    myDisplay.writeLine(4, TITLE_LINE4);
    myDisplay.writeLine(5, TITLE_LINE5);
    myDisplay.writeLine(6, TITLE_LINE6);
    myDisplay.refresh();
    delay(1000);

    myDisplay.wipe();
    myDisplay.writeLine(1, SW_VERSION);
    myDisplay.writeLine(2, "Connecting to Sensor..");
    myDisplay.refresh();
    DHT22Sensor.setup(DHTPIN, DHT22Sensor.AM2302);
    // rf24 stuff
    myDisplay.writeLine(3, "Connecting to RF24..");
    myDisplay.refresh();
    connectRF24();
    // attempt to connect to Wifi network:
    myDisplay.writeLine(4, "Connecting to WiFi..");
    myDisplay.refresh();
    connectWiFi();
    // you're connected now, so print out the status:
    printWifiStatus();
    // server.begin();
    CR;
    myDisplay.writeLine(5, "Connecting to MQTT..");
    myDisplay.refresh();
    connectMQTT();
    myDisplay.writeLine(6, "DONE");
    myDisplay.refresh();
    timeClient.begin();
    timeClient.update();
    // Serial.println(timeClient.getFormattedTime());
    DEBUG_PRINTLN(timeClient.getFormattedTime());
    delay(200);

    // Send Email
    // e.send(EMAIL_ADDRESS, EMAIL_ADDRESS, EMAIL_SUBJECT, "programm
    // started/restarted"); myWebhook.trigger("433Bridge Boot/Reboot");
    // myWebhook.trigger();

    myDisplay.wipe();
    // connectWiFi();
    resetWatchdog();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    setupOTA();
    resetWatchdog();

    // MQTTclient.
    // myWebhook.trigger("433Bridge Boot/Reboot");
    myLightSensor.readLevelIfDue();
    // client.begin(MY_SSID, MY_SSID_PASSWORD);
    // initit = true;
}

/**
 * @brief
 *
 */
// text buffer for main loop
char tempString[] = "12345678901234567890";

// Variable to track when to turn off the display after motion
// unsigned long displayOnUntil = 0;

static unsigned long displayOnUntil = millis() + 10000;
static bool displayIsOn = true;

void processPir() {
    bool motion = myPIRSensor.processPIRSensor(MQTTclient);
    if (motion) {
        displayOnUntil = millis() + DISPLAY_ON_TIME_MS;
        if (!displayIsOn) {
            myDisplay.display();  // or myDisplay.displayOn()
            displayIsOn = true;
            myWebSerial.println("Motion detected!");
        }
    }
    if (displayIsOn && millis() > displayOnUntil) {
        myDisplay.noDisplay();  // or myDisplay.displayOff()
        displayIsOn = false;
    }
}

void processTime() {
    if (WiFi.status() == WL_CONNECTED) {
        static unsigned long lastNTPCheck = 0;
        static bool timeUpdatedFromInternet = false;
        if (millis() - lastNTPCheck >= 60000) {  // 60 seconds
            lastNTPCheck = millis();
            IPAddress ntpServerIP;
            if (WiFi.hostByName(NTP_ADDRESS, ntpServerIP)) {
                timeUpdatedFromInternet = timeClient.update();
                if (timeUpdatedFromInternet) {
                    DEBUG_PRINT("NTP Time updated: ");
                    DEBUG_PRINTLN(timeClient.getFormattedTime());
                } else {
                    DEBUG_PRINTLN("NTP Time update failed.");
                    DEBUG_PRINTLN(timeClient.getFormattedTime());
                }
            } else {
                DEBUG_PRINTLN("Failed to resolve NTP server address.");
            }
        }
    }
}

void processTemperatureSensor() {
    if (DHT22Sensor.takeReadings()) {
        DEBUG_PRINTLN("=======> New- Temp reading - MQTT pub: ");
        MQTTclient.publish(publishTempTopic, DHT22Sensor.getTemperatureString());
        DEBUG_PRINT("new temp reading: ");
        DEBUG_PRINTLN(DHT22Sensor.getTemperatureString());
        DEBUG_PRINT("new humidity reading: ");
        DEBUG_PRINTLN(DHT22Sensor.getHumidityString());
        MQTTclient.publish(publishHumiTopic, DHT22Sensor.getHumidityString());
    }
}

void processRF24ZoneWatchdog() {
    processZoneRF24Message();
    if (ZCs[0].manageRestarts(transmitter)) {
        // myWebhook.trigger("ESP32 Watchdog: Zone 1 power cycled");
    }
    ZCs[1].resetZoneDevice();  // If you want to keep this always-on reset
    if (ZCs[2].manageRestarts(transmitter)) {
        // myWebhook.trigger("ESP32 Watchdog: Zone 3 power cycled");
    }
}

void checkMQTT() {
    if (!MQTTclient.connected()) {
        reconnectMQTT();
    } else {
        MQTTclient.loop();
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Sensor and connectivity checks
    // Use static to preserve lastTemperatureSensorCheck value between loop() calls
    static unsigned long lastTemperatureSensorCheck = 0;
    if (currentMillis - lastTemperatureSensorCheck >= SENSOR_INTERVAL_MS) {
        lastTemperatureSensorCheck = currentMillis;
        processTemperatureSensor();
        // processLightSensor();
        myLightSensor.process(MQTTclient);
        processPir();
    }

    // Use static to preserve lastWiFiCheck value between loop() calls
    static unsigned long lastWiFiCheck = 0;
    if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL_MS) {
        lastWiFiCheck = currentMillis;
        checkWifi();
    }

    // Core maintenance
    ArduinoOTA.handle();
    resetWatchdog();
    heartBeatLED.update();

    // Use static to preserve lastTelemetryCheck value between loop() calls
    static unsigned long lastTelemetryCheck = 0;
    if (currentMillis - lastTelemetryCheck >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryCheck = currentMillis;
        processTime();
        publishTelemetryIfDue();
    }

    // MQTT handling
    checkMQTT();
    processMQTTRecievedMessageAction();

    // WebSocket and broadcast (once per loop)
    webSocket.loop();
    broadcastWS();

    // Display update
    if (displayIsOn) {
        // updateDisplayData();
        myDisplay.updateDisplayData(
            ZCs, DHT22Sensor, myWebSerial, warnLED, timeClient, displayMode);
    }

    // RF24 zone management
    processRF24ZoneWatchdog();

    // Web page requests
    checkForPageRequest();
}

/**
 * @brief Reset the watchdog timer to prevent the ESP32 from rebooting.
 *
 * This function resets the watchdog timer at specified intervals to ensure
 * that the ESP32 does not reboot due to inactivity. It prints a message to
 * the WebSerial indicating that the watchdog has been reset.
 */
void resetWatchdog(void) {
    static unsigned long lastResetWatchdogMillis = millis();

    if ((millis() - lastResetWatchdogMillis) >= resetWatchdogIntervalMs) {
        timerWrite(timer, 0);  // reset timer (feed watchdog)
        myWebSerial.print(timeClient.getTimeStr());
        myWebSerial.println("+> Reset Bridge Watchdog");
        // DEBUG_PRINT(getTimeStr());

        lastResetWatchdogMillis = millis();
    }
}
