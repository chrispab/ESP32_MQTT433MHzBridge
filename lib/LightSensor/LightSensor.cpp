#include "LightSensor.h"
////

LightSensor::LightSensor(uint8_t ADC_Pin) : pin(ADC_Pin)
{
    state = false;
    newStateFlag = true;
}


bool LightSensor::sampleState()
{
    int threshold = 500;
    //int sensorValue = getLightSensor();
    bool currentState;

    currentState = (getLightSensor() > threshold);

    if (currentState != state) //new reading value
    {
        state = currentState;
        newStateFlag = true;
    }
    return state;
}

int LightSensor::getLevel()
{
    return analogRead(pin);
}