
#include "TempSensor.h"
//#include <NewRemoteTransmitter.h>

// check a unit and see if restart reqd
TempSensor::TempSensor()
{
    // init sensor
    // unsigned long currentMillis;
    intervalSensorReadMillis = 30000;
    previousSensorReadMillis = millis() - intervalSensorReadMillis;
    Serial.println("==== constructor Sensor Read Millis data ====");
    Serial.println(intervalSensorReadMillis);
    Serial.println(previousSensorReadMillis);
}

// returns true if temp has been updated
// takes a new reading every intervalSensorReadMillis - in class header
boolean TempSensor::takeReadings(void)
{
    float prevTemperature;
    float prevHumidity;

    currentMillis = millis();

    // throttle here and only read if passed read interval
    if (currentMillis - previousSensorReadMillis > intervalSensorReadMillis)
    {
        //Serial.println("Taking New sensor readings......");

        prevTemperature = temperature; //get last temp
        prevHumidity = humidity;       //get last temp

        this->readSensor(); // update temperature and humidity properties
        if (!(getStatus() == ERROR_NONE))
        {
            Serial.println(getStatusString());
        }
        //prevTemp = temperature;//get last temp
        if (isnan(temperature))
        { //current reading bad
            //replace with old reading
            temperature = prevTemperature;
            Serial.println("reading BAD from SENSOR..using old TEMP value");
        }
        if (isnan(humidity))
        { //current reading bad
            //replace with old reading
            humidity = prevHumidity;
            Serial.println("reading BAD from SENSOR..using old HUMIDITY value");
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

//return true if new reading taken and published
//return false if not done
boolean TempSensor::publishReadings(PubSubClient MQTTclient,
                                 char *publishTempTopic,
                                 char *publishHumiTopic)
{
    // get current readings and pub via mqtt
    if (takeReadings())//new readings taken?
    {
        MQTTclient.publish(publishTempTopic, temperatureString);
        MQTTclient.publish(publishHumiTopic, humidityString);
        return true;
    }
    return false;
}

//! check for nan and use previous temp val
// get status string from temp sensor
char *TempSensor::getTempDisplayString(char *displayString)
{

    strcpy(displayString, "Temp: ");
    //strcat(displayString, temperatureString);
    if (isnan(temperature))
    {
        // Serial.println("Failed to read from DHT sensor!");
        strcat(displayString, "------");
    }
    else
    {
        strcat(displayString, temperatureString); // is a number
//        strcat(displayString, "\xb0");            // degree symbol
        strcat(displayString, "o");            // degree symbol
        strcat(displayString, "C");               // add suffix degrees C
    }

    return displayString; // pass back pointer to the temp string
}

char *TempSensor::getHumiDisplayString(char *displayString)
{

    strcpy(displayString, "Humi: ");
    strcat(displayString, humidityString);
    if (isnan(humidity))
    {
        // Serial.println("Failed to read from DHT sensor!");
        strcat(displayString, "-=NaN=-");
    }
    else
    {                                // is a number
        strcat(displayString, "\%"); // degree symbol
        //strcat(displayString, "C");    // add suffix degrees C
    }

    return displayString; // pass back pointer to the temp string
}