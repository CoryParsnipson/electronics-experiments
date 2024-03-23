#include <EEPROM.h>
#include <Wire.h>

// ------------------------------------------------------------------------
// System defines
// ------------------------------------------------------------------------
#if defined(MILLIS_USE_TIMERA0)
  #error "This sketch takes over TCA0, don't use for millis here. Pin mappings on 8-pin parts are different."
#endif

// ------------------------------------------------------------------------
// PWM defines
// ------------------------------------------------------------------------
#define PWM_OUTPUT_PIN      PIN_PA3
#define PWM_MAX_PERIOD      0xFFFF
#define PWM_DEFAULT_FREQ    1750

// ------------------------------------------------------------------------
// I2C defines
// ------------------------------------------------------------------------
#define NUM_REG    5

// This is the I2C address to listen for commands on. Picked at random
// to one that didn't seem very overloaded. Change this to any
// convenient value if needed.
#define I2C_ADDR   0x3F

// I2C register indexes (for both system regs and eeprom)
#define REG_ADDR_CTRL          0
#define REG_ADDR_PWM_FREQ_LOW  1
#define REG_ADDR_PWM_FREQ_HIGH 2
#define REG_ADDR_PWM_DUTY_LOW  3
#define REG_ADDR_PWM_DUTY_HIGH 4

// REG_CTRL_DUTY_CYCLE_RESOLUTION
// If the value at this bit is:
//   0 = 16 bit res (0 to 65535)
//   1 = 8 bit (0 to 255)
#define REG_CTRL_DUTY_CYCLE_RESOLUTION 0

// REG_CTRL_INVERT_MODE
// This will invert the phase of the PWM. If this is set to 0, the
// PWM signal will be high if the counter is < the CMP value. If set
// to 1, the opposite will be true.
#define REG_CTRL_INVERT_MODE 1

// ------------------------------------------------------------------------
// Serial defines
// ------------------------------------------------------------------------
#define SERIAL_ENABLED 0
#define SERIAL_MSG_BUFFER_SIZE 40
#define SERIAL_BAUD_RATE  115200

#if SERIAL_ENABLED
  char serial_msg_buffer[SERIAL_MSG_BUFFER_SIZE];

  #define SERIAL_MSG(...) \
    do { \
      snprintf(serial_msg_buffer, SERIAL_MSG_BUFFER_SIZE, __VA_ARGS__); \
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
volatile uint8_t device_registers[NUM_REG] = {0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t write_mask[NUM_REG] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
volatile uint8_t serial_write_pointer = 0;
uint16_t period = 0xFFFF;

void setup() {
  SERIAL_INIT(SERIAL_BAUD_RATE);

  Wire.begin(I2C_ADDR);
  Wire.onReceive(onI2CWrite);
  Wire.onRequest(onI2CRead);
  
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  takeOverTCA0();
  
  // this needs to be set (even though it's implied to be DEFAULT) to have WO0
  // muxed to PA3. Probably something related to the mega tiny core lib changes.
  PORTMUX.CTRLC = PORTMUX_TCA00_DEFAULT_gc;

  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc); // set dual slope PWM mode, overflow on bottom
  TCA0.SINGLE.PER = PWM_MAX_PERIOD; // count all the way up to 0xFFFF
  TCA0.SINGLE.CMP0 = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler

  // restore configuration from non-volatile memory
  readEEPROM((uint8_t*)device_registers, NUM_REG);
  updateLocalVariables((uint8_t*)device_registers);

  SERIAL_MSG(
    "[PWM] init (f: %ld, d: 0x%X)",
    getFrequency((uint8_t*)device_registers),
    getDutyCycle((uint8_t*)device_registers)
  );
}

void loop() {}

bool isDualSlope() {
  return TCA0.SINGLE.CTRLB | TCA_SINGLE_WGMODE_DSTOP_gc | TCA_SINGLE_WGMODE_DSBOTH_gc | TCA_SINGLE_WGMODE_DSBOTTOM_gc;
}

void setDutyCycle(uint16_t duty_cycle, uint16_t period) {
  TCA0.SINGLE.CMP0 = map(duty_cycle, 0, PWM_MAX_PERIOD, 0, period);
}

void setFrequency(unsigned long freqInHz, uint16_t* period) {
  unsigned long temp_period = (F_CPU / freqInHz);
  temp_period /= isDualSlope() ? 2 : 1;
  
  byte presc = 0;
  while (temp_period > PWM_MAX_PERIOD && presc < 7) {
    presc++;
    temp_period = temp_period >> (presc > 4 ? 2 : 1);
  }
  *period = temp_period;

  TCA0.SINGLE.CTRLA = (presc << 1) | TCA_SINGLE_ENABLE_bm;
  TCA0.SINGLE.PER = *period;

  // also update duty cycle, because the duty cycle is calculated off the period
  // (so if the user sets frequency after duty cycle, things would get messed up)
  setDutyCycle(TCA0.SINGLE.CMP0, *period);
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
  // onI2CWrite is called before every I2C read. If this is true, was_ctrl_write
  // is set to true
  bool was_ctrl_write = true;

  // reset this value to zero in case previous read used it before
  Wire.getBytesRead();

  // get base write address, modulo with number of registers so we
  // don't write off the end of the array
  serial_write_pointer = (uint8_t)Wire.read();
  --num_bytes;

  if (num_bytes > 0) {
    was_ctrl_write = false;
  }

  // now for the rest of the parameters, write into registers
  while (num_bytes > 0) {
    uint8_t orig_data = device_registers[serial_write_pointer] & ~write_mask[serial_write_pointer];
    device_registers[serial_write_pointer] = (Wire.read() & write_mask[serial_write_pointer]) | orig_data;

    SERIAL_MSG(
      "[I2C] Wrote reg[%d] = 0x%X (mask: 0x%X)",
      serial_write_pointer,
      device_registers[serial_write_pointer],
      write_mask[serial_write_pointer]
    );

    ++serial_write_pointer;
    serial_write_pointer = serial_write_pointer;
    --num_bytes;

    EEPROM.update(serial_write_pointer, device_registers[serial_write_pointer]);  
  }

  if (!was_ctrl_write) {
    updateLocalVariables((uint8_t*)device_registers);  
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
  // get base pointer to register address. NOTE: before every read, i2c write is
  // called first, with length 0 data. This is to set the write pointer. Then
  // when the read is called afterwards, we just need to retrive the write pointer
  // value here.
  serial_write_pointer = (serial_write_pointer + Wire.getBytesRead());

  // read the register value out at pointer address
  uint8_t read_val = device_registers[serial_write_pointer];
  SERIAL_MSG("[I2C] Read reg[%d] = 0x%X (mask: 0x%X)", serial_write_pointer, read_val, write_mask[serial_write_pointer]);
  
  Wire.write(read_val);
}

void readEEPROM(uint8_t* registers, uint8_t num_regs) {
  for (uint8_t addr = 0; addr < num_regs; ++addr) {
    registers[addr] = EEPROM[addr];
  }
}

uint16_t getDutyCycle(uint8_t* registers) {
  uint16_t duty_cycle = registers[REG_ADDR_PWM_DUTY_HIGH];
  duty_cycle = (duty_cycle << 8) | registers[REG_ADDR_PWM_DUTY_LOW];
  return duty_cycle;
}

uint32_t getFrequency(uint8_t* registers) {
  // frequency calculation works like this:
  // lowest 14 bits are the base number, giving you a range of 0 to 16,384.
  // The highest 2 bits are used as a x10 multiplier (so x1, x10, x100, x1000)
  // letting you reach up to 16 MHz.
  uint32_t freq = (registers[REG_ADDR_PWM_FREQ_HIGH] & 0x3F);
  freq = (freq << 8) | registers[REG_ADDR_PWM_FREQ_LOW];

  int freq_multi = registers[REG_ADDR_PWM_FREQ_HIGH] >> 6;
  for (int i = 0; i < freq_multi; ++i) {
    freq *= 10;
  }
  
  if (freq == 0) {
    SERIAL_MSG("[I2C][WARNING] 0 freq detected. Using default.");
    freq = PWM_DEFAULT_FREQ;
  }
  
  return freq;  
}

void updateLocalVariables(uint8_t* registers) {
  setFrequency(getFrequency(registers), &period);
  setDutyCycle(getDutyCycle(registers), period);

  SERIAL_MSG(
    "[PWM] set (f: %ld, d: 0x%X)",
    getFrequency((uint8_t*)device_registers),
    getDutyCycle((uint8_t*)device_registers)
  );
}
