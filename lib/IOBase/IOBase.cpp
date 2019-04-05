#include "IOBase.h"
#include "config.h"

IOBase::IOBase()
{
    state = false;
    newStateFlag = false;
    defaultState = false;
    prevStateChangeMillis = millis();
    onMillis = 0;
    offMillis = 0;
}

bool IOBase::getState()
{
    return state;
}
void IOBase::setState(bool pnewStateFlag)
{
    if (pnewStateFlag != state)
    {
        state = pnewStateFlag;
        newStateFlag = true;
    }
}
bool IOBase::hasNewState()
{
    return newStateFlag;
}
bool IOBase::clearNewStateFlag()
{
    //ensures MQTT pub only sent once per state change since last readState
    newStateFlag = false; //indicate data read and used e.g MQTT pub
    return state;
}

void IOBase::setOnMillis(long ponMillis)
{
    onMillis = ponMillis;
}
void IOBase::setOffMillis(long poffMillis)
{
    offMillis = poffMillis;
}
