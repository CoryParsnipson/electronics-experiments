#include <EEPROM.h>

// ------------------------------------------------------------------------
// Serial defines
// ------------------------------------------------------------------------
#define SERIAL_ENABLED 1
#define SERIAL_BAUD_RATE  115200

#if SERIAL_ENABLED
  char serial_msg_buffer[40];

  #define SERIAL_MSG(...) \
    do { \
      sprintf(serial_msg_buffer, __VA_ARGS__); \
      Serial.println(serial_msg_buffer); \
    } while (false);
    
  #define SERIAL_INIT(x) \
    Serial.begin(x); \
    while (!Serial) {} // wait for Serial to initialize
#else
  #define SERIAL_MSG(...)
  #define SERIAL_INIT(x)
#endif


void setup() {
  SERIAL_INIT(SERIAL_BAUD_RATE);
  SERIAL_MSG("Serial comm enabled and initialized...");

  SERIAL_MSG("EEPROM length: %d", EEPROM.length());

  // add delay for demonstration purposes only
  delay(10);

  SERIAL_MSG("EEPROM addr 0: 0x%x", (uint8_t)EEPROM[0]);
  EEPROM[0] = 0x23; // some random value
  
  SERIAL_MSG("EEPROM addr 0: 0x%x", (uint8_t)EEPROM[0]);
}

void loop() {
}
