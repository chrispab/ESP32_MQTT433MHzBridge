#ifndef __SUPPORT_LIB
#define __SUPPORT_LIB

bool processTouchPads(void);
char *getElapsedTimeStr();
void updateDisplayData();
void checkConnections();
void checkLightSensor();
void checkPIRSensor();




enum displayModes
{
    NORMAL,
    BIG_TEMP,
    MULTI
} ;

//#define SYS_FONT u8g2_font_8x13_tf
#define SYS_FONT u8g2_font_6x12_tf       // 7 px high
#define BIG_TEMP_FONT u8g2_font_fub30_tf //30px hieght
// 33 too big - #define BIG_TEMP_FONT u8g2_font_inb33_mf

#endif
