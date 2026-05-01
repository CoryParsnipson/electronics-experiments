#!/bin/bash
#
# backlight-breathe.sh
#
# Use the /sys/class/backlight/<device name> linux utility to alter the
# brightness of an attached LCD screen in a periodic manner.

MAX_DUTY_CYCLE=0xFFFF
MIN_DUTY_CYCLE=0x0000
DEVICE_NAME=tst043wvbi
INTERVAL=0xFF

sigint_detected=false
curr_brightness=0

function set_brightness() {
	device_name=$1
	brightness=$2

	echo $brightness | tee "/sys/class/backlight/$device_name/brightness"
}

function sigint_handler() {
	sigint_detected=true
}

trap sigint_handler INT

while [ true ]; do
	if [ $sigint_detected == "true" ]; then
		echo ""
		echo "SIGINT detected. Writing brightness to 32768 and exiting..."
		set_brightness $DEVICE_NAME 32768
		exit 0
	fi

	if (( curr_brightness > (MAX_DUTY_CYCLE - INTERVAL) )); then
		going_up=false
	elif (( curr_brightness < (MIN_DUTY_CYCLE + INTERVAL) )); then
		going_up=true
	fi

	if [ $going_up == "true" ]; then
		((curr_brightness += INTERVAL))
	else
		((curr_brightness -= INTERVAL))
	fi

	set_brightness $DEVICE_NAME $curr_brightness
done
