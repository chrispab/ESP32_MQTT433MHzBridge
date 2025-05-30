#include "PIRSensor.h"
////

PIRSensor::PIRSensor(uint8_t IPPin) : pin(IPPin)
{
    pinMode(IPPin, INPUT);
}

bool PIRSensor::readState(void)
{
    currentState = digitalRead(pin);
    if (currentState != state){
        newStateFlag= true;
        state = currentState;
    }
    return state;
}

// bool PIRSensor::getState(void)
// {
//     currentState = digitalRead(pin);
//     if (currentState != state){
//         newStateFlag= true;
//         state = currentState;
//     }
//     return state;
// }
