
#include "ZoneController.h"
#include <NewRemoteTransmitter.h>

// check a unit and see if restart reqd
ZoneController::ZoneController(int zoneID, int remoteSocketID,
                               const char *zoneName,
                               const char *strHeartBeatText) {
    id_number = zoneID; // 0 to 2
    zone = zoneID + 1;  // 1 to 3
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

void ZoneController::manageRestarts(NewRemoteTransmitter transmitter) {
    // now check if need to reboot a device
    if ((millis() - lastGoodAckMillis) >
        maxMillisNoAckFromPi) { // over time limit so reboot first time in then
                                // just upadte time each other time

        // printFreeRam();
        if (!isRebooting) { // all this done first time triggered
            isRebooting = true; // signal device is rebooting
            isPowerCycling = true; // signal in power cycle

            powerCycle(transmitter);

            isPowerCycling = false; // signal in power cycle
            powerCyclesSincePowerOn++;
            rebootMillisLeft = waitForPiPowerUpMillis;
            lastRebootMillisLeftUpdate = millis();

            Serial.println("power cycled in manageRestarts");

            Serial.println(rebootMillisLeft);
        } else // is rebooting
        {      // this executes till end of reboot timer
            // device is rebooting now - do some stuff to update countdown
            // timers wait for pi to come back up - do nothing millis since last
            // update
            unsigned long millisLapsed = millis() - lastRebootMillisLeftUpdate;

            // test if next subtraction will take us to/over limit
            if (millisLapsed >= rebootMillisLeft) {
                // zero or neg will be reached reached
                rebootMillisLeft = 0;
            } else { // otherwise ok to do timer subtraction
                rebootMillisLeft = rebootMillisLeft - millisLapsed;
            }
            lastRebootMillisLeftUpdate = millis();

            // if next time  takes it below zero, act as if it was zero
            if (rebootMillisLeft == 0) { // reboot stuff completed here
                lastGoodAckMillis = millis();
                Serial.print("Assume Pi back up:");
                Serial.println(id_number);
                // printD("Assume pi back up");
                // printD2Str("Assume up:", name);
                isRebooting = false; // signal device has stopped rebooting
            }
        }
    }
}
void ZoneController::powerCycle(NewRemoteTransmitter transmitter) {
    // this is a blocking routine so need to keep checking messages and
    // updating vars etc
    // but do NOT do manage restarts as could be recursive and call this routine
    // again.
    Serial.println("sending off");
    transmitter.sendUnit(socketID, false);
    delay(3000);
    Serial.println("sending on");
    transmitter.sendUnit(socketID, true);
    Serial.println("power cycle completed");
}

void ZoneController::resetZoneDevice(void) {
    this->lastGoodAckMillis = millis();
    this->isRebooting = false;
    this->rebootMillisLeft = false;
}

// return a text string with current status of zone controller
// e.g ""
char *ZoneController::getStatus(char *statusMessage) {
    // all three lines can be displayed at once
    const char rebootMsg[] = {"Reboot: "};
    const char powerCycleMsg[] = {"Power Cycle"};
    //int zoneID; // only initialised once at start

    unsigned int secsSinceAck = 0; // max secs out considered good

    char str_output[20] = {0};
    unsigned int secsLeft;
    char buf[17];

    // do just for THIS zone controller
    secsSinceAck = (millis() - this->lastGoodAckMillis) / 1000;
    // make sure check for restarting device
    // if so return current secs in wait for reboot cycle
    if (this->isRebooting) {
        secsLeft = (this->rebootMillisLeft) / 1000UL;

        Serial.print("--rebootMillisLeft: ");
        Serial.println((this->rebootMillisLeft));

        Serial.print("--secsLeft var: ");
        Serial.println(secsLeft);

        // build string to show if cycling or coming back up
        strcpy(str_output, this->name);
        strcat(str_output, ": ");
        // char message[] = " Reboot: ";
        if (this->isPowerCycling) {
            strcat(str_output, powerCycleMsg);
            // printD(str_output);
            // myDisplay.writeLine(zoneID + 4, str_output);
            // secsLeft = '';
        } else {
            strcat(str_output, rebootMsg);
            // printDWithVal(str_output, secsLeft);
            sprintf(buf, "%d", secsLeft);
            strcat(str_output, buf);

            // myDisplay.writeLine(zoneID + 4, str_output);
        }
    } else if ((secsSinceAck > goodSecsMax)) {
        strcpy(str_output, this->name);
        strcat(str_output, ": ");
        strcat(str_output, this->badStatusMess);
        strcat(str_output, ": ");

        // printDWithVal(str_output, secsSinceAck);

        sprintf(buf, "%d", secsSinceAck);
        strcat(str_output, buf);
        // myDisplay.writeLine(zoneID + 4, str_output);
        // badLED();
        // LEDsOff();
    } else {
        // u8g2.print("u");
        strcpy(str_output, this->name);
        strcat(str_output, ": ");
        strcat(str_output, this->goodStatusMess);
        // add restarts soince power on
        strcat(str_output, " (");

        sprintf(buf, "%i", this->powerCyclesSincePowerOn);

        strcat(str_output, buf);
        strcat(str_output, ")");
        // myDisplay.writeLine(zoneID + 4, str_output);
        // goodLED();
    }
    strcpy(statusMessage, str_output); // copy status mess to loc
    return statusMessage;              // return pointer to status message
}
