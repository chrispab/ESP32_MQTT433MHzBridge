#ifndef TempSensor_h
#define TempSensor_h
#include <DHT.h>
#include <PubSubClient.h>

class TempSensor : public DHT
{
public:
  TempSensor();
  char *getTempDisplayString(char *thistempStr);
  char *getHumiDisplayString(char *thishumiStr);

  boolean publishReadings(PubSubClient MQTTclient, char *publishTempTopic,
                       char *publishHumiTopic);
  char *getTemperatureString();
  char *getHumidityString();
  boolean takeReadings(void);

private:
  // single for all instances - share this data
  unsigned long currentMillis;
  unsigned long intervalSensorReadMillis; // = 30000;
  unsigned long previousSensorReadMillis; /// trigger on start
  //char humiStr[20];
  char displayString[20];
  //static float temperature;
  //static float humidity;
  char temperatureString[20];
  char humidityString[20];
};

#endif // TempSensor_h
