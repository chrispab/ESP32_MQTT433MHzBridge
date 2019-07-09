#include <WebSerial.h>
#include <stdlib.h>  // for dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);

// DO CHECKS FOR WIFI CONNECTED IN ALL FUNCS

WebSerial::WebSerial() {}
// redraw the display with contents of displayLine array
char *WebSerial::getBuffer(void) { return consoleBuffer; }

String WebSerial::getString(void) {
  String str(consoleBuffer);
  return str;
}

boolean WebSerial::hasData(void) {
  if (strlen(consoleBuffer))  // returns len
  {
    return true;  // non zero so has a length
  }
  return false;  // else return 'string has no data'
}

void WebSerial::clearBuffer(void) {
  consoleBuffer[0] = '\0';  // ste null at pos 1 to indicate end of string
  return;
}

// redraw the display with contents of displayLine array
void WebSerial::refresh(void) {}
// redraw the display with contents of displayLine array
void WebSerial::wipe(void) {}

// add-update a line of text in the display text buffer
void WebSerial::println(const char *lineText) {
  Serial.println(lineText);

  int newLineLen = strlen(lineText);

  //! do whole lines
  if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1)) {
    memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20),
            BUFF_SIZE - newLineLen - 20);
  }
  strcat(consoleBuffer, lineText);
  strcat(consoleBuffer, "<br>");
}

// print text with a u long number
void WebSerial::println(const char *lineText, unsigned long uLongNumber) {
  char buff1[] =
      "MQTT rxed [this/is/the/topic/for/this/mesage] : and finally the payload "
      "room in the string";
  char buff2[] =
      "MQTT rxed [this/is/the/topic/for/this/mesage] : and finally the payload "
      "room in the string";

  strcpy(buff1, lineText);
  sprintf(buff2, "%lu", uLongNumber);
  strcat(buff1, buff2);
  Serial.println(buff1);

  int newLineLen = strlen(buff1);

  //! do whole lines
  if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1)) {
    memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20),
            BUFF_SIZE - newLineLen - 20);
  }
  strcat(consoleBuffer, buff1);
  strcat(consoleBuffer, "<br>");
}

void WebSerial::print(const char *lineText) {
  Serial.println(lineText);

  int newLineLen = strlen(lineText);

  //! do whole lines
  if ((strlen(consoleBuffer) + newLineLen + 20) > (BUFF_SIZE - 1)) {
    memmove(&consoleBuffer, (&consoleBuffer[newLineLen] + 20),
            BUFF_SIZE - newLineLen - 20);
  }
  strcat(consoleBuffer, lineText);
}