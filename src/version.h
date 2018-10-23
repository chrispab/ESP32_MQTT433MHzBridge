#define SW_VERSION "V3.86 Br:\"master\""

#define TITLE_LINE1 "     ESP32"
#define TITLE_LINE2 "MQTT 433MhZ Bridge"
#define TITLE_LINE3 "Zone Wireless Dog"
#define TITLE_LINE4 "MQTT Temp Sensor"
#define TITLE_LINE5 "OTA enabled"
#define TITLE_LINE6 SW_VERSION
//#define SYS_FONT u8g2_font_8x13_tf
#define SYS_FONT u8g2_font_6x12_tf       // 7 px high
#define BIG_TEMP_FONT u8g2_font_fub30_tf //30px hieght
// 33 too big - #define BIG_TEMP_FONT u8g2_font_inb33_mf

//! Pin GPIO usage
//Note that GPIO_NUM_34 – GPIO_NUM_39 are input mode only
//refer to  cct diag for extra info
#define ESP32_ONBOARD_BLUE_LED_PIN GPIO_NUM_2 // RHS_P_4 esp32 devkit on board blue LED
#define GREEN_LED_PIN GPIO_NUM_33             //LHS_P_9
#define DHTPIN GPIO_NUM_25                    // LHS_P_8 what digital pin we're connected to
#define TX433PIN GPIO_NUM_32                  //LHS_P_10
#define RF24_CE_PIN GPIO_NUM_5                //RHS_P_8
#define RF24_CS_PIN GPIO_NUM_4                //RHS_P_5
#define RF24_SPI_CLK GPIO_NUM_18              //RHS_P_9 green wire
#define RF24_SPI_MISO GPIO_NUM_19             //RHS_P_10 purple wire
#define RF24_SPI_MOSI GPIO_NUM_23             //RHS_P_15 blue wire
#define OLED_CLOCK_PIN GPIO_NUM_22            //RHS_P_14 SCL
#define OLED_DATA_PIN GPIO_NUM_21             //RHS_P_11 SDA
#define RED_LED_PIN GPIO_NUM_26               //LHS_P_7  ??

#define TOUCH_SENSOR_1 GPIO_NUM_13 //LHS_P_3
#define TOUCH_SENSOR_2 GPIO_NUM_12 //LHS_P_4
#define TOUCH_SENSOR_3 GPIO_NUM_14 //LHS_P_5
#define TOUCH_SENSOR_4 GPIO_NUM_27 //LHS_P_6
#define TOUCH_SENSOR_5 GPIO_NUM_15 // RHS_P_3  !! also used by 433 Tx??? - resolve

#define TOUCH_PIN T4 // ESP32 Pin gpio13

#define CR Serial.println()
