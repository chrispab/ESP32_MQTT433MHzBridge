// #include <Arduino.h>
#include <WebSerial.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

//char displayLine[DISPLAY_LINES][31]; // 6 lines of n chars +terminator for dispaly store

WebSerial::WebSerial()
{
}
// redraw the display with contents of displayLine array
char *WebSerial::getBuffer(void)
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
    return consoleBuffer;
}
String WebSerial::getString(void)
{
    String str(consoleBuffer);
    return str;
}
boolean WebSerial::hasData(void)
{
    if (strlen(consoleBuffer)) //returns len
    {
        return true; //non zero so has a length
    }

    return false; //else return 'string has no data'
}

void WebSerial::clearBuffer(void)
{
    consoleBuffer[0] = '\0';//ste null at pos 1 to indicate end of string
    return;
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

    //! do whole lines
    if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1))
    {
        memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20), BUFF_SIZE - newLineLen - 20);
        //memmove(&items[k + 1], &items[k], (numItems - k - 1) * sizeof(double));
        //items[k] = value;
        strcat(consoleBuffer, lineText);
        strcat(consoleBuffer, "<br>");
    }
    else
    {
        //if not - add the string
        strcat(consoleBuffer, lineText);
        strcat(consoleBuffer, "<br>");

        //else remove
    }
}
