#include "PIRSensor.h"
#include <NTPClient.h>

#include "SupportLib.h"
#include "config.h"
extern NTPClient timeClient;

PIRSensor::PIRSensor(uint8_t IPPin) : pin(IPPin) {
    pinMode(IPPin, INPUT);
}

// Define the getState method
bool PIRSensor::getState() const {
    return state;
}

bool PIRSensor::updateStateIfDue(void) {
    unsigned long nowMs = millis();

    if ((nowMs - lastReadMillis) > readIntervalMillis) {
        bool currentState = digitalRead(pin);
        if (currentState != state) {
            state = currentState;
            newStateFlag = true;
        }
        lastReadMillis = nowMs;
    }
    return getState();
}


bool PIRSensor::processPIRSensor(PubSubClient& MQTTclient) {

    // DEBUG_PRINTLN("PIRSensor::checkPIRSensor() called");

    updateStateIfDue();  // sample if due
    if (hasNewState()) {
        // publish the new state to MQTT
        MQTTclient.publish(publishPIRStateTopic, getState() ? "true" : "false"); // Publish state to the defined topic
        DEBUG_PRINT(timeClient.getFormattedTime().c_str());
        // The stringToPrint function formats and combines the given strings for debugging purposes.
        DEBUG_PRINTLN(stringToPrint("myPIRSensor.getState(): ", getState() ? "true" : "false"));

        clearNewStateFlag();
    }
    return getState();
}
