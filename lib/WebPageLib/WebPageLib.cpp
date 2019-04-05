//#include "WebSocketLib.h"
#include "WebPageLib.h"
#include "../../src/secret.h"


extern WiFiServer  server;


#include <NTPClient.h>
extern NTPClient timeClient;

#include "TempSensor.h"
extern TempSensor DHT22Sensor;

#include "MQTTLib.h"
extern char *getMQTTDisplayString(char *MQTTDisplayString);

#include "ZoneController.h"
extern ZoneController ZCs[];

#include "WebSerial.h"
extern WebSerial myWebSerial; 

#include "webPageText.h"

void checkForPageRequest(void)
{
    // static char tempDisplayString[30];
    // static char humiDisplayString[30];
    // static char zone1DisplayString[30];
    // static char zone3DisplayString[30];
    // static char MQTTDisplayString[30];

    WiFiClient client = server.available(); // listen for incoming clients
    //client = server.available();
    if (client)
    {                                  // if you get a client,
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        //!send out the web page
                        client.print(webPageString);
                        // client.println("<meta http-equiv='refresh' content='5'>");
                        // client.println(timeClient.getFormattedTime());
                        // client.print("<br>");

                        // //Serial.println("!----------! BIG_TEMP Display Refresh");
                        // client.print(DHT22Sensor.getTempDisplayString(tempDisplayString));
                        // client.print("<br>");
                        // client.print(DHT22Sensor.getHumiDisplayString(humiDisplayString));
                        // client.print("<br>");
                        // client.print(getMQTTDisplayString(MQTTDisplayString));
                        // client.print("<br>");

                        // client.print(ZCs[0].getDisplayString(zone1DisplayString));
                        // client.print("<br>");
                        // client.print(ZCs[2].getDisplayString(zone3DisplayString));
                        // client.print("<br>");
                        // // Serial.println(zone3DisplayString);
                        // //Serial.println("^----------^");
                        // //! now push out the buffer
                        // client.print("buffer op<br><br>");
                        // client.print(myWebSerial.getBuffer());

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /H"))
                {
                    //digitalWrite(LED_PIN, HIGH);               // GET /H turns the LED on
                }
                if (currentLine.endsWith("GET /L"))
                {
                    //digitalWrite(LED_PIN, LOW);                // GET /L turns the LED off
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}

void WiFiLocalWebPageCtrl(void)
{
    static char tempDisplayString[30];
    static char humiDisplayString[30];
    static char zone1DisplayString[30];
    static char zone3DisplayString[30];
    static char MQTTDisplayString[30];

    WiFiClient client = server.available(); // listen for incoming clients
    //client = server.available();
    if (client)
    {                                  // if you get a client,
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println("<meta http-equiv='refresh' content='5'>");
                        client.println(timeClient.getFormattedTime());
                        client.print("<br>");

                        //Serial.println("!----------! BIG_TEMP Display Refresh");
                        client.print(DHT22Sensor.getTempDisplayString(tempDisplayString));
                        client.print("<br>");
                        client.print(DHT22Sensor.getHumiDisplayString(humiDisplayString));
                        client.print("<br>");
                        client.print(getMQTTDisplayString(MQTTDisplayString));
                        client.print("<br>");

                        client.print(ZCs[0].getDisplayString(zone1DisplayString));
                        client.print("<br>");
                        client.print(ZCs[2].getDisplayString(zone3DisplayString));
                        client.print("<br>");
                        // Serial.println(zone3DisplayString);
                        //Serial.println("^----------^");
                        //! now push out the buffer
                        client.print("buffer op<br><br>");
                        client.print(myWebSerial.getBuffer());

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /H"))
                {
                    //digitalWrite(LED_PIN, HIGH);               // GET /H turns the LED on
                }
                if (currentLine.endsWith("GET /L"))
                {
                    //digitalWrite(LED_PIN, LOW);                // GET /L turns the LED off
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
}