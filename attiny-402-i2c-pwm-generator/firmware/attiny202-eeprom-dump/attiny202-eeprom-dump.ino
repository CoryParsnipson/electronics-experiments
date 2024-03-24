#include <EEPROM.h>

// NOTE: for this sketch, it is not recommended to use the "minimal"
// printf library because it does not support zero padding.

// ------------------------------------------------------------------------
// Serial defines
// ------------------------------------------------------------------------
#define SERIAL_ENABLED 1
#define SERIAL_BAUD_RATE  115200

#if SERIAL_ENABLED
  char serial_msg_buffer[100];

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
  print_eeprom();
}

void loop() {
}

void print_eeprom() {
  #ifdef SERIAL_ENABLED
    int row_size = 16;
    char* pos = serial_msg_buffer;

    pos += sprintf(pos, "     "); 
    for (int h = 0; h < row_size; ++h) {
      pos += sprintf(pos, "% 4d ", h);
    }
    Serial.println(serial_msg_buffer);
    pos = serial_msg_buffer;

    for (int r = 0; r < EEPROM.length(); r += row_size) {
      pos += sprintf(pos, "%04d: ", r);
      
      for (int c = r; c < r + row_size; ++c) {
        pos += sprintf(pos, "0x%02X ", (uint8_t)EEPROM.read(c));
      }

      Serial.println(serial_msg_buffer);
      pos = serial_msg_buffer;
    }
  #endif
}
