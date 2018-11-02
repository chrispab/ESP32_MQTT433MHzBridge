
#include "TouchPad.h"
#include <Arduino.h>

TouchPad::TouchPad(int pin)
{
    _pin=pin;
    lastFilteredVal = 54;
    filteredVal = 54;
    filterConstant = 3; // 2 ishalf, 4 is quarter etc
    touchThreshold = 35;
    newTouchValue = 100;
    lastTouchReadMillis = millis();
    touchReadInterval = 25;
}

bool TouchPad::getState(void)
{

    // lastFilteredVal = 54;
    // filteredVal = 54;
    // filterConstant = 3; // 2 ishalf, 4 is quarter etc
    // touchThreshold = 32;
    // newTouchValue = 100;
    // only read touch sensors every 100ms
    // static unsigned long lastTouchReadMillis = millis();
    // unsigned long touchReadInterval = 25;

    if ((millis() - lastTouchReadMillis) >= touchReadInterval)
    {
        newTouchValue = touchRead(_pin);
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

            return true;
        }
        lastFilteredVal = filteredVal;

        return false; // no edge detected
    }
    return false; // no edge detected
}
