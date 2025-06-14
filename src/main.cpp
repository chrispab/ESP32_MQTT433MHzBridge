#include "debug.h"
#include "config.h"

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
// Consider moving these to config.h if they are configurable
constexpr uint32_t RF433_REMOTE_ADDRESS = 282830;
constexpr uint16_t RF433_PULSE_WIDTH = 260;
constexpr uint8_t  RF433_REPEAT_TRANSMISSIONS = 4;
My433Transmitter transmitter(RF433_REMOTE_ADDRESS, Pins::TX433PIN, RF433_PULSE_WIDTH, RF433_REPEAT_TRANSMISSIONS);


#include "RF24Lib.h"  //// Set up nRF24L01 rf24Radio on SPI bus plus pins 7 & 8
RF24 rf24Radio(Pins::RF24_CE_PIN, Pins::RF24_CS_PIN);

#include "LightSensor.h"
LightSensor myLightSensor(Pins::LDR_PIN);

// Global vars for display state, could be encapsulated if project grows
static unsigned long displayOnUntil = 0; // Will be initialized in setup
static bool displayIsOn = true;

static displayModes displayMode = NORMAL; // Definition of displayMode, static if only used in main.cpp
// create the display object
Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, Pins::OLED_CLOCK_PIN,
                  Pins::OLED_DATA_PIN);
// def zone controllers, 2nd parm is socketID (0-15)
ZoneController ZCs[3] = {ZoneController(0, 13, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 14, "SHD", "SSS")};

// WiFiServer server(80); // Uncomment if HTTP server on port 80 is needed

// create object
// SendEmail e("smtp.gmail.com", 465, EMAIL_ADDRESS, APP_PASSWORD,
// 2000, true);
// set parameters. pin 13, go from 0 to 255 every n milliseconds
// Consider defining these LED parameters as named constants if they have specific meanings
constexpr uint8_t HEARTBEAT_LED_CHANNEL = 1;
constexpr int HEARTBEAT_LED_MIN_BRIGHTNESS = 0;
constexpr int HEARTBEAT_LED_MAX_BRIGHTNESS = 50;
constexpr uint8_t WARN_LED_CHANNEL = 2;
constexpr int WARN_LED_MIN_BRIGHTNESS = 0;
constexpr int WARN_LED_MAX_BRIGHTNESS = 255;
constexpr unsigned long WARN_LED_FADE_TIME_MS = 451;
LedFader heartBeatLED(Pins::GREEN_LED_PIN, HEARTBEAT_LED_CHANNEL, HEARTBEAT_LED_MIN_BRIGHTNESS, HEARTBEAT_LED_MAX_BRIGHTNESS, HEART_BEAT_TIME, true);
LedFader warnLED(Pins::RED_LED_PIN, WARN_LED_CHANNEL, WARN_LED_MIN_BRIGHTNESS, WARN_LED_MAX_BRIGHTNESS, WARN_LED_FADE_TIME_MS, true);

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

// extern bool touchedFlag; // This is fine if SupportLib.cpp defines it and main needs it.
extern bool touchedFlag;  // = false;

#include "TouchPad.h"
TouchPad touchPad1 = TouchPad(Pins::TOUCH_SENSOR_1);
TouchPad touchPad2 = TouchPad(Pins::TOUCH_SENSOR_2);

// ! big issue - does not work when no internet connection - resolve
// hang on wifi connect etc
//!! poss fixed - !!RETEST


#include "PIRSensor.h"
PIRSensor myPIRSensor(Pins::PIR_PIN);

#include <RestClient.h>
// char restHost[]="homested.local";
// char restHost[]="192.168.0.40";
char restHost[] = "chrisiot.com";

// RestClient client = RestClient(restHost, 443); // For HTTPS if needed
RestClient client = RestClient(restHost, 80);
// If REST_BEARER_TOKEN is a const char*, this is more efficient:
// constexpr const char* bearerToken = REST_BEARER_TOKEN; // REST_BEARER_TOKEN is not constexpr
const char* bearerToken = REST_BEARER_TOKEN; 
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
static unsigned long previousAPIWriteMillis = 0;
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
    // Consider moving to config.h
    constexpr unsigned long DO_REST_INTERVAL_MS = 20000;
    unsigned long currentMillis = millis();
    String dateTimeStr = "";
    String postStrFull = "";

    String postStr1 = "";
    String postStr2 = "";
    // do every 20 secs
    if (currentMillis - previousAPIWriteMillis > DO_REST_INTERVAL_MS) {
        client.setHeader("Accept: application/json");

        // chrisiot auth token
        // String authHeader = String("Authorization: Bearer ") + REST_BEARER_TOKEN; // Original
        char authHeader[sizeof("Authorization: Bearer ") + strlen(bearerToken) + 1];
        strcpy(authHeader, "Authorization: Bearer ");
        strcat(authHeader, bearerToken);
        client.setHeader(authHeader);
        // build the POST string
        postStr1 =
            "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        dateTimeStr = timeClient.getFormattedDateTime(0);
        postStrFull = postStr1 + dateTimeStr;
        postStrFull.replace(" ", "%20");
        postStrFull.toCharArray(postMessage, sizeof(postMessage));
        int statusCode = client.post(postMessage, ""); // Assuming empty body if postParameter is not used
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
// Helper function for basic URL encoding (replace with a more robust one if needed)
String urlEncode(const char* str) {
    String encodedString = "";
    char c;
    char hex[4];
    while ((c = *str++)) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedString += c;
        } else {
            sprintf(hex, "%%%02X", c);
            encodedString += hex;
        }
    }
    return encodedString;
}
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
    // chrisiot auth token
    // String authHeader = String("Authorization: Bearer ") + REST_BEARER_TOKEN; // Original
    char authHeader[sizeof("Authorization: Bearer ") + strlen(bearerToken) + 1];
    strcpy(authHeader, "Authorization: Bearer ");
    strcat(authHeader, bearerToken);
    client.setHeader(authHeader);

    // build the POST string
    //"/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
    postStr1 =
        "/api/todo?";  //"/test/topic""&content=%7Bcontent:body%7D&published_at=";
    
    postTopic = "topic=";
    postTopic += urlEncode(topic); // URL Encode the topic
    postPayload = "&content=";
    postPayload += urlEncode(payload); // URL Encode the payload
    // dateTimeStr = timeClient.getFormattedDateTime(0); // Not used in final postStrFull
    published_atStr = "&published_at=";
    published_atStr += String(published_at);

    postStrFull =
        postStr1 + postTopic + postPayload + published_atStr;  // dateTimeStr;
    // replace any spaces (esp the one bet date and time) with %20
    postStrFull.replace(" ", "%20");
    // postStrFull.replace("{", "%7B"); // Encoding should happen per-parameter
    // postStrFull.replace("}", "%7D"); // Encoding should happen per-parameter

    int statusCode = client.post(postStrFull.c_str(), ""); // Assuming postParameter was always empty
    DEBUG_PRINT("Status code from server: ");
    DEBUG_PRINTLN(statusCode);
    if (statusCode < 200 || statusCode >= 300) {
        DEBUG_PRINTLN("[REST] Warning: Non-success status code returned.");
    }
    // previousAPIWriteMillis = currentMillis; // Decide if storeREST should affect doRest's schedule
}

void setup() {
    Serial.begin(115200);
    myWebSerial.println("==========running setup==========");
    heartBeatLED.begin();                         // initialize
    warnLED.begin();                              // initialize
    pinMode(Pins::ESP32_ONBOARD_BLUE_LED_PIN, OUTPUT);  // set the LED pin mode

    //watchdog timer setup
    DEBUG_PRINTLN("Setting up Watchdog Timer");
    timer = timerBegin(0, 8000, true);  // timer 0, 80mhz div 8000
    timerAttachInterrupt(timer, &resetModule, true); // ESP_INTR_FLAG_IRAM
    timerAlarmWrite(timer, (uint64_t)wdtTimeoutMs * 10, false);  // Alarm value is in timer ticks. (wdtTimeoutMs * 1000 / 100)
    timerAlarmEnable(timer);                           // enable interrupt

    // setup OLED display
    displayMode = BIG_TEMP; // Set intended default display mode
    // displayMode = MULTI; // Assuming BIG_TEMP uses Fonts::BIG_TEMP_FONT internally
    myDisplay.begin();
    myDisplay.setFont(Fonts::SYS_FONT); // Assuming SYS_FONT is now in Fonts namespace
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
    myDisplay.refresh(); // Refresh before sensor setup
    DHT22Sensor.setup(Pins::DHTPIN, DHT22Sensor.AM2302);
    // rf24 stuff
    myDisplay.writeLine(3, "Connecting to RF24..");
    myDisplay.refresh();
    connectRF24();
    // attempt to connect to Wifi network:
    myDisplay.writeLine(4, "Connecting to WiFi..");
    myDisplay.refresh();
    if (connectWiFi()) { // connectWiFi should return true on success
        // printWifiStatus(); // Already printed by connectWiFi on success
    } else {
        myWebSerial.println("Initial WiFi connection failed in setup.");
    }
    printWifiStatus();
    // server.begin();
    Serial.println();
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

    myDisplay.wipe();
    resetWatchdog();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    setupOTA();
    resetWatchdog();

    // Consider moving INITIAL_DISPLAY_ON_TIME_MS to config.h
    constexpr unsigned long INITIAL_DISPLAY_ON_TIME_MS = 10000;
    displayOnUntil = millis() + INITIAL_DISPLAY_ON_TIME_MS;
    myLightSensor.readLevelIfDue();
    // client.begin(MY_SSID, MY_SSID_PASSWORD);
}



void processPir() {
    bool motion = myPIRSensor.processPIRSensor(MQTTclient);
    if (motion) {
        displayOnUntil = millis() + PIR_TRIGGERED_DISPLAY_ON_TIME_MS;
        if (!displayIsOn) {
            myDisplay.display();
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
        if (millis() - lastNTPCheck >= NTP_INTERVAL) {  // Use constant from config.h
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
        // Optional: myWebhook.trigger("ESP32 Watchdog: Zone 0 power cycled");
    }
    // ZCs[1].resetZoneDevice();  // Review: Zone 1 is always reset. Is this intended vs. manageRestarts?
    if (ZCs[2].manageRestarts(transmitter)) {
        // Optional: myWebhook.trigger("ESP32 Watchdog: Zone 2 power cycled");
    }
}

/**
 * @brief The main loop of the program that handles sensor readings, MQTT communication,
 *        display updates, and other periodic tasks.
 *
 * This function runs continuously after setup() and performs the following tasks:
 * - Processes temperature and light sensors at defined intervals.
 * - Checks WiFi connectivity periodically.
 * - Handles MQTT messages and publishes telemetry data.
 * - Updates the display if it is turned on.
 * - Manages RF24 zone controllers.
 * - Handles WebSocket connections and broadcasts.
 */
void loop() {
    unsigned long currentMillis = millis();

    // Sensor and connectivity checks
    // Use static to preserve lastTemperatureSensorCheck value between loop() calls
    static unsigned long lastSensorProcessTime = 0;
    if (currentMillis - lastSensorProcessTime >= MAIN_LOOP_SENSOR_PROCESS_INTERVAL_MS) {
        lastSensorProcessTime = currentMillis;
        processTemperatureSensor();
        // processLightSensor();
        myLightSensor.process(MQTTclient);
        processPir();
    }

    // Use static to preserve lastWiFiConnectivityCheck value between loop() calls
    static unsigned long lastWiFiConnectivityCheck = 0;
    if (currentMillis - lastWiFiConnectivityCheck >= MAIN_LOOP_WIFI_CHECK_INTERVAL_MS) {
        lastWiFiConnectivityCheck = currentMillis;
        checkWifi();
    }

    // Core maintenance
    ArduinoOTA.handle();
    resetWatchdog(); // Manages its own interval via ESP32_WATCHDOG_RESET_INTERVAL_SECS
    heartBeatLED.update();

    // Use static to preserve lastMainLoopTelemetryCheck value between loop() calls
    static unsigned long lastMainLoopTelemetryCheck = 0;
    if (currentMillis - lastMainLoopTelemetryCheck >= MAIN_LOOP_TELEMETRY_INTERVAL_MS) {
        lastMainLoopTelemetryCheck = currentMillis;
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
    // checkForPageRequest(); // Uncomment if HTTP server is active
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
