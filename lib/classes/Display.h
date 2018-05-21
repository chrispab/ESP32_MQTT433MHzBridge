/*

 */

#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <U8g2lib.h>


/**
*/
class Display: public U8G2 {
	public:
		/**
		* Constructor.
		*
		* To obtain the correct period length, use the ShowReceivedCodeNewRemote example, or you
		* can use an oscilloscope.
		*
		* @param address	Address of this transmitter [0..2^26-1] Duplicate the address of your hardware, or choose a random number.
		* @param pin		Output pin on Arduino to which the transmitter is connected
		* @param periodusec	Duration of one period, in microseconds. One bit takes 8 periods (but only 4 for 'dim' signal).
		* @param repeats	[0..8] The 2log-Number of times the signal is repeated. The actual number of repeats will be 2^repeats. 2 would be bare minimum, 4 seems robust, 8 is maximum (and overkill).
		*/
		//Display(void);
		Display(void);

		/**
		 * Send on/off command to the address group.
		 *
		 * @param switchOn  True to send "on" signal, false to send "off" signal.
		 */
		void setup(void );

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
};
#endif
