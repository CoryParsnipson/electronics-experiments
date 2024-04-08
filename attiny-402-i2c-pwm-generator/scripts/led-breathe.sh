#!/bin/bash
#
# led-breathe.sh
#
# Sets the pwm duty cycle in a periodic function to make the
# led brightness "breathe" (slowly turn on and off).

MAX_DUTY_CYCLE=0xFFFF
MIN_DUTY_CYCLE=0x0000

chip_addr=1
device_addr=0x3F

CTRL_reg_addr=0x00
PWM_freq_low_addr=0x01
PWM_freq_high_addr=0x02
PWM_duty_low_addr=0x03
PWM_duty_high_addr=0x04

going_up=true
sigint_detected=false
interval=0xFF   # step size to traverse duty cycle range
duty_cycle=0    # range of [0x0000 to 0xFFFF]

function i2c_read() {
  echo "$(i2cget -y ${chip_addr} ${device_addr} $1)"
}

function i2c_write() {
  i2cset -y ${chip_addr} ${device_addr} $1 $2 $3
  sleep 0.01 # wait for write to finish
}

function read_word() {
  reg_addr=$1
  ((reg_addr += 1))

  read_data_high=$(i2c_read ${reg_addr})
  read_data_high=${read_data_high#"0x"}

  ((reg_addr -= 1))

  read_data_low=$(i2c_read ${reg_addr})
  read_data_low=${read_data_low#"0x"}

  echo "0x${read_data_high}${read_data_low}"
}

function print_regs() {
  echo ""
  echo "CTRL[0x00]: $(i2c_read ${CTRL_reg_addr})"
  echo "FREQ[0x02..0x01]: $(read_word ${PWM_freq_low_addr})"
  echo "DUTY[0x04..0x03]: $(read_word ${PWM_duty_low_addr})"
  echo ""
}

i2cdetect -y ${chip_addr}
sleep 0.1
print_regs

echo "LED breath starting. Interval = ${interval}"

function sigint_handler() {
  sigint_detected=true
}

trap sigint_handler INT

while [ true ]; do
  if [ $sigint_detected == "true" ]; then
    echo ""
    echo "SIGINT detected. Writing duty cycle to 0x0000 and exiting..."
    i2c_write ${PWM_duty_low_addr} 0x0000 w
    exit 0
  fi

  if (( duty_cycle > (MAX_DUTY_CYCLE - interval) )); then
    going_up=false
  elif (( duty_cycle < (MIN_DUTY_CYCLE + interval) )); then
    going_up=true
  fi

  if [ $going_up == "true" ]; then
    ((duty_cycle += interval))
  else
    ((duty_cycle -= interval))
  fi

  i2c_write ${PWM_duty_low_addr} ${duty_cycle} w
done
