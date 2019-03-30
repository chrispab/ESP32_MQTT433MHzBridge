/*

 */

#ifndef _MY433TRANSMITTER_H
#define _MY433TRANSMITTER_H

#include <Arduino.h>
#include <NewRemoteTransmitter.h>


/**
 */
// 433Mhz settings
// 282830 addr of 16ch remote
// param 3 is pulse width, last param is num times control message  is txed

class My433Transmitter : public NewRemoteTransmitter {
  public:
  		My433Transmitter(unsigned long address, byte pin, unsigned int periodusec, byte repeats);
      void operateSocket(uint8_t socketID, uint8_t state);

};

#endif
