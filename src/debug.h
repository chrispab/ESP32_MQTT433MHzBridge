// Uncomment to enable debug output
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print("~");Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.print("~");Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
