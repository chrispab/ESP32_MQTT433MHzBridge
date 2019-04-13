#include "PIRSensor.h"
////

PIRSensor::PIRSensor(uint8_t IPPin) : pin(IPPin)
{
    //state = false;
    //currentLevel = 500; //start level??
    //newStateFlag = true;
    //newLevelFlag = true;
    pinMode(IPPin, INPUT);
}

