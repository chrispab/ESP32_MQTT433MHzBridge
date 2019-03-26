//#include "WebSocketLib.h"
#include "WiFiLib.h"
#include "../../src/secret.h"

// WiFi settings
const char ssid[] = MY_SSID;
const char pass[] = MY_SSID_PASSWORD;
int status = WL_IDLE_STATUS;

#include "WebSerial.h"
extern WebSerial myWebSerial;

#include "WebSerial.h"
extern WiFiServer server;

void connectWiFi()
{
    bool wifiConnectTimeout = false;
    u16_t startMillis;
    u16_t timeOutMillis = 15000;

    //wifiConnectTimeout = false;
    WiFi.begin(ssid, pass);

    startMillis = millis();
    myWebSerial.println("Attempting to connect to SSID: ");
    myWebSerial.println(ssid);
    while (!WiFi.isConnected() && !wifiConnectTimeout)
    {
        // Connect to WPA/WPA2 network. Change this line if using open
        // or WEP network:
        // WiFi.begin(ssid, pass);
        //WiFi.begin(ssid, pass);

        // enable jump out if connection attempt has timed out
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
