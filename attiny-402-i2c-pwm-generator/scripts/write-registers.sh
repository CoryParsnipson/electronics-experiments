#!/bin/bash
#
# write-registers.sh
#
# Will print the current register contents of the pwm generator, then
# overwrite them to the specified values.

chip_addr=1
device_addr=0x3F

CTRL_reg_addr=0x00
PWM_freq_low_addr=0x01
PWM_freq_high_addr=0x02
PWM_duty_low_addr=0x03
PWM_duty_high_addr=0x04

CTRL_invert_mode=0 # invert mode config (0 -> 0x0000 is off 0xFFFF is max)
CTRL_8bit_mode=0   # use only LOW reg duty cycle if 1, else use both regs (16 bit)

PWM_frequency=1750 # range [0..16380000]
PWM_duty_cycle=0x7FFF # range [0..65535] or [0..255] if CTRL_8bit_mode == 1

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

print_regs

echo "Writing I2C registers..."

((CTRL_reg_value=(0x00 | (CTRL_invert_mode << 0) | (CTRL_8bit_mode << 1))))

echo "Writing config (invert mode = ${CTRL_invert_mode}, 8bit mode = ${CTRL_8bit_mode})..."
i2c_write ${CTRL_reg_addr} ${CTRL_reg_value}

echo "Writing PWM frequency = ${PWM_frequency}..."
i2c_write ${PWM_freq_low_addr} ${PWM_frequency} w

echo "Writing PWM duty cycle = ${PWM_duty_cycle}..."
i2c_write ${PWM_duty_low_addr} ${PWM_duty_cycle} w

print_regs
