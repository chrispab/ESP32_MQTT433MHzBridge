# ESP32_MQTT433MHzBridge
// note : for I2C problem use ;
// https://desire.giesecke.tk/index.php/2018/04/20/how-to-use-stickbreakers-i2c-improved-code/
//
// https://github.com/espressif/arduino-esp32/issues/1352
//
// also fix for u8g2lib using wire lib
// https://community.particle.io/t/i2c-lcd-display-getting-corrupted-solved/9767/78
// edit
// /home/chris/Projects/git/ESP32_MQTT433MHzBridge/.piolibdeps/U8g2_ID942/src/U8x8lib.cpp
// and add 2us delays - 3 off, start write, end transmission
//! for led pwm
// example here : https://github.com/kriswiner/ESP32/tree/master/PWM
// use GPIO13 , phys pin 3 up on LHS
// mod of gammon ledfader

/* I2C slave Address Scanner
for 5V bus
 * Connect a 4.7k resistor between SDA and Vcc
 * Connect a 4.7k resistor between SCL and Vcc
for 3.3V bus
 * Connect a 2.4k resistor between SDA and Vcc
 * Connect a 2.4k resistor between SCL and Vcc

A resistor value of 4.7KΩ is recommended
It is strongly recommended that you place a 10 micro farad capacitor between +ve and -ve as close to your ESP32 as you can

Note that GPIO_NUM_34 – GPIO_NUM_39 are input mode only. You can not use these pins
for signal output. Also, pins 6 (SD_CLK), 7 (SD_DATA0), 8 (SD_DATA1), 9
(SD_DATA2), 10 (SD_DATA3), 11 (SD_CMD) 16 (CS) and 17(Q) are used to interact
with the SPI flash chip ... you can not use those for other purposes.
When using pSRAM,
Strapping pins are GPIO0, GPIO2 and GPIO12.
TX and RX (as used for flash) are GPIO1 and GPIO3.
The data type called gpio_num_t is a C language enumeration with values corresponding to these names. It is recommended to use these values rather than
attempt to use numeric values.
*/

#### OTA upload
```
python ~/Projects/git/ESP32-IDF-ZoneController/components/arduino-esp32/tools/espota.py -i 192.168.0.230 -I 192.168.0.54 -p 3232 -P 3232 -a iotsharing -f ~/Projects/git/ESP32_MQTT433MHzBridge/.pioenvs/esp32dev/firmware.bin

/home/chris/Projects/git/ESP32-ZoneController/bin

python ~/Projects/git/ESP32-ZoneController/bin/espota.py -i 192.168.0.230 -I 192.168.0.54 -p 3232 -P 3232 -a iotsharing -f ~/Projects/git/ESP32_MQTT433MHzBridge/.pioenvs/esp32dev/firmware.bin


python ~/Projects/git/ESP32-IDF-ZoneController/components/arduino-esp32/tools/espota.py -i 192.168.0.230 -I 192.168.0.54 -p 3232 -P 3232 -a iotsharing -f ~/Projects/git/ESP32_MQTT433MHzBridge/.pio/build/esp32dev/firmware.bin

/home/chris/Projects/git/ESP32_MQTT433MHzBridge/tools

python ~/Projects/git/ESP32_MQTT433MHzBridge/tools/espota.py -i 192.168.0.230 -I 192.168.0.54 -p 3232 -P 3232 -a iotsharing -f ~/Projects/git/ESP32_MQTT433MHzBridge/.pio/build/esp32dev/firmware.bin

via VPN:
python ~/Projects/git/ESP32_MQTT433MHzBridge/tools/espota.py -i 192.168.0.230 -I 10.8.0.3 -p 3232 -P 3232 -a iotsharing -f ~/Projects/git/ESP32_MQTT433MHzBridge/.pio/build/esp32dev/firmware.bin

CQ71:
python tools/espota.py -i 192.168.0.230 -I 192.168.0.20 -p 3232 -P 3232 -a iotsharing -f .pio/build/esp32dev/firmware.bin

python tools/espota.py -i 192.168.0.230 -I 192.168.0.57 -p 3232 -P 3232 -a iotsharing -f .pio/build/esp32dev/firmware.bin
```


