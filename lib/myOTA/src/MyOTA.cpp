#include "MyOTA.h"
#include "Display.h"
////
extern Display myDisplay;

void setupOTA(void)
{
    /* create a connection at port 3232 */
    //ArduinoOTA.setPort(3232);
    /* we use mDNS instead of IP of ESP32 directly */
    //hostname.local
    ArduinoOTA.setHostname("MQTT433Bridge");
    ArduinoOTA.setPassword("iotsharing");

    /* this callback function will be invoked when updating start */
    ArduinoOTA.onStart([]() {
        Serial.println("Start updating");
        myDisplay.wipe();
        myDisplay.setFont(u8g2_font_fub25_tf); //30px hieght);
        myDisplay.writeLine(4, "OTA-U");
        myDisplay.refresh();
    });
    /* this callback function will be invoked when updating end */
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd updating");
        myDisplay.wipe();
        myDisplay.writeLine(1, "Done OTA update");
        myDisplay.writeLine(3, "Reboot ESP");
        myDisplay.refresh();
        delay(200);
    });
    /* this callback function will be invoked when a number of chunks of software was flashed
  so we can use it to calculate the progress of flashing */
    //char str[12];
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); }); //       restart in case of an error during OTA
    /* this callback function will be invoked when updating error */
    // ArduinoOTA.onError([](ota_error_t error) {
    //     Serial.printf("Error[%u]: ", error);
    //     if (error == OTA_AUTH_ERROR)
    //         Serial.println("Auth Failed");
    //     else if (error == OTA_BEGIN_ERROR)
    //         Serial.println("Begin Failed");
    //     else if (error == OTA_CONNECT_ERROR)
    //         Serial.println("Connect Failed");
    //     else if (error == OTA_RECEIVE_ERROR)
    //         Serial.println("Receive Failed");
    //     else if (error == OTA_END_ERROR)
    //         Serial.println("End Failed");
    //     ESP.restart();
    // });
    /* start updating */
    ArduinoOTA.begin();
    // Serial.print("ESP IP address: ");
    // Serial.println(WiFi.localIP());
    // Serial.println(ArduinoOTA.getHostname());
}
