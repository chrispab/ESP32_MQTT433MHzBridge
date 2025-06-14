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

// In your WiFiLib.cpp (or equivalent file)

#include "WiFiLib.h"
#include <WiFi.h>
// ... other necessary includes ...
extern WebSerial myWebSerial; // Assuming myWebSerial is accessible
// ...

bool connectWiFi() { // Return type changed to bool
    myWebSerial.println("Attempting to connect to WiFi...");
    WiFi.begin(MY_SSID, MY_SSID_PASSWORD); // MY_SSID and MY_SSID_PASSWORD from secret.h

    unsigned long startTime = millis();
    // Allow, for example, 10 seconds to connect
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 10000)) {
        delay(500);
        myWebSerial.print(".");
        // You might want to add your watchdog reset here if connection takes time
    }

    if (WiFi.status() == WL_CONNECTED) {
        myWebSerial.println("\nWiFi connected successfully!");
        printWifiStatus(); // Assuming this function prints IP, etc.
        return true; // Return true on success
    } else {
        myWebSerial.println("\nFailed to connect to WiFi.");
        // printWifiStatus(); // Optionally print status even on failure
        return false; // Return false on failure
    }
}

// ... rest of WiFiLib.cpp ...

// void connectWiFi()
// {

//     //! ensure non blocking so can act as zone watchdog

//     bool wifiConnectTimeout = false;
//     unsigned long startMillis;
//     unsigned long timeOutMillis = 3000;

//     WiFi.begin(ssid, pass);//move out or if

//     startMillis = millis();
//     myWebSerial.println("Attempting to connect to SSID: ");
//     myWebSerial.println(ssid);
//     while (!WiFi.isConnected() && !wifiConnectTimeout)
//     {
//         // enable jump out if connection attempt has timed out
//         //WiFi.reconnect();

//         wifiConnectTimeout =
//             ((millis() - startMillis) > timeOutMillis) ? true : false;
//     }

//     wifiConnectTimeout ? myWebSerial.println("WiFi Connection attempt Timed Out!")
//                        : myWebSerial.println("Wifi Connection made!");

//     server.begin();
// }


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
