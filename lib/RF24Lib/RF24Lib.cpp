#include <RF24Lib.h>
#include "ZoneController.h"



//RF24 rf24Radio(RF24_CE_PIN, RF24_CS_PIN);
uint8_t writePipeLocS[] = "NodeS";
uint8_t readPipeLocS[] = "Node0";
uint8_t writePipeLocC[] = "NodeC";
uint8_t readPipeLocC[] = "Node0";
uint8_t writePipeLocG[] = "NodeG";
uint8_t readPipeLocG[] = "Node0";
// Payload
const int max_payload_size = 32;
char receive_payload[max_payload_size + 1];
// +1 to allow room for a terminating NULL char
// static unsigned int goodSecsMax = 15; // 20
extern RF24 rf24Radio;
void setPipes(uint8_t *writingPipe, uint8_t *readingPipe);
int equalID(char *receive_payload, const char *targetID);


void connectRF24()
{
    rf24Radio.begin();
    // enable dynamic payloads
    rf24Radio.enableDynamicPayloads();
    // optionally, increase the delay between retries & # of retries
    // rf24Radio.setRetries(15, 15);
    rf24Radio.setPALevel(RF24_PA_MAX);
    rf24Radio.setDataRate(RF24_250KBPS);
    rf24Radio.setChannel(124);
    rf24Radio.startListening();
    rf24Radio.printDetails();
    // autoACK enabled by default
    setPipes(writePipeLocC,
             readPipeLocC); // SHOULD NEVER NEED TO CHANGE PIPES
    rf24Radio.startListening();
}


extern ZoneController ZCs[];
#include "WebSerial.h"
extern WebSerial myWebSerial; 

void processZoneRF24Message(void)
{
    char messageText[17];
    while (rf24Radio.available())
    { // Read all available payloads

        // Grab the message and process
        uint8_t len = rf24Radio.getDynamicPayloadSize();

        // If a corrupt dynamic payload is received, it will be flushed
        if (!len)
        {
            return;
        }

        rf24Radio.read(receive_payload, len);

        // Put a zero at the end for easy printing etc
        receive_payload[len] = 0;

        // who was it from?
        // reset that timer

        if (equalID(receive_payload, ZCs[0].heartBeatText))
        {
            ZCs[0].resetZoneDevice();
            //Serial.println("RESET G Watchdog");
            myWebSerial.println("RESET G Watchdog");

            strcpy(messageText, ZCs[0].heartBeatText);
        }
        else if (equalID(receive_payload, ZCs[1].heartBeatText))
        {
            ZCs[1].resetZoneDevice();
            //Serial.println("RESET C Watchdog");
            myWebSerial.println("RESET C Watchdog");

            strcpy(messageText, ZCs[1].heartBeatText);
        }
        else if (equalID(receive_payload, ZCs[2].heartBeatText))
        {
            ZCs[2].resetZoneDevice();
            //Serial.println("RESET S Watchdog");
            myWebSerial.println("RESET S Watchdog");

            strcpy(messageText, ZCs[2].heartBeatText);
        }
        else
        {
            Serial.println("NO MATCH");
            strcpy(messageText, "NO MATCH");
        }
    }
}

void setPipes(uint8_t *writingPipe, uint8_t *readingPipe)
{
    // config rf24Radio to comm with a node
    rf24Radio.stopListening();
    rf24Radio.openWritingPipe(writingPipe);
    rf24Radio.openReadingPipe(1, readingPipe);
}

int equalID(char *receive_payload, const char *targetID)
{
    // check if same 1st 3 chars
    if ((receive_payload[0] == targetID[0]) &&
        (receive_payload[1] == targetID[1]) &&
        (receive_payload[2] == targetID[2]))
    {
        return true;
    }
    else
    {
        return false;
    }
}