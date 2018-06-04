/*

 */

#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <U8g2lib.h>

/**
*/
class Display : public U8G2
{
  public:
	Display(void){
		public : U8G2_SSD1306_128X64_NONAME_F_HW_I2C(const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) : U8G2(){
			u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
	u8x8_SetPin_HW_I2C(getU8x8(), reset, clock, data);
}
}
;

/**
		 * Send on/off command to the address group.
		 *
		 * @param switchOn  True to send "on" signal, false to send "off" signal.
		 */
void setup(void);

/**
		 * Send on/off command to an unit on the current address.
		 *
		 * @param unit      [0..15] target unit.
		 * @param switchOn  True to send "on" signal, false to send "off" signal.
		 */
void displayRefresh(void);

/**
		 * Send dim value to an unit on the current address. This will also switch on the device.
		 * Note that low bound can be limited on the dimmer itself. Setting a dimLevel of 0
		 * may not actually turn off the device.
		 *
		 * @param unit      [0..15] target unit.
		 * @param dimLevel  [0..15] Dim level. 0 for off, 15 for brightest level.
		 */
//void sendDim(byte unit, byte dimLevel);
void displayWipe(void);

void displayWriteLine(int lineNumber, const char *lineText);
/**
		 * Send dim value the current address group. This will also switch on the device.
		 * Note that low bound can be limited on the dimmer itself. Setting a dimLevel of 0
		 * may not actually turn off the device.
		 *
		 * @param dimLevel  [0..15] Dim level. 0 for off, 15 for brightest level.
		 */
//void sendGroupDim(byte dimLevel);

protected:
//unsigned long _address;		// Address of this transmitter.
//byte _pin;					// Transmitter output pin
//unsigned int _periodusec;	// Oscillator period in microseconds
//byte _repeats;				// Number over repetitions of one telegram

/**
		 * Transmits start-pulse
		 */
//void _sendStartPulse();

/**
		 * Transmits address part
		 */
//void _sendAddress();

/**
		 * Transmits unit part.
		 *
		 * @param unit      [0-15] target unit.
		 */
//void _sendUnit(byte unit);

/**
		 * Transmits stop pulse.
		 */
//void _sendStopPulse();

/**
		 * Transmits a single bit.
		 *
		 * @param isBitOne	True, to send '1', false to send '0'.
		 */
//void _sendBit(boolean isBitOne);
}
;
#endif
