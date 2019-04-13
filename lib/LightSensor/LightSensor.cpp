#include "LightSensor.h"
////

LightSensor::LightSensor(uint8_t ADC_Pin) : pin(ADC_Pin)
{
    state = false;
    currentLevel = 500; //start level??
    newStateFlag = true;
    newLevelFlag = true;
    pinMode(ADC_Pin, INPUT);
}

u_int LightSensor::getLevel()
{
    int nowMs = millis();
    //only read a new sample if time is due
    if (nowMs > (lastReadMillis + readIntervalMillis))
    {
        currentLevel = analogRead(pin);
        if (currentLevel != previousLevel)
        {
            newLevelFlag = true;
            previousLevel = currentLevel;
        }
        lastReadMillis = nowMs;
    }
    return currentLevel;
}

bool LightSensor::hasNewLevel()
{
    return newLevelFlag;
}

u_int LightSensor::clearNewLevelFlag()
{
    newLevelFlag = false; //indicate new data has been read by the program
    return currentLevel;
    //return analogRead(pin);
}

bool LightSensor::getState()
{
    getLevel();        //initiate new sensor sample if due
    if (hasNewLevel()) //hystresis algorythm
    {
        clearNewLevelFlag();
        if (state == true)
        {
            if (currentLevel < lowerThresholdLevel)
            { //: #(currentLevel < lowerHys):
                state = false;
                newStateFlag = true;
            }
        }
        else //state is false
        {
            if (currentLevel > upperThresholdLevel)
            { //}: #(currentLevel > upperHys):
                state = true;
                newStateFlag = true;
            }
        }
    }
    return state;
}