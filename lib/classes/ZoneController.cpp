
#include "ZoneController.h"
#include <NewRemoteTransmitter.h>

//check a unit and see if restart reqd
ZoneController::ZoneController(int zoneID, int remoteSocketID, const char *zoneName, const char *strHeartBeatText)
{
    id_number = zoneID; //0 to 2
    zone = zoneID + 1;  //1 to 3
    lastGoodAckMillis = millis();
    isRebooting = false;
    isPowerCycling = false;
    rebootMillisLeft = 0;
    lastRebootMillisLeftUpdate = millis();
    powerCyclesSincePowerOn = 0;
    strcpy(badStatusMess, "Away");
    strcpy(goodStatusMess, "OK");

    // individual stuff
    socketID = remoteSocketID;
    strcpy(name, zoneName);
    strcpy(heartBeatText, strHeartBeatText);
}

void ZoneController::manageRestarts(NewRemoteTransmitter transmitter)
{
    // now check if need to reboot a device
    if ((millis() - lastGoodAckMillis) > maxMillisNoAckFromPi)
    { // over time limit so reboot first time in then just upadte time each other time

        //printFreeRam();
        if (!isRebooting)
        { // all this done first time triggered
            //printD("Reboot : ");

            isRebooting = true; //signal device is rebooting

            isPowerCycling = true; // signal in power cycle

            powerCycle(transmitter);

            isPowerCycling = false; // signal in power cycle
            powerCyclesSincePowerOn++;
            rebootMillisLeft = waitForPiPowerUpMillis;
            lastRebootMillisLeftUpdate = millis();

            Serial.println("triggered reboot in manage reboots");

            Serial.println(rebootMillisLeft);
            //delay(1000);
        }
        else
        { // this executes till end of reboot timer
            //device is rebooting now - do some stuff to update countdown timers
            //wait for pi to come back up - do nothing
            //millis since last update
            unsigned long millisLapsed = millis() - lastRebootMillisLeftUpdate;

            // next subtraction will take us to/over limit
            if (millisLapsed >= rebootMillisLeft)
            {
                //zero or neg reached
                rebootMillisLeft = false;
                //timerDone = 1;
            }
            else
            { // ok to do timer subtraction

                rebootMillisLeft =
                    rebootMillisLeft - millisLapsed;
            }
            lastRebootMillisLeftUpdate = millis();

            // if next time  takes it below zero, act as if it was zero
            if (rebootMillisLeft == 0)
            { // reboot stuff completed here
                //if (timerDone == 1) { // reboot stuff completed here
                lastGoodAckMillis = millis();
                Serial.print(F("Assume Pi back up:"));
                Serial.println(id_number);
                //printD("Assume pi back up");
                //printD2Str("Assume up:", name);
                isRebooting = false; //signal device has stopped rebooting
            }
        }
    }
}
void ZoneController::powerCycle(NewRemoteTransmitter transmitter)
{
    //this is a blocking routine so need to keep checking messages and
    //updating vars etc
    // but do NOT do manage restarts as could be recursive and call this routine again.
    int transmitEnable = 1;
    if (transmitEnable == 1)
    {
        Serial.println(F("sending off"));
        //badLED();
        //beep(1, 2, 1);
        // for (int i = 0; i < 3; i++)
        // { // turn socket off
        // processZoneMessage();
        // myDisplay.refresh();
        transmitter.sendUnit(socketID, false);
        // }
        // processZoneMessage();
        // myDisplay.refresh();
        delay(1000);
        // processZoneMessage();
        // myDisplay.refresh();
        delay(1000);
        // processZoneMessage();
        // myDisplay.refresh();
        delay(1000);
        // processZoneMessage();
        // myDisplay.refresh();
        // Switch Rxunit on
        Serial.println(F("sending on"));
        //printD2Str("Power on :", name);

        // for (int i = 0; i < 3; i++)
        // { // turn socket back on
        // processZoneMessage();
        // myDisplay.refresh();
        transmitter.sendUnit(socketID, true);
        // }
        // processZoneMessage();
        // myDisplay.refresh();
        //LEDsOff();
        //beep(1, 2, 1);
        Serial.println(F("complete"));
    }
    else
    {
        Serial.println(F("not transmitting"));
    }
}

void ZoneController::resetZoneDevice(void)
{
    lastGoodAckMillis = millis();
    isRebooting = 0;
    rebootMillisLeft = 0;
}