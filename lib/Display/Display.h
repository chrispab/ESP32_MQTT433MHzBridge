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
	// Display(const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE)
	// 	: U8G2()
	// {
	// 	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
	// 	u8x8_SetPin_HW_I2C(getU8x8(), reset, clock, data);
	// };

	
	Display(const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE);
	// 	: U8G2()
    //void setFont()
	void writeLine(int lineNumber, const char *lineText);
	void refresh(void);
	void wipe(void);
	void setup();
	void printO(int x, int y, const char *text);
	void printO(const char *message);
	void printOWithVal(const char *message, int value);

	void printO2Str(const char *str1, const char *str2);
};

#endif
