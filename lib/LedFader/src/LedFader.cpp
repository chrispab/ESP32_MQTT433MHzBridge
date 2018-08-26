/*

Author: Nick Gammon
Date:   27 December 2012
updated for ESP32 usage
Chris Battisson
2 Aug 2018


Usage:

  LedFader ledName (pin, pwmChannel, minValue, maxValue, msPerCycle, initiallyActive);   // set parameters

  eg.

  LedFader laserTurrent (13, 1, 0, 255, 3000, true);   // set parameters. pin 5, go from 0 to 255 every 3 seconds

  laserTurrent.begin ();    // initialize
  laserTurrent.on ();       // turn on
  laserTurrent.off ();      // turn off
  bool isOn = laserTurrent.isOn ();  // is it currently on?

  laserTurrent.update ();   // call in loop function


      // EXAMPLE CODE ---------------------------------------------------

      // set up some LEDs
      //                 pin  min max  millis
      LedFader strobe     (3, 10, 200,  1000);
      LedFader navigation (5, 10, 200,   500);
      LedFader torpedoes  (6, 10, 200,   250);

      void setup()
        {
        strobe.begin ();
        navigation.begin ();
        torpedoes.begin ();
        }  // end of setup

      void loop()
        {
        // update lights
        strobe.update ();
        navigation.update ();
        torpedoes.update ();

        // do other useful stuff here ...

        }  // end of loop

*/

#include <LedFader.h>

// constructor
LedFader::LedFader (const byte pin,
                    const byte pwmChannel,
                    const byte minValue,
                    const byte maxValue,
                    const unsigned long msPerCycle,
                    const bool active,
                    const bool stopWhenOn) :
         pin_ (pin), pwmChannel_(pwmChannel), minValue_ (minValue), maxValue_ (maxValue), msPerCycle_ (msPerCycle)
   {
   startTime_ = 0;
   active_ = active;
   stopWhenOn_ = stopWhenOn;
   forwards_ = true;
   }  // end of  LedFader::LedFader

// set pin to output, get current time
void LedFader::begin ()
  {
  //pinMode (pin_, OUTPUT);
  //digitalWrite (pin_, LOW);
      ledcAttachPin(pin_, pwmChannel_); // assign  led pins to channels
          // Initialize channels
    // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
    ledcSetup(pwmChannel_, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcWrite(pwmChannel_, 255);      // test high output of all leds in sequence
    delay(1000);
    ledcWrite(pwmChannel_, 0);

  startTime_ = millis ();
  }  // end of LedFader::begin

// set pin to output, get current time
void LedFader::fullOn ()
  {
  //pinMode (pin_, OUTPUT);
  //digitalWrite (pin_, LOW);
//      ledcAttachPin(pin_, pwmChannel_); // assign  led pins to channels
          // Initialize channels
    // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
//    ledcSetup(pwmChannel_, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcWrite(pwmChannel_, 255);      // test high output of all leds in sequence
//    delay(1000);
//    ledcWrite(pwmChannel_, 0);

//  startTime_ = millis ();
  }  // end of LedFader::begin
void LedFader::fullOff ()
  {
  //pinMode (pin_, OUTPUT);
  //digitalWrite (pin_, LOW);
//      ledcAttachPin(pin_, pwmChannel_); // assign  led pins to channels
          // Initialize channels
    // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
//    ledcSetup(pwmChannel_, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcWrite(pwmChannel_, 0);      // test high output of all leds in sequence
//    delay(1000);
//    ledcWrite(pwmChannel_, 0);

//  startTime_ = millis ();
  }  // end of LedFader::begin


// call from loop to flash the LED
void LedFader::update ()
  {
  // do nothing if not active
  if (!active_)
    return;

  unsigned long now = millis ();
  unsigned long howFarDone = now - startTime_;
  if (howFarDone >= msPerCycle_)
    {
    forwards_ = !forwards_;  // change direction
    if (forwards_)
      //analogWrite (pin_, minValue_);
      ledcWrite(pwmChannel_, minValue_);

    else
      //analogWrite (pin_, maxValue_);
       ledcWrite(pwmChannel_, maxValue_);
    startTime_ = now;

    // stop when at required brightness?
    if (stopWhenOn_)
      active_ = false;
    }  // end of overshot time for this cycle
  else
    {
    unsigned long newValue;
    byte valDifference = maxValue_ - minValue_;

    if (forwards_)
      newValue = (howFarDone * valDifference) / msPerCycle_;
    else
      newValue = ((msPerCycle_ - howFarDone) * valDifference) / msPerCycle_;

    //analogWrite (pin_, newValue + minValue_);
      ledcWrite(pwmChannel_, newValue + minValue_);
    }  // end of still in same cycle


  } // end of LedFader::update

 // activate this LED
 void LedFader::on ()
   {
   active_ = true;
   startTime_ = millis ();
   forwards_ = true;
   }  // end of LedFader::on

 // deactivate this LED
 void LedFader::off ()
   {
   active_ = false;
   digitalWrite(pin_, LOW);
   }  // end of LedFader::off

 // is it active?
 bool LedFader::isOn () const
   {
   return active_;
   }  // end of LedFader::isOn

