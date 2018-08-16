
#include "TempSensor.h"
//#include <NewRemoteTransmitter.h>

// check a unit and see if restart reqd
TempSensor::TempSensor() {
    // init sensor
    // unsigned long currentMillis;
    intervalSensorReadMillis = 30000;
    previousSensorReadMillis = millis() - intervalSensorReadMillis;
    Serial.println("==== constructor Sensor Read Millis data ====");
    Serial.println(intervalSensorReadMillis);
    Serial.println(previousSensorReadMillis);
}

// returns true if temp has been updated
boolean TempSensor::takeReadings(void) {
    currentMillis = millis();

    // throttle here and only read if passed read interval
    if (currentMillis - previousSensorReadMillis > intervalSensorReadMillis) {
        Serial.println("Taking New sensor readings......");

        readSensor(); // update temperature and humidity properties
        if (!(getStatus() == ERROR_NONE)) {
            Serial.println(getStatusString());
        }

        // update other props
        dtostrf(temperature, 4, 1, temperatureString);
        dtostrf(humidity, 4, 1, humidityString);

        previousSensorReadMillis = currentMillis;
        return true;
    }
    return false;
}

char *TempSensor::getTemperatureString() { return temperatureString; }

char *TempSensor::getHumidityString() { return humidityString; }

void TempSensor::publishReadings(PubSubClient MQTTclient,
                                 char *publishTempTopic,
                                 char *publishHumiTopic) {
    // get current readings and pub via mqtt
    if (takeReadings()) {

        char messageString[20];
        MQTTclient.publish(publishTempTopic, temperatureString);
        MQTTclient.publish(publishHumiTopic, humidityString);
        // format for serial print
        strcpy(messageString, getTemperatureString());
        strcat(messageString, "\xb0"); // degree symbol
        strcat(messageString, "C");    // suffix
        Serial.print("MQTT published Temperature: ");
        Serial.println(getTemperatureString());
        Serial.print("MQTT published Humidity: ");
        Serial.println(getHumidityString());
    }
}

// get status string from temp sensor
char *TempSensor::getTempDisplayString(char *displayString) {

    strcpy(displayString, "Temp: ");
    strcat(displayString, temperatureString);
    if (isnan(temperature)) {
        // Serial.println("Failed to read from DHT sensor!");
        strcat(displayString, "-=NaN=-");
    } else {                           // is a number
        strcat(displayString, "\xb0"); // degree symbol
        strcat(displayString, "C");    // add suffix degrees C
    }

    return displayString; // pass back pointer to the temp string
}
char *TempSensor::getHumiDisplayString(char *displayString) {

    strcpy(displayString, "Humi: ");
    strcat(displayString, humidityString);
    if (isnan(humidity)) {
        // Serial.println("Failed to read from DHT sensor!");
        strcat(displayString, "-=NaN=-");
    } else {                           // is a number
        strcat(displayString, "\%"); // degree symbol
        //strcat(displayString, "C");    // add suffix degrees C
    }

    return displayString; // pass back pointer to the temp string
}