#if defined(MILLIS_USE_TIMERA0)
  #error "This sketch takes over TCA0, don't use for millis here. Pin mappings on 8-pin parts are different."
#endif

#define INCREMENT 0x00FF
#define OUTPUT_PIN PIN_PA3

uint16_t period = 0xFFFF;
uint16_t duty_cycle = 0;

bool going_up = true;

void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);

  takeOverTCA0();

  // this needs to be set (even though it's implied to be DEFAULT) to have WO0
  // muxed to PA3. Probably something related to the mega tiny core lib changes.
  PORTMUX.CTRLC = PORTMUX_TCA00_DEFAULT_gc;
  
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc); // set dual slope PWM mode, overflow on bottom
  TCA0.SINGLE.PER = 0xFFFF; // count all the way up to 0xFFFF
  TCA0.SINGLE.CMP0 = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler

  setDutyCycle(0x7FFF);
  setFrequency(1750);
}

void loop() {
  delay(10);

  if (duty_cycle >= (0xFFFF - INCREMENT)) {
    going_up = false;
  } else if (duty_cycle <= INCREMENT) {
    going_up = true;
  }

  if (going_up) {
    duty_cycle += INCREMENT;
  } else {
    duty_cycle -= INCREMENT;
  }

  setDutyCycle(duty_cycle);
}

bool isDualSlope() {
  return TCA0.SINGLE.CTRLB | TCA_SINGLE_WGMODE_DSTOP_gc | TCA_SINGLE_WGMODE_DSBOTH_gc | TCA_SINGLE_WGMODE_DSBOTTOM_gc;
}

void setDutyCycle(uint16_t duty_cycle) {
  TCA0.SINGLE.CMP0 = map(duty_cycle, 0, 65535, 0, period);
}

void setFrequency(unsigned long freqInHz) {
  unsigned long temp_period = (F_CPU / freqInHz);
  temp_period /= isDualSlope() ? 2 : 1;
  
  byte presc = 0;

  while (temp_period > 65536 && presc < 7) {
    presc++;
    temp_period = temp_period >> (presc > 4 ? 2 : 1);
  }

  period = temp_period;

  TCA0.SINGLE.CTRLA = (presc << 1) | TCA_SINGLE_ENABLE_bm;
  TCA0.SINGLE.PER = period;

  // also update duty cycle, because the duty cycle is calculated off the period
  // (so if the user sets frequency after duty cycle, things would get messed up)
  setDutyCycle(TCA0.SINGLE.CMP0);
}
