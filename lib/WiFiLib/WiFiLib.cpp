#include "debug.h"
#include "config.h"

#include "WiFiLib.h"
#include "../../src/secret.h"

// WiFi settings
// const char ssid[] = MY_SSID;
// const char pass[] = MY_SSID_PASSWORD;
const char* ssid = MY_SSID;
const char* pass = MY_SSID_PASSWORD;
int status = WL_IDLE_STATUS;

#include "WebSerial.h"
extern WebSerial myWebSerial;

#include "WebSerial.h"
extern WiFiServer server;

u_long previousConnCheckMillis = 0;
// u_long intervalConnCheckMillis = 30000;

#ifndef WIFI_CONNECTION_CHECK_INTERVAL
#define WIFI_CONNECTION_CHECK_INTERVAL 30000
#endif


void checkWifi() {
    unsigned long currentMillis = millis();
    if ((currentMillis - previousConnCheckMillis) > WIFI_CONNECTION_CHECK_INTERVAL) {
        DEBUG_PRINTLN("Checking if wifi is connected");

        if (!WiFi.isConnected()) {  //!= WL_CONNECTED)
            myWebSerial.println("Wifi Needs reconnecting");
            connectWiFi();
        } else {
            DEBUG_PRINTLN("OK - WiFi is connected");
// #ifdef DEBUG_WSERIAL
//             myWebSerial.println("OK - WiFi is connected");
// #endif
        }
        previousConnCheckMillis = currentMillis;
    }
}


void connectWiFi()
{

    //! ensure non blocking so can act as zone watchdog

    bool wifiConnectTimeout = false;
    unsigned long startMillis;
    unsigned long timeOutMillis = 3000;

    WiFi.begin(ssid, pass);//move out or if

    startMillis = millis();
    myWebSerial.println("Attempting to connect to SSID: ");
    myWebSerial.println(ssid);
    while (!WiFi.isConnected() && !wifiConnectTimeout)
    {
        // enable jump out if connection attempt has timed out
        //WiFi.reconnect();

        wifiConnectTimeout =
            ((millis() - startMillis) > timeOutMillis) ? true : false;
    }

    wifiConnectTimeout ? myWebSerial.println("WiFi Connection attempt Timed Out!")
                       : myWebSerial.println("Wifi Connection made!");

    server.begin();
}


void connectWiFiOld()
{

    //! ensure non blocking so can act as zone watchdog

    bool wifiConnectTimeout = false;
    unsigned long startMillis;
    unsigned long timeOutMillis = 5000;

    WiFi.begin(ssid, pass);//move out or if

//! if wifi notconnected then
//wifi begin
//{}

    startMillis = millis();
    myWebSerial.println("Attempting to connect to SSID: ");
    myWebSerial.println(ssid);
    while (!WiFi.isConnected() && !wifiConnectTimeout)
    {
        // enable jump out if connection attempt has timed out
        //WiFi.reconnect();

        wifiConnectTimeout =
            ((millis() - startMillis) > timeOutMillis) ? true : false;
    }

    wifiConnectTimeout ? myWebSerial.println("WiFi Connection attempt Timed Out!")
                       : myWebSerial.println("Wifi Connection made!");

    server.begin();
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}
