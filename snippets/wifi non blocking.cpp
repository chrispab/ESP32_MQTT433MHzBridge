#include <ESP8266WiFi.h>

const char* ssid = "********";
const char* password = "********";

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

bool ledState;


void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
  {
    Serial.print("Station connected, IP: ");
    Serial.println(WiFi.localIP());
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
  {
    Serial.println("Station disconnected");
  });

  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.begin(ssid, password);
}


void loop()
{
  digitalWrite(LED_BUILTIN, ledState);
  ledState = !ledState;
  delay(250);
}