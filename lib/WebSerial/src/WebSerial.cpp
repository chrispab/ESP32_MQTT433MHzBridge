#include <WebSerial.h>
#include <stdlib.h> // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

WebSerial::WebSerial()
{
}
// redraw the display with contents of displayLine array
char *WebSerial::getBuffer(void)
{
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
    consoleBuffer[0] = '\0'; //ste null at pos 1 to indicate end of string
    return;
}

// redraw the display with contents of displayLine array
void WebSerial::refresh(void)
{
}
// redraw the display with contents of displayLine array
void WebSerial::wipe(void)
{
}
// add-update a line of text in the display text buffer
void WebSerial::println(const char *lineText)
{

    // String statusString;
    // statusString = timeClient.getFormattedTime() + ": " + this.getBuffer();
    // Serial.print(statusString);

    Serial.println(lineText);

    int newLineLen = strlen(lineText);

    //! do whole lines
    if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1))
    {
        memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20), BUFF_SIZE - newLineLen - 20);
        //memmove(&items[k + 1], &items[k], (numItems - k - 1) * sizeof(double));
        //items[k] = value;
    }
    strcat(consoleBuffer, lineText);
    strcat(consoleBuffer, "<br>");
}
void WebSerial::print(const char *lineText)
{
    Serial.println(lineText);

    int newLineLen = strlen(lineText);

    //! do whole lines
    if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1))
    {
        memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20), BUFF_SIZE - newLineLen - 20);
        //memmove(&items[k + 1], &items[k], (numItems - k - 1) * sizeof(double));
        //items[k] = value;
    }
    strcat(consoleBuffer, lineText);
    //strcat(consoleBuffer, "<br>");
}