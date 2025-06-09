// #include "debug.h"
#include "config.h"
#include "LightSensor.h"
#include "SupportLib.h"

LightSensor::LightSensor(uint8_t ADC_Pin) : pin(ADC_Pin) {
    state = false;
    currentLevel = 500;  // start level??
    newStateFlag = true;
    newLevelFlag = true;
    pinMode(ADC_Pin, INPUT);
}

/**
 * @brief Reads the current light sensor level if the read interval has elapsed.
 *
 * This function checks if the specified read interval has passed since the last sensor reading.
 * If so, it reads the current analog value from the sensor pin, updates the current and previous
 * level values, sets a flag if the level has changed, and calls updateState(). The function always
 * returns the most recently read sensor level.
 *
 * @return u_int The current light sensor level.
 */
u_int LightSensor::readLevelIfDue() {
    u_int nowMs = millis();
    // only read a new sample if time is due
    if (nowMs > (lastReadMillis + readIntervalMillis)) {
        currentLevel = analogRead(pin);
        if (currentLevel != previousLevel) {
            newLevelFlag = true;
            previousLevel = currentLevel;
            updateState();
        }
        lastReadMillis = nowMs;
    }
    return currentLevel;
}

bool LightSensor::hasNewLevel() {
    return newLevelFlag;
}

void LightSensor::clearNewLevelFlag() {
    newLevelFlag = false;  // to indicate new data has been read/processed by the program
}

/**
 * @brief Updates the state of the light sensor based on the current light level and hysteresis thresholds.
 *
 * This function checks if there is a new light level reading using the hasNewLevel() method.
 * If a new level is detected, it applies a hysteresis algorithm to determine if the sensor's state
 * should change:
 * - If the current state is true (e.g., light detected) and the current level falls below the lower threshold,
 *   the state is set to false and a flag indicating a new state is set.
 * - If the current state is false (e.g., no light detected) and the current level rises above the upper threshold,
 *   the state is set to true and a flag indicating a new state is set.
 *
 * @return The updated state of the light sensor (true if light is detected, false otherwise).
 */
bool LightSensor::updateState() {
    if (hasNewLevel())  // hystresis algorythm
    {
        if (state == true) {
            if (currentLevel < lowerThresholdLevel) {  //: #(currentLevel < lowerHys):
                state = false;
                newStateFlag = true;
            }
        } else  // state is false
        {
            if (currentLevel > upperThresholdLevel) {  //}: #(currentLevel > upperHys):
                state = true;
                newStateFlag = true;
            }
        }
    }
    return state;
}

bool LightSensor::getState() {
    return state;
}

int LightSensor::getLevel() {
    return currentLevel;
}

/**
 * @param MQTTclient Reference to a PubSubClient instance used for publishing sensor data
 *                   to an MQTT broker. This client handles the connection and communication
 *                   with the MQTT server, allowing the LightSensor to send updates or readings.
 */
void LightSensor::process(PubSubClient& MQTTclient) {
    char str[8];

    readLevelIfDue();  // sample ldr value if due
    if (hasNewLevel()) {
        sprintf(str, "%d", getLevel());
        // DEBUG_PRINT("myLightSensor.getLevel(): ");
        // DEBUG_PRINTLN(str);
        
        DEBUG_PRINTLN(stringToPrint("myLightSensor.getLevel(): ",str));
        // DEBUG_PRINTLN(stringToPrint("myLightSensor.getState(): ", myLightSensor.getState() ? "true" : "false"));


        MQTTclient.publish(publishLightLevelTopic, str);
        clearNewLevelFlag();
    }
    if (hasNewState()) {
        MQTTclient.publish(publishLightStateTopic, getState() ? "true" : "false");
        clearNewStateFlag();
    }
}