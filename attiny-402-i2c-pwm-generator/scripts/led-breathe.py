#!/bin/python
#
# led-breathe.py
#
# Sets the pwm duty cycle in a periodic function to make the
# led brightness "breathe" (slowly turn on and off).

from functools import partial
from math import floor
import signal
from smbus import SMBus
import sys
import time

DEVICE_ADDR=0x3F

CTRL_REG_ADDR=0x00
PWM_FREQ_LOW_ADDR=0x01
PWM_FREQ_HIGH_ADDR=0x02
PWM_DUTY_LOW_ADDR=0x03
PWM_DUTY_HIGH_ADDR=0x04


def i2c_read(i2cbus, device_addr, addr, word = False):
	if word:
		word_data = 0x0000
		word_data |= i2cbus.read_byte_data(device_addr, addr+1)
		word_data = word_data << 8
		word_data |= (i2cbus.read_byte_data(device_addr, addr) & 0xFF)
		return word_data
	return i2cbus.read_byte_data(device_addr, addr)


def i2c_write(i2cbus, device_addr, addr, data, word = False):
	if word:
		i2cbus.write_word_data(device_addr, addr, data)
	i2cbus.write_byte_data(device_addr, addr, data)


def print_regs(i2cbus):
	print("CTRL[0x00]: 0x%X" % i2c_read(i2cbus, DEVICE_ADDR, CTRL_REG_ADDR))
	print("FREQ[0x02..0x01]: 0x%X" % i2c_read(i2cbus, DEVICE_ADDR, PWM_FREQ_LOW_ADDR, True))
	print("DUTY[0x03..0x04]: 0x%X" % i2c_read(i2cbus, DEVICE_ADDR, PWM_DUTY_LOW_ADDR, True))


def sigint_handler(sig, frame, i2cbus):
	print("SIGINT detected! Writing duty cycle to 0x0000 and exiting...")
	i2c_write(i2cbus, DEVICE_ADDR, PWM_DUTY_LOW_ADDR, 0x0000, True)
	time.sleep(0.1)
	sys.exit(0)


def scale_inverse_log(x, min_bound, max_bound):
	scaled_x = x / max_bound
	unscaled_val = 1.012 ** (500 * (scaled_x - 1))
	return max_bound * unscaled_val


if __name__ == "__main__":
	print("Running led-breathe.py...")
	i2cbus = SMBus(1) # Create a new I2C bus
	print_regs(i2cbus)

	# handle sigint so we don't shutdown in the middle of EEPROM write
	bound_sigint_handler = partial(sigint_handler, i2cbus=i2cbus)
	signal.signal(signal.SIGINT, bound_sigint_handler)

	going_up = True
	duty_cycle = 0
	interval = 0xFF
	MAX_DUTY_CYCLE = 0xFFFF
	MIN_DUTY_CYCLE = 0

	while True:
		if duty_cycle > (MAX_DUTY_CYCLE - interval):
			going_up = False
		elif duty_cycle < (MIN_DUTY_CYCLE + interval):
			going_up = True
		
		if going_up:
			duty_cycle += interval
		else:
			duty_cycle -= interval

		scaled = scale_inverse_log(duty_cycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE)
		scaled = floor(scaled)

		i2c_write(i2cbus, DEVICE_ADDR, PWM_DUTY_LOW_ADDR, scaled, True)
		time.sleep(0.05)
