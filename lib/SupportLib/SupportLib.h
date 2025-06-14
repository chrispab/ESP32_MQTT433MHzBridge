#ifndef __SUPPORT_LIB
#define __SUPPORT_LIB

#include <NTPClient.h>
#include <U8g2lib.h> // For U8g2 font types like u8g2_font_6x12_tf
extern NTPClient timeClient;

// /**
//  * @brief Get the current time as a formatted string.
//  * @return Pointer to a static buffer containing the formatted time string.
//  */
// char* getTimeStr();

// /**
//  * @brief Get the elapsed time as a formatted string.
//  * @return Pointer to a static buffer containing the elapsed time string.
//  */
// char* getElapsedTimeStr();

// /**
//  * @brief Update the display data with the latest sensor and status information.
//  */
// void updateDisplayData();

// /**
//  * @brief Check WiFi connection and reconnect if necessary.
//  */
// void checkWifi();

// // /**
// //  * @brief Check the PIR sensor and update state if motion is detected.
// //  */
// // bool checkPIRSensor();

// // /**
// //  * @brief Process the light sensor and publish updates if needed.
// //  */
// // void processLightSensor();

// /**
//  * @brief Convert a literal and value to a formatted string.
//  * @param literal The literal string.
//  * @param value The value string.
//  * @return Pointer to a static buffer containing the formatted string.
//  */
// char* stringToPrint(const char* literal, const char* value);

// enum displayModes {
//     NORMAL,
//     BIG_TEMP,
//     MULTI
// };

namespace Fonts {
    // constexpr auto SYS_FONT_OLD = u8g2_font_8x13_tf;
    constexpr const uint8_t* SYS_FONT = u8g2_font_6x12_tf;        // 7 px high
    constexpr const uint8_t* BIG_TEMP_FONT = u8g2_font_fub30_tf;  // 30px height
    // Example of another font, if needed:
    // constexpr const uint8_t* ANOTHER_FONT = u8g2_font_inb33_mf; // 33px height (example)
}

#endif
