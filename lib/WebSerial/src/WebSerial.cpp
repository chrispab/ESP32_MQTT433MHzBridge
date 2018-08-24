// #include <Arduino.h>
#include <WebSerial.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

//char displayLine[DISPLAY_LINES][31]; // 6 lines of n chars +terminator for dispaly store

WebSerial::WebSerial()
{
}

// redraw the display with contents of displayLine array
void WebSerial::refresh(void)
{
    // u8g2.begin();
    // clearBuffer();
    // // setFont(u8g2_font_8x13_tf);
    // for (int i = 0; i < DISPLAY_LINES; i++) {
    //     drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    // }
    // //delay(50);
    // sendBuffer();
    //delay(50);
}
// redraw the display with contents of displayLine array
void WebSerial::wipe(void)
{
    // u8g2.begin();
    // clearBuffer();
    // // setFont(u8g2_font_8x13_tf);
    // for (int i = 0; i < DISPLAY_LINES; i++) {
    //     strcpy(displayLine[i], " ");
    //     drawStr(0, ((i + 1) * 9) + (i * 1), displayLine[i]);
    // }
    // //delay(20);
    // sendBuffer();
}
// add-update a line of text in the display text buffer
void WebSerial::println(const char *lineText)
{
    // update a line in the diaplay text buffer
    //strcpy(displayLine[lineNumber - 1], lineText);
    //add new string to buffer
    //will adding it overflow the buffer?
    int newLineLen = strlen(lineText);

    if ((strlen(consoleBuffer) + newLineLen) > (BUFF_SIZE - 1))
    {
        memmove(&consoleBuffer, (&consoleBuffer[newLineLen]), BUFF_SIZE - newLineLen);
        //memmove(&items[k + 1], &items[k], (numItems - k - 1) * sizeof(double));
        //items[k] = value;
    }
    else
    {
        //if not - add the string
        strcat(consoleBuffer, lineText);
        //else remove
    }
}
