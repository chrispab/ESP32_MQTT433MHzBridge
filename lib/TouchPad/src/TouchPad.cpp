#include "TouchPad.h"
#include <Arduino.h>

TouchPad::TouchPad(int pin)
{
    _pin = pin;
    state= false;
    rawValue = 0;
    lastFilteredVal = 54;
    filteredVal = 65;
    filterConstant = 3; // 2 ishalf, 4 is quarter etc
    touchThreshold = 40;
    newTouchValue = 100;
    lastTouchReadMillis = millis();
    touchReadInterval = 25;
}

int TouchPad::getValue(void)
{
    //return rawValue;
    return lastFilteredVal;
}
bool TouchPad::getState(void)
{
    //return rawValue;
    processTouchPad();
    return state;
}

bool TouchPad::processTouchPad(void)
{
    if ((millis() - lastTouchReadMillis) >= touchReadInterval)
    {
        newTouchValue = touchRead(_pin);
        rawValue = newTouchValue;
        //Serial.println("===touchRead");
        //Serial.println(newTouchValue);
        // if (newTouchValue<30){
        //     return true;
        // }else {
        //     return false;
        // }

        lastTouchReadMillis = millis();

        //! software addition filter here to even out spurious readings
        int diff;
        //filteredVal = filteredVal + (filteredVal )
        if (newTouchValue > filteredVal) //if going up - add onto filteredval
        {
            diff = (newTouchValue - filteredVal);
            filteredVal = filteredVal + (diff / filterConstant);
        }
        if (newTouchValue < filteredVal) //if going down - subtract  from filteredval
        {
            diff = (filteredVal - newTouchValue);
            filteredVal = filteredVal - (diff / filterConstant);
        }

        // !! detect edges
        if ((lastFilteredVal > touchThreshold) && (filteredVal < touchThreshold))
        { //high to low edge finger on
            Serial.print("$$$]_ Touch Val = ");
            Serial.println(newTouchValue);
            Serial.print("filtered Val = ");
            Serial.println(filteredVal);

            //displayMode = MULTI;
            lastFilteredVal = filteredVal;
            //lastTouchReadMillis = millis();
            state=true;
            return true;
        }
        if ((lastFilteredVal < touchThreshold) && (filteredVal > touchThreshold))
        { //low to high edge finger off
            Serial.print("$$$_[ Touch Val = ");
            Serial.println(newTouchValue);
            Serial.print("filtered Val = ");
            Serial.println(filteredVal);
            //displayMode = BIG_TEMP;
            lastFilteredVal = filteredVal;
            //lastTouchReadMillis = millis();
            state=false;
            return true;
        }
        lastFilteredVal = filteredVal;
        //Serial.println(lastFilteredVal);

        return false; // no edge detected
    }
    return false; // no edge detected
}
