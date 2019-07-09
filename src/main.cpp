//#define DEBUG
#define RELEASE

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <NewRemoteTransmitter.h>
#include <RF24.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdlib.h>  // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
#include "Arduino.h"
#include "Display.h"
#include "LedFader.h"
#include "MyOTA.h"
#include "TempSensor.h"
#include "ZoneController.h"
#include "pins.h"
#include "secret.h"
#include "sendemail.h"
#include "version.h"

// time stuff
#include <NTPClient.h>
#include <WiFiUdp.h>
#define NTP_OFFSET 60 * 60      // In seconds, 0 for GMT, 60*60 for BST
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

// forward declarations

void doNonBlockingConnectionSetup(void);
void doConnectionRequiredTasks(void);
void doConnectionIndependantTasks(void);
void checkConnections(void);
// void updateDisplayData(void);
void resetESP32Watchdog(void);
// boolean processTouchPads(void);

void IRAM_ATTR resetModule();

// DHT22 stuff
TempSensor DHT22Sensor;

// MQTT stuff
#include <PubSubClient.h>
IPAddress mqttBroker(192, 168, 0, 200);
WiFiClient WiFiEClient;
PubSubClient MQTTClient(mqttBroker, 1883, MQTTRxcallback, WiFiEClient);

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
unsigned long previousConnCheckMillis = 0;
unsigned long intervalConnCheckMillis = 30000;

unsigned long intervalTempDisplayMillis = 60000;
unsigned long previousTempDisplayMillis =
    millis() - intervalTempDisplayMillis;  // trigger on start

// create the display object
Display myDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, OLED_CLOCK_PIN,
                  OLED_DATA_PIN);
ZoneController ZCs[3] = {ZoneController(0, 14, "GRG", "GGG"),
                         ZoneController(1, 4, "CNV", "CCC"),
                         ZoneController(2, 15, "SHD", "SSS")};

WiFiServer server(80);

// create object
// SendEmail e("smtp.gmail.com", 465, EMAIL_ADDRESS, APP_PASSWORD,
// 2000, true);
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

extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                           size_t length);

#include "WebPageLib.h"
#include "WiFiLib.h"

#include "SupportLib.h"
extern boolean processTouchPads(void);
extern char *getElapsedTimeStr();
extern void updateDisplayData();
extern void checkConnections();
extern displayModes displayMode;
extern boolean touchedFlag;  // = false;

#include "TouchPad.h"
TouchPad touchPad1 = TouchPad(TOUCH_SENSOR_1);
TouchPad touchPad2 = TouchPad(TOUCH_SENSOR_2);

// ! big issue - does not work when no internet connection - resolve
// hang on wifi connect etc
//!! poss fixed - !!RETEST

//#define myWEBHOOk
//"https://maker.ifttt.com/trigger/ESP32BridgeBoot/with/key/dF1NEy_aQ5diUyluM3EKcd"
#include <IFTTTWebhook.h>
IFTTTWebhook myWebhook(IFTTT_API_KEY, IFTTT_EVENT_NAME);

#include "PIRSensor.h"
PIRSensor myPIRSensor(PIR_PIN);
/**
 * @brief
 *
 */
//! WATCHDOG STUFF
#include "esp_system.h"
hw_timer_t *timer = NULL;
const int wdtTimeoutS = 40;
const int wdtTimeoutMs =
    wdtTimeoutS * 1000;  // time in ms to trigger the watchdog
unsigned long resetWatchdogIntervalMs = 20000;

void IRAM_ATTR resetModule() {
  ets_printf("ESP32 Rebooted by Internal Watchdog\n");
  esp_restart();
}

void setup() {
  //! Remeber no wifi or MQTT at this point till main loop

  Serial.begin(115200);
  Serial.println("==========running setup==========");
  // myWebSerial.println("==========running setup==========");
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
  // myDisplay.writeLine(4, "Connecting to WiFi..");
  // myDisplay.refresh();
  // connectWiFi();
  // you're connected now, so print out the status:
  // printWifiStatus();
  // server.begin();
  // CR;
  // myDisplay.writeLine(5, "Connecting to MQTT..");
  // myDisplay.refresh();
  // connectMQTT();
  myDisplay.writeLine(6, "DONE");
  myDisplay.refresh();
  Serial.println("setup 0");

  // timeClient.begin();
  //         Serial.println("setup 0.2");

  // timeClient.update();

  Serial.println("setup 0.5");

  Serial.println(timeClient.getFormattedTime());
  delay(900);

  // Send Email
  // e.send(EMAIL_ADDRESS, EMAIL_ADDRESS, EMAIL_SUBJECT, "programm
  // started/restarted"); myWebhook.trigger("433Bridge Boot/Reboot");
  // myWebhook.trigger();
  Serial.println("setup 1");

  myDisplay.wipe();

  connectWiFi();

  resetESP32Watchdog();
  Serial.println("setup 2");

  Serial.println("setup 3");

  Serial.println("setup 4");

  resetESP32Watchdog();

  // MQTTClient.
  myLightSensor.getLevel();
  Serial.println("setup END");
}

char tempString[] = "12345678901234567890";

//*** vars for non blocking wifi and MQTT functionality
//**
uint8_t conn_stat = 0;        // Connection status for WiFi and MQTT:
unsigned long waitCount = 0;  // counter
const char *Version = "{\"Version\":\"low_prio_wifi_v2\"}";
const char *Status = "{\"Message\":\"up and running\"}";
unsigned long lastStatus = 0;  // counter in example code for conn_stat == 5
unsigned long lastTask = 0;    // counter in example code for conn_stat <> 5

void loop() {
  doNonBlockingConnectionSetup();
  doConnectionRequiredTasks();
  doConnectionIndependantTasks();
}

void doNonBlockingConnectionSetup() {
  // start of non-blocking connection setup section
  if ((WiFi.status() != WL_CONNECTED) && (conn_stat != 1)) {
    conn_stat = 0;
  }
  if ((WiFi.status() == WL_CONNECTED) && !MQTTClient.connected() &&
      (conn_stat != 3)) {
    conn_stat = 2;
  }
  if ((WiFi.status() == WL_CONNECTED) && MQTTClient.connected() &&
      (conn_stat != 5)) {
    conn_stat = 4;
  }
  switch (conn_stat) {
    case 0:  // MQTT and WiFi down: start WiFi
      Serial.println("MQTT and WiFi down: start WiFi");
      WiFi.begin(MY_SSID, MY_SSID_PASSWORD);
      conn_stat = 1;
      break;
    case 1:  // WiFi starting, do nothing here
      Serial.println("WiFi starting, wait : " + String(waitCount));
      waitCount++;
      server.begin();

      break;
    case 2:  // WiFi up, MQTT down: start MQTT
      Serial.println("WiFi up, MQTT down: start MQTT");
      // MQTTClient.begin(mqtt_broker.c_str(), 8883, TCP); //   config MQTT
      // Server, use port 8883 for secure connection
      // MQTTClient.connect(Hostname, mqtt_user.c_str(), mqtt_pw.c_str());
      // MQTTClient.connect(mqttBroker);

      MQTTClient.connect("433BridgeMQTTClient");  // dosent work - see case 3

      timeClient.begin();
      // Serial.println("setup 0.2");

      // timeClient.update();
      webSocket.begin();
      webSocket.onEvent(webSocketEvent);
      setupOTA();

      myWebhook.trigger("433Bridge BootReboot");

      conn_stat = 3;
      waitCount = 0;
      break;
    case 3:  // WiFi up, MQTT starting, do nothing here
      Serial.println("WiFi up, MQTT starting, wait : " + String(waitCount));
      waitCount++;
      // must call connect here as wont work in case 2
      MQTTClient.connect("433BridgeMQTTClient");

      break;

    case 4:  // WiFi up, MQTT up: finish MQTT configuration
      Serial.println("WiFi up, MQTT up: finish MQTT configuration");
      // MQTTClient.subscribe(output_topic);
      MQTTClient.subscribe(subscribeTopic);

      // MQTTClient.publish(input_topic, Version);
      conn_stat = 5;
      break;
  }
  // end of non-blocking connection setup section
}

void doConnectionRequiredTasks() {
  // start section with tasks where WiFi/MQTT is required
  if (conn_stat == 5) {
    // if (millis() - lastStatus > 10000)
    // { // Start send status every 10 sec (just as an example)
    //     Serial.println(Status);
    //     //MQTTClient.publish(pub, Status); //      send status to broker
    //     MQTTClient.loop();     //      give control to MQTT to send message
    //     to broker lastStatus = millis(); //      remember time of last sent
    //     status message
    // }
    // ArduinoOTA.handle(); // internal household function for OTA
    // MQTTClient.loop();   // internal household function for MQTT
    MQTTClient.loop();     // process any MQTT stuff, returned in callback
    processMQTTMessage();  // check flags set above and act on
    ArduinoOTA.handle();
    webSocket.loop();
    timeClient.update();
    broadcastWS();

    checkForPageRequest();
    // if new readings taken, op to serial etc
    if (DHT22Sensor.publishReadings(MQTTClient, publishTempTopic,
                                    publishHumiTopic)) {
      myWebSerial.print("New-MQTT pub: ");
      myWebSerial.println(DHT22Sensor.getTempDisplayString(tempString));
    }
  }
  // end of section for tasks where WiFi/MQTT are required
}

void doConnectionIndependantTasks() {
  // start section for tasks which should run regardless of WiFi/MQTT
  // if (millis() - lastTask > 1000)
  // { // Print message every second (just as an example)
  //     Serial.println("print this every second");
  //     lastTask = millis();
  // }
  // delay(100);
  // end of section for tasks which should run regardless of WiFi/MQTT

  // MY NEW STUFF
  resetESP32Watchdog();
  checkLightSensor();
  checkPIRSensor();
  heartBeatLED.update();  // initialize
  updateDisplayData();
  processZoneRF24Message();  // process any zone watchdog messages
  if (ZCs[0].manageRestarts(transmitter) == true) {
    myWebhook.trigger("ESP32 Watchdog: Zone 1 power cycled");
  }
  // disbale zone 2 restarts for now
  ZCs[1].resetZoneDevice();

  if (ZCs[2].manageRestarts(transmitter) == true) {
    myWebhook.trigger("ESP32 Watchdog: Zone 3 power cycled");
  }
}

void resetESP32Watchdog(void) {
  static unsigned long lastResetWatchdogMillis = millis();

  if ((millis() - lastResetWatchdogMillis) >= resetWatchdogIntervalMs) {
    timerWrite(timer, 0);  // reset timer (feed watchdog)
    // myWebSerial.println("Reset Bridge Watchdog");
    Serial.println("Reset Bridge Watchdog");
    lastResetWatchdogMillis = millis();
  }
}
