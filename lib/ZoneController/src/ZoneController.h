#ifndef ZoneController_h
#define ZoneController_h

#include <Arduino.h>
#include <My433Transmitter.h>
#include "config.h"

class ZoneController
{

public:
  ZoneController(int zoneID, int socketID, const char *name, const char *heartBeatText);
  boolean manageRestarts(My433Transmitter transmitter);
  void powerCycle(My433Transmitter transmitter);
  void resetZoneDevice(void);
  char *getDisplayString(char *statusMessage);



  char heartBeatText[4]; // allow enough space for text plus 1 extra for null terminator
  unsigned long lastGoodAckMillis;
  bool isRebooting; //indicate if booting up
  unsigned long rebootMillisLeft;
  char name[4];
  bool isPowerCycling;     //indicate if power cycling
  char badStatusMess[10];  // enough for 1 line on display
  char goodStatusMess[10]; // enough for 1 line on display
  uint8_t powerCyclesSincePowerOn;

private:
  //single for all instances - share this data
  static const unsigned int goodSecsMax = ZONE_WAIT_BEFORE_FLAG_AWAY;                         //20
  // static const unsigned long maxMillisNoAckFromPi = 1000UL * 420UL;   //300 max millisces to wait if no ack from pi before power cycling pi
  // static const unsigned long waitForPiPowerUpMillis = 1000UL * 180UL; //120
  static const unsigned long maxMillisNoAckFromPi = ZONE_HEARTBEAT_TIMEOUT_MS;   //300 max millisces to wait if no ack from pi before power cycling pi
  static const unsigned long waitForPiPowerUpMillis = ZONE_COLD_BOOT_TIME_MS; //120

  uint8_t id_number;
  uint8_t zone;
  uint8_t socketID;

  unsigned long lastRebootMillisLeftUpdate;
};

#endif //ZoneController_h
