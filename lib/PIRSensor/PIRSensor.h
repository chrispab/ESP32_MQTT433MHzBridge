#ifndef __PIRSENSOR_H
#define __PIRSENSOR_H

#include <Arduino.h>
#include <PubSubClient.h>

#include "IOBase.h"

#ifndef PIR_READ_INTERVAL
#define PIR_READ_INTERVAL 1000  // ms, minimum interval between reading sensor
#endif

class PIRSensor : public IOBase {
   private:
    // bool state;
    int pin;
    // bool newLevelFlag;
    //  bool currentState;
    // u_int previousLevel = 500;
    u_int readIntervalMillis = PIR_READ_INTERVAL;  // min interval between reading sensor in ms
    int lastReadMillis = 0 - PIR_READ_INTERVAL;

   public:
    PIRSensor(uint8_t pin);

    bool getState() const;

    // int getPIRSensor();
    //  bool readState();
    // bool readStateIfDue(void);
    bool updateStateIfDue(void);
    // read the state of the PIR sensor
    // bool checkPIRSensor(PubSubClient& MQTTclient);
        bool processPIRSensor(PubSubClient& MQTTclient);

    // bool checkPIRSensor();
    // bool getState(); // get the state of the PIR sensor
};

#endif
