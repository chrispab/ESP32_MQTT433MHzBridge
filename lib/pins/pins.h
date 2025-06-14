#pragma once // Good practice for include guard

// It's assumed that GPIO_NUM_X and T4 macros are defined by including Arduino.h or esp-idf headers before this file.
// If not, this file would need to include the necessary headers.

//! Pin GPIO usage
//Note that GPIO_NUM_34 – GPIO_NUM_39 are input mode only
//refer to  cct diag for extra info
namespace Pins {
    constexpr int ESP32_ONBOARD_BLUE_LED_PIN = GPIO_NUM_2; // RHS_P_4 esp32 devkit on board blue LED
    constexpr int GREEN_LED_PIN              = GPIO_NUM_33; //LHS_P_9
    constexpr int DHTPIN                     = GPIO_NUM_25; // LHS_P_8
    constexpr int TX433PIN                   = GPIO_NUM_32; //LHS_P_10
    constexpr int RF24_CE_PIN                = GPIO_NUM_5;  //RHS_P_8
    constexpr int RF24_CS_PIN                = GPIO_NUM_4;  //RHS_P_5
    constexpr int RF24_SPI_CLK               = GPIO_NUM_18; //RHS_P_9 green wire
    constexpr int RF24_SPI_MISO              = GPIO_NUM_19; //RHS_P_10 purple wire
    constexpr int RF24_SPI_MOSI              = GPIO_NUM_23; //RHS_P_15 blue wire
    constexpr int OLED_CLOCK_PIN             = GPIO_NUM_22; //RHS_P_14 SCL
    constexpr int OLED_DATA_PIN              = GPIO_NUM_21; //RHS_P_11 SDA
    constexpr int RED_LED_PIN                = GPIO_NUM_26; //LHS_P_7  ??

    constexpr int TOUCH_SENSOR_1             = GPIO_NUM_13; //LHS_P_3
    constexpr int TOUCH_SENSOR_2             = GPIO_NUM_12; //LHS_P_4 -
    constexpr int TOUCH_SENSOR_3             = GPIO_NUM_14; //LHS_P_5
    constexpr int TOUCH_SENSOR_4             = GPIO_NUM_27; //LHS_P_6
    constexpr int TOUCH_SENSOR_5             = GPIO_NUM_15; // RHS_P_3  !! also used by 433 Tx??? - resolve

    //currently exposed on box via screw head
    constexpr int TOUCH_PIN                  = T4;          // ESP32 Pin gpio13 rhs of panel (T4 is an ESP-IDF touch pad enum/macro)
    //gpio12 is the other exposed screw head
    constexpr int LDR_PIN                    = GPIO_NUM_36; // ADC_0 LHS_P_14
    constexpr int PIR_PIN                    = GPIO_NUM_16; // RHS_P_6
}
