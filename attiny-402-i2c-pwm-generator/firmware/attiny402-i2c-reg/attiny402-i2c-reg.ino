#include <Wire.h>

// ------------------------------------------------------------------------
// I2C defines
// ------------------------------------------------------------------------
#define NUM_REG    2

// This is the I2C address to listen for commands on. Picked at random
// to one that didn't seem very overloaded. Change this to any
// convenient value if needed.
#define I2C_ADDR   0x3F

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

// ------------------------------------------------------------------------
// Global variables
// ------------------------------------------------------------------------
volatile uint8_t device_registers[NUM_REG] = {0x00, 0x00};
const uint8_t write_mask[NUM_REG] = {0xFF, 0xFF};
volatile uint8_t serial_write_pointer = 0;

void setup() {
  SERIAL_INIT(SERIAL_BAUD_RATE);
  SERIAL_MSG("Serial comm enabled and initialized...");

  Wire.begin(I2C_ADDR);
  Wire.onReceive(onI2CWrite);
  Wire.onRequest(onI2CRead);
  
  SERIAL_MSG("I2C registers initialized...");
}

void loop() {
}

// Handler for I2C writes. When this chip receives a command like this:
//
//   `i2cset -y 1 0x3F 0x00 0x12`
//
// corresponds to a write to addr 0x3F (this chip) with 2 bytes of data.
// This firmware will take the second bit (0x00), use that as a register
// address, and then write the rest of the data sequentially into the
// register file.
void onI2CWrite(int num_bytes) {
  SERIAL_MSG("I2C Write detected (%d bytes)", num_bytes);

  // reset this value to zero in case previous read used it before
  Wire.getBytesRead();

  // get base write address, modulo with number of registers so we
  // don't write off the end of the array
  serial_write_pointer = Wire.read() % NUM_REG;
  --num_bytes;

  SERIAL_MSG("Write pointer: %d", serial_write_pointer);

  // now for the rest of the parameters, write into registers
  while (num_bytes > 0) {
    SERIAL_MSG(
      "Reg %d: 0x%x - write mask: 0x%x",
      serial_write_pointer,
      device_registers[serial_write_pointer],
      write_mask[serial_write_pointer]
    );
    
    uint8_t orig_data = device_registers[serial_write_pointer] & ~write_mask[serial_write_pointer];
    device_registers[serial_write_pointer] = (Wire.read() & write_mask[serial_write_pointer]) | orig_data;
    SERIAL_MSG("Wrote 0x%x to register %d", device_registers[serial_write_pointer], serial_write_pointer);

    ++serial_write_pointer;
    serial_write_pointer = serial_write_pointer % NUM_REG;
    --num_bytes;
  }
}

// Handler for I2C reads. When this chip receives a command like this:
//
//   `ic2get -y 1 0x3F 0x00`
//
// This will cause this chip to read one byte from the current write pointer.
// (I believe that it is the master's responsibility to do a no-data write
// to register address 0x00 to set the write pointer right before it sends
// the read.)
void onI2CRead() {
  SERIAL_MSG("I2C Read detected");

  // get base pointer to register address. NOTE: before every read, i2c write is
  // called first, with length 0 data. This is to set the write pointer. Then
  // when the read is called afterwards, we just need to retrive the write pointer
  // value here.
  serial_write_pointer = (serial_write_pointer + Wire.getBytesRead()) % NUM_REG;

  // read the register value out at pointer address
  uint8_t read_val = device_registers[serial_write_pointer];
  SERIAL_MSG("Read %x from reg %d. Write mask: %x", read_val, serial_write_pointer, write_mask[serial_write_pointer]);
  
  Wire.write(read_val);
}
