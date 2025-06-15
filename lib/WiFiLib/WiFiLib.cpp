#include "debug.h"
#include "config.h"

#include "WiFiLib.h"
#include "../../src/secret.h"


#include "WebSerial.h"
extern WebSerial myWebSerial;

u_long previousConnCheckMillis = 0;
// u_long intervalConnCheckMillis = 30000;

void checkWifi() {
    unsigned long currentMillis = millis();
    if ((currentMillis - previousConnCheckMillis) > WIFI_CONNECTION_CHECK_INTERVAL) {
        DEBUG_PRINTLN("Checking if wifi is connected");

        if (!WiFi.isConnected()) {  //!= WL_CONNECTED)
            myWebSerial.println("Wifi Needs reconnecting");
            connectWiFi();
        } else {
            DEBUG_PRINTLN("OK - WiFi is connected");
        }
        previousConnCheckMillis = currentMillis;
    }
}


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


void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    myWebSerial.print("SSID: ");
    myWebSerial.println(WiFi.SSID().c_str());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    myWebSerial.print("IP Address: ");
    myWebSerial.println(ip.toString().c_str());

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    myWebSerial.print("signal strength (RSSI):");
    myWebSerial.print(String(rssi).c_str());
    myWebSerial.println(" dBm");
}
