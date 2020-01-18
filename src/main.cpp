// //#define DEBUG
// #define RELEASE
#include "config.h"

#include "Arduino.h"
#include "version.h"
#include <NewRemoteTransmitter.h>
#include <RF24.h>
#include <WiFi.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "MyOTA.h"
#include "Display.h"
#include "ZoneController.h"
#include "secret.h"
#include "LedFader.h"
#include "TempSensor.h"
#include "sendemail.h"
#include "pins.h"

#include "SupportLib.h"

//time stuff
#include <NTPClient.h>
#include <WiFiUdp.h>
#define NTP_OFFSET 0  * 60     // In seconds, 0 for GMT, 60*60 for BST
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "europe.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
//NTPClient timeClient(ntpUDP);

#include "MQTTLib.h"
extern char subscribeTopic[];   // = "433Bridge/cmnd/#";
extern char publishTempTopic[]; // = "433Bridge/Temperature";
extern char publishHumiTopic[]; // = "433Bridge/Humidity";
extern bool MQTTNewData;
// forward decs
void checkConnections(void);
//void updateDisplayData(void);
void resetWatchdog(void);
//boolean processTouchPads(void);

void IRAM_ATTR resetModule();

// DHT22 stuff
TempSensor DHT22Sensor;

//MQTT stuff
#include <PubSubClient.h>
IPAddress mqttBroker(192, 168, 0, 200);
WiFiClient WiFiEClient;
PubSubClient MQTTclient(mqttBroker, 1883, MQTTRxcallback, WiFiEClient);

// 433Mhz settings
// 282830 addr of 16ch remote
// param 3 is pulse width, last param is num times control message  is txed
#include "My433Transmitter.h"
//NewRemoteTransmitter transmitter(282830, TX433PIN, 260, 4);
My433Transmitter transmitter(282830, TX433PIN, 260, 4);

#include "RF24Lib.h" //// Set up nRF24L01 rf24Radio on SPI bus plus pins 7 & 8
RF24 rf24Radio(RF24_CE_PIN, RF24_CS_PIN);

#include "LightSensor.h"
LightSensor myLightSensor(LDR_PIN);

// Global vars
unsigned long currentMillis = 0;
unsigned long previousConnCheckMillis = 0;
unsigned long intervalConnCheckMillis = 30000;

unsigned long intervalTempDisplayMillis = 60000;
unsigned long previousTempDisplayMillis =
    millis() - intervalTempDisplayMillis; // trigger on start

// create the display object
Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, OLED_CLOCK_PIN, OLED_DATA_PIN);
//def zone controllers, 2nd parm is socketID (0-15)
ZoneController ZCs[3] = {ZoneController(0, 13, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 14, "SHD", "SSS")};

WiFiServer server(80);

// create object
//SendEmail e("smtp.gmail.com", 465, EMAIL_ADDRESS, APP_PASSWORD,
//2000, true);
// set parameters. pin 13, go from 0 to 255 every n milliseconds
#define HEART_BEAT_TIME 500
LedFader heartBeatLED(GREEN_LED_PIN, 1, 0, 25, HEART_BEAT_TIME, true);
LedFader warnLED(RED_LED_PIN, 2, 0, 255, 451, true);

#include <WebSerial.h>
WebSerial myWebSerial;

#include "WebSocketLib.h"

WebSocketsServer webSocket = WebSocketsServer(81);

//#define EMAIL_SUBJECT "ESP32 Bridge - REBOOTED"

#include "WebSocketLib.h"

extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

#include "WiFiLib.h"
#include "WebPageLib.h"

#include "SupportLib.h"
extern boolean processTouchPads(void);
extern char *getElapsedTimeStr();
extern void updateDisplayData();
extern void checkConnections();
extern displayModes displayMode;
extern boolean touchedFlag; // = false;

#include "TouchPad.h"
TouchPad touchPad1 = TouchPad(TOUCH_SENSOR_1);
TouchPad touchPad2 = TouchPad(TOUCH_SENSOR_2);

// ! big issue - does not work when no internet connection - resolve
//hang on wifi connect etc
//!! poss fixed - !!RETEST

//#define myWEBHOOk "https://maker.ifttt.com/trigger/ESP32BridgeBoot/with/key/dF1NEy_aQ5diUyluM3EKcd"
// #include <IFTTTWebhook.h>
// IFTTTWebhook myWebhook(IFTTT_API_KEY, IFTTT_EVENT_NAME);

#include "PIRSensor.h"
PIRSensor myPIRSensor(PIR_PIN);


#include <RestClient.h>
//char restHost[]="homested.local";
//char restHost[]="192.168.0.40";
char restHost[]="chrisiot.com";

//RestClient client = RestClient(restHost, 443);
RestClient client = RestClient(restHost, 80);

boolean initit = true;
/**
 * @brief 
 * 
 */
//! WATCHDOG STUFF
#include "esp_system.h"
hw_timer_t *timer = NULL;
const int wdtTimeoutS = 60;
const int wdtTimeoutMs = wdtTimeoutS * 1000; //time in ms to trigger the watchdog
unsigned long resetWatchdogIntervalMs = 20000;

void IRAM_ATTR resetModule()
{
    ets_printf("ESP32 Rebooted by Internal Watchdog\n");
    esp_restart();
}

//------------------------------------------------------------
unsigned long previousAPIWriteMillis =0;
void doRest()
{
    char postParameter[79];
        char postMessage[255];

    String postValue = "";
    postValue.toCharArray(postParameter, 79);
    unsigned long intervalAPIWriteMillis=20000;
    unsigned long currentMillis=millis();
    String dateTimeStr="";
     String postStrFull="";

     String postStr1="";
String postStr2="";
    //do every 5 secs
    if ( currentMillis - previousAPIWriteMillis > intervalAPIWriteMillis){
        client.setHeader("Accept: application/json");
        
        //local auth token
        //client.setHeader("Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjI1MjE2OWI3MTMxNjVlNTczNWU1MGUwMGY2NzZhYjdiOGYwYTUzMTY0YWEyZTdiYzdiMzAzMDMzNzE4ZmRlMmE5M2QwZDdlODEwMDI1NDMwIn0.eyJhdWQiOiIzIiwianRpIjoiMjUyMTY5YjcxMzE2NWU1NzM1ZTUwZTAwZjY3NmFiN2I4ZjBhNTMxNjRhYTJlN2JjN2IzMDMwMzM3MThmZGUyYTkzZDBkN2U4MTAwMjU0MzAiLCJpYXQiOjE1Njc0NDkyOTEsIm5iZiI6MTU2NzQ0OTI5MSwiZXhwIjoxNTk5MDcxNjkwLCJzdWIiOiIyIiwic2NvcGVzIjpbXX0.Q8i63MAVgbGjRTYilydHHb0ljHvKhkeSANJbJ-qD_8_uWhPC_vQUrAC67mL3DmHm3pZkOvNm5WTAx5zQpTfOq-nJkB4c6vUytjQmyQNG-eV8WF90q_ccO5jbljlHORvfUzDF7TJRgKwO4Dcl8lWSQYgta3g_MkgH42qJHg9HEbGOKvgAvGsMsmeouLKwYojN8Oh02gKCQ_T7hcUkcB3zWVH9_ltV3qiSqA66VMyT45NzMuz3yxOYbSwXWaJl4AgiMs96LBDnpqMZzJIIYJ2YMMkXdaYljJhHga6vsGgxwc9HrZM2ZdY4EJcRcokVc6S6TGIJLEeGuIgGet-qDXhTEN832ufwh8saETrH_D_isnDohMEOkHjwWHkfcF4kfoYvQyD5jTg7DP4zqMDIE7uQmdiWDES512nByqmpzWNenIIMKZ1e5nT2EqvLDT21mdHhF35JzL0FUWd341xXTqjJLV27lfX3HAcs0pn69kY5X7Wqb4GNnEKlU-BbV-d6tBMNQI6yDcnKFYE2eJADtauMzmcAr_nNRqf212jqjLjblrqH1Qaoh1ZGHHnITUPd6Ai5uZa_x-phv1sTK4IaWwdtLn4RTQEWfiR1wVYePkfVM9xl1eTuiRrTfwAmRu-flCTCC66_ZobhYqLLmOssImK-GrxOmqQFC15zgC6PxklihpE");
        
        //chrisiot auth token
        client.setHeader("Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjZhOWExZTg1MTEwM2JlNDgxYWQ1Nzk0ZGMzNGM5NTkzYjk0NDk4YjQ3ZjA1YjBiOTU5ODg2YzM4YzRmMmRmODMyZmE1ODZjZjE1NDFmZjBmIn0.eyJhdWQiOiIxIiwianRpIjoiNmE5YTFlODUxMTAzYmU0ODFhZDU3OTRkYzM0Yzk1OTNiOTQ0OThiNDdmMDViMGI5NTk4ODZjMzhjNGYyZGY4MzJmYTU4NmNmMTU0MWZmMGYiLCJpYXQiOjE1Njc3ODQ3MDIsIm5iZiI6MTU2Nzc4NDcwMiwiZXhwIjoxNTk5NDA3MTAyLCJzdWIiOiIzIiwic2NvcGVzIjpbXX0.lmMl9R3CHo9ODPwwYcLH3tDrqolIyJgVeQWBVJF41RYzHmZDkZzFG9oL_trt1ewwDqYWsx_G7_Ka6_rwrQoKvefR4KY7HzMXACc-iiKpjoYX4Ersd3tXqFuj0AdkM7xzLjzPWHJhFleHjrrMwNuITD2-YXHGqjznCr5mCsfgTxfW0h3sEpKTv3DBukGScPmPFzPn-hL0-tmDZHImuQAwT6aDVjdEMJfSgtrkGDmF1CaXPi27JL8TjbCvGA2cyuNp6wpuutsqi9UuKTt_gQbrH9hsVxOwgS3GST2GMhWlbGx9vWkrilUWnkOVpSR0RzLzRLb-8se4BPOsi3Jer_h1pXNSKlOYylpeRZm_9Qd_ooI6YI7PIuU0ZN9hj5QDeRbq2JVfXMnBgI9X9x9cJEpWu7vuWtJCntVTvSVJqaBVoh0SCQn9yzpPfj5wJjuw1EZN-w8nphb5vgw-LTfjv4QeBdZ9Vo9VoUrUnNng6Ki3_uNzbvZOiCDQ8sKWBBHPQ421Rv-Z2UFghNAsG7_GL9_n6etFrKch34CGIAqPjcF1fJhTv3ERQh3ep5Ym_LsWxCYFv7WkKOAP1mxoPWNPtBvQ9ms5SHYZNnUtOzPTL6HDWuAllwZFOyVj7YmCZm1imN4d1dvUlhCAfkgd33ZNPREaGxjJyAyRMYE9D_Y7K0pnMNg");
        

        //build the POST string
        postStr1 =  "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        dateTimeStr = timeClient.getFormattedDateTime(0);
        //postStr2 =  "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        postStrFull = postStr1 + dateTimeStr;
        //replace any spaces (esp the one bet date and time) with %20
        postStrFull.replace(" ","%20");
        postStrFull.toCharArray(postMessage, 255);
        //int statusCode = client.post("/api/todo?topic=/test/topic&content={content:body}&published_at=2019-08-12 21:12:26.987", postParameter);
        //int statusCode = client.post("/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=2019-08-12%2021:12:26.123", postParameter);
        int statusCode = client.post(postMessage, postParameter);
        //int statusCode = client.post("/api/todo?topic=/test/topic&amp; content={content:body}&amp; published_at=2019-08-12 21:12:26.123", postParameter);
      //POST /api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=2019-08-12%2021:12:26.123 HTTP/1.1\r\n
        
        Serial.print("Status code from server: ");
        Serial.println(statusCode);
        previousAPIWriteMillis = currentMillis;

    }
}

//extern void storeREST(topic, payload, published_at);
// unsigned long previousAPIWriteMillis =0;
void storeREST(char* topic, char* payload, char* published_at)
{
    char postParameter[79];
        char postMessage[255];

    String postValue = "";
    postValue.toCharArray(postParameter, 79);
    //unsigned long intervalAPIWriteMillis=20000;
    unsigned long currentMillis=millis();
    String dateTimeStr="";
     String postStrFull="";

     String postStr1="";
String postStr2="";
String postTopic="";
String postPayload="";
String published_atStr="";

    //do every 5 secs
  //  if ( currentMillis - previousAPIWriteMillis > intervalAPIWriteMillis){
        client.setHeader("Accept: application/json");
        
        //local auth token
        //client.setHeader("Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjI1MjE2OWI3MTMxNjVlNTczNWU1MGUwMGY2NzZhYjdiOGYwYTUzMTY0YWEyZTdiYzdiMzAzMDMzNzE4ZmRlMmE5M2QwZDdlODEwMDI1NDMwIn0.eyJhdWQiOiIzIiwianRpIjoiMjUyMTY5YjcxMzE2NWU1NzM1ZTUwZTAwZjY3NmFiN2I4ZjBhNTMxNjRhYTJlN2JjN2IzMDMwMzM3MThmZGUyYTkzZDBkN2U4MTAwMjU0MzAiLCJpYXQiOjE1Njc0NDkyOTEsIm5iZiI6MTU2NzQ0OTI5MSwiZXhwIjoxNTk5MDcxNjkwLCJzdWIiOiIyIiwic2NvcGVzIjpbXX0.Q8i63MAVgbGjRTYilydHHb0ljHvKhkeSANJbJ-qD_8_uWhPC_vQUrAC67mL3DmHm3pZkOvNm5WTAx5zQpTfOq-nJkB4c6vUytjQmyQNG-eV8WF90q_ccO5jbljlHORvfUzDF7TJRgKwO4Dcl8lWSQYgta3g_MkgH42qJHg9HEbGOKvgAvGsMsmeouLKwYojN8Oh02gKCQ_T7hcUkcB3zWVH9_ltV3qiSqA66VMyT45NzMuz3yxOYbSwXWaJl4AgiMs96LBDnpqMZzJIIYJ2YMMkXdaYljJhHga6vsGgxwc9HrZM2ZdY4EJcRcokVc6S6TGIJLEeGuIgGet-qDXhTEN832ufwh8saETrH_D_isnDohMEOkHjwWHkfcF4kfoYvQyD5jTg7DP4zqMDIE7uQmdiWDES512nByqmpzWNenIIMKZ1e5nT2EqvLDT21mdHhF35JzL0FUWd341xXTqjJLV27lfX3HAcs0pn69kY5X7Wqb4GNnEKlU-BbV-d6tBMNQI6yDcnKFYE2eJADtauMzmcAr_nNRqf212jqjLjblrqH1Qaoh1ZGHHnITUPd6Ai5uZa_x-phv1sTK4IaWwdtLn4RTQEWfiR1wVYePkfVM9xl1eTuiRrTfwAmRu-flCTCC66_ZobhYqLLmOssImK-GrxOmqQFC15zgC6PxklihpE");
        
        //chrisiot auth token
        client.setHeader("Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjZhOWExZTg1MTEwM2JlNDgxYWQ1Nzk0ZGMzNGM5NTkzYjk0NDk4YjQ3ZjA1YjBiOTU5ODg2YzM4YzRmMmRmODMyZmE1ODZjZjE1NDFmZjBmIn0.eyJhdWQiOiIxIiwianRpIjoiNmE5YTFlODUxMTAzYmU0ODFhZDU3OTRkYzM0Yzk1OTNiOTQ0OThiNDdmMDViMGI5NTk4ODZjMzhjNGYyZGY4MzJmYTU4NmNmMTU0MWZmMGYiLCJpYXQiOjE1Njc3ODQ3MDIsIm5iZiI6MTU2Nzc4NDcwMiwiZXhwIjoxNTk5NDA3MTAyLCJzdWIiOiIzIiwic2NvcGVzIjpbXX0.lmMl9R3CHo9ODPwwYcLH3tDrqolIyJgVeQWBVJF41RYzHmZDkZzFG9oL_trt1ewwDqYWsx_G7_Ka6_rwrQoKvefR4KY7HzMXACc-iiKpjoYX4Ersd3tXqFuj0AdkM7xzLjzPWHJhFleHjrrMwNuITD2-YXHGqjznCr5mCsfgTxfW0h3sEpKTv3DBukGScPmPFzPn-hL0-tmDZHImuQAwT6aDVjdEMJfSgtrkGDmF1CaXPi27JL8TjbCvGA2cyuNp6wpuutsqi9UuKTt_gQbrH9hsVxOwgS3GST2GMhWlbGx9vWkrilUWnkOVpSR0RzLzRLb-8se4BPOsi3Jer_h1pXNSKlOYylpeRZm_9Qd_ooI6YI7PIuU0ZN9hj5QDeRbq2JVfXMnBgI9X9x9cJEpWu7vuWtJCntVTvSVJqaBVoh0SCQn9yzpPfj5wJjuw1EZN-w8nphb5vgw-LTfjv4QeBdZ9Vo9VoUrUnNng6Ki3_uNzbvZOiCDQ8sKWBBHPQ421Rv-Z2UFghNAsG7_GL9_n6etFrKch34CGIAqPjcF1fJhTv3ERQh3ep5Ym_LsWxCYFv7WkKOAP1mxoPWNPtBvQ9ms5SHYZNnUtOzPTL6HDWuAllwZFOyVj7YmCZm1imN4d1dvUlhCAfkgd33ZNPREaGxjJyAyRMYE9D_Y7K0pnMNg");
        

        //build the POST string
        //"/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        postStr1 =  "/api/todo?"; //"/test/topic""&content=%7Bcontent:body%7D&published_at=";
        postTopic="topic=";
        postTopic+= topic;
        postPayload = "&content=";
        postPayload+= payload;
        dateTimeStr = timeClient.getFormattedDateTime(0);
        published_atStr = "&published_at=";
        published_atStr+= String(published_at);
        //postStr2 =  "/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=";
        postStrFull = postStr1 + postTopic + postPayload + published_atStr; //dateTimeStr;
        //replace any spaces (esp the one bet date and time) with %20
        postStrFull.replace(" ","%20");
        postStrFull.replace("{","%7B");
        postStrFull.replace("}","%7D");

        postStrFull.toCharArray(postMessage, 255);
        //int statusCode = client.post("/api/todo?topic=/test/topic&content={content:body}&published_at=2019-08-12 21:12:26.987", postParameter);
        //int statusCode = client.post("/api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=2019-08-12%2021:12:26.123", postParameter);
        int statusCode = client.post(postMessage, postParameter);
        //int statusCode = client.post("/api/todo?topic=/test/topic&amp; content={content:body}&amp; published_at=2019-08-12 21:12:26.123", postParameter);
      //POST /api/todo?topic=/test/topic&content=%7Bcontent:body%7D&published_at=2019-08-12%2021:12:26.123 HTTP/1.1\r\n
        
        Serial.print("Status code from server: ");
        Serial.println(statusCode);
        previousAPIWriteMillis = currentMillis;

   // }
}

void setup()
{
    Serial.begin(115200);
    myWebSerial.println("==========running setup==========");
    heartBeatLED.begin();                        // initialize
    warnLED.begin();                             // initialize
    pinMode(ESP32_ONBOARD_BLUE_LED_PIN, OUTPUT); // set the LED pin mode

    timer = timerBegin(0, 8000, true); // timer 0, 80mhz div 8000
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, wdtTimeoutMs * 10, false); //set time in us
    timerAlarmEnable(timer);                          // enable interrupt

    // setup OLED display
    displayMode = NORMAL;
    displayMode = BIG_TEMP;
    //displayMode = MULTI;
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
    //server.begin();
    CR;
    myDisplay.writeLine(5, "Connecting to MQTT..");
    myDisplay.refresh();
    connectMQTT();
    myDisplay.writeLine(6, "DONE");
    myDisplay.refresh();
    timeClient.begin();
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    delay(200);

    //Send Email
    //e.send(EMAIL_ADDRESS, EMAIL_ADDRESS, EMAIL_SUBJECT, "programm started/restarted");
    //myWebhook.trigger("433Bridge Boot/Reboot");
    //myWebhook.trigger();

    myDisplay.wipe();
    //connectWiFi();
    resetWatchdog();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    setupOTA();
    resetWatchdog();

    //MQTTclient.
    // myWebhook.trigger("433Bridge Boot/Reboot");
    myLightSensor.getLevel();
        //client.begin(MY_SSID, MY_SSID_PASSWORD);
    //initit = true;
}

/**
 * @brief 
 * 
 */
//text buffer for main loop
char tempString[] = "12345678901234567890";

// rest vars
void loop()
{
#ifdef DEBUG_WSERIAL
    Serial.print("1..");
#endif

    //doRest();
    checkLightSensor();
    //checkPIRSensor();
    checkConnections(); // and reconnect if reqd

    ArduinoOTA.handle();
    resetWatchdog();
    heartBeatLED.update(); // initialize
    webSocket.loop();

    timeClient.update();

    broadcastWS();
    //if new readings taken, op to serial etc
    // TODO make vital readings a priority
    // and publish
    //also do every minute when no new reading
    if (DHT22Sensor.publishReadings(MQTTclient, publishTempTopic, publishHumiTopic))
    {
            // Serial.print("1..");
#ifdef DEBUG_WSERIAL
        myWebSerial.print("=> New- Temp reading - MQTT pub: ");
        myWebSerial.println(DHT22Sensor.getTempDisplayString(tempString));
        //initin = false;
#endif

    }
    MQTTclient.loop();    // process any MQTT stuff, returned in callback
    processMQTTMessage(); // check flags set above and act on

    webSocket.loop();
    broadcastWS();
    //MQTTclient.loop(); // process any MQTT stuff, returned in callback
    ArduinoOTA.handle();

    // touchedFlag = touchPad1.getState();
    // (touchPad1.getState()) ? displayMode = MULTI : displayMode = BIG_TEMP;

    // if (touchPad2.getState())
    // {
    //     if (millis() % 2000 == 0)
    //     {
    //         warnLED.fullOn();
    //         delay(10);
    //         warnLED.fullOff();
    //         //!  MQTTclient.publish("433Bridge/Button1", "1");
    //     }
    //     displayMode = MULTI;
    // }
    // else
    // {
    //     displayMode = BIG_TEMP;
    // }
    ArduinoOTA.handle();

    updateDisplayData();
    ArduinoOTA.handle();

    webSocket.loop();
    broadcastWS();
    processZoneRF24Message(); // process any zone watchdog messages
    if (ZCs[0].manageRestarts(transmitter) == true)
    {
        // myWebhook.trigger("ESP32 Watchdog: Zone 1 power cycled");
    }
    broadcastWS();
    // disbale zone 2 restarts for now
    ZCs[1].resetZoneDevice();
    if (ZCs[2].manageRestarts(transmitter) == true)
    {
        // myWebhook.trigger("ESP32 Watchdog: Zone 3 power cycled");
            // myWebSerial.print("=> New- Temp reading - MQTT pub: ");

    }
    broadcastWS();
    webSocket.loop();

    ArduinoOTA.handle();

    //WiFiLocalWebPageCtrl();
    checkForPageRequest();
    //webSocket.loop();
    //broadcastWS();
}




/**
 * @brief 
 * 
 */
//extern unsigned long resetWatchdogIntervalMs;

void resetWatchdog(void)
{
    static unsigned long lastResetWatchdogMillis = millis();

    if ((millis() - lastResetWatchdogMillis) >= resetWatchdogIntervalMs)
    {
        timerWrite(timer, 0); // reset timer (feed watchdog)
        //get current time, prepend to message
        // myWebSerial.print("T:");
        // myWebSerial.print(timeClient.getFormattedTime().c_str());
        // myWebSerial.print(":");
                        myWebSerial.print(getTimeStr());

        myWebSerial.println("+> Reset Bridge Watchdog");
        lastResetWatchdogMillis = millis();
    }
}
