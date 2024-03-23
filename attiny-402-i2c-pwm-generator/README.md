# attiny-402-i2c-pwm-generator

This is a project to use the new ATTiny series microcontroller to make an I2C controlled PWM signal that can be varied by frequency and duty cycle.

## Arduino IDE Version

Current version: 1.8.13

This is not the latest version as of the time of this writing, but according to Spence Konde, the maintaining of the megatinycore library, this is the best version to use for stability and compatibility reasons.

## Description

![ATTiny 402 I2C PWM Generator](/images/attiny-402-i2c-pwm-generator/test-blink.jpg?raw=true)

Unlike Arduino, which is extremely user-friendly, the ATTiny microcontrollers require a bit of setup to use. However, they are inexpensive, self-contained, and have a small physical footprint, making them excellent choices for certain applications (such as this one, which needs to be glued to the back of an LCD screen inside an enclosure with very limited space).

### ATTiny Family Overview

TBD

### How to Program a new series ATTiny

TBD

### Dev Setup

For this we are using a breakout board from Adafruit and a breadboard to lay down a temporary circuit for testing. The UPDI programming can be done using an Arduino Uno and hooking up a wire to Pin 6 through the breadboard, but when the ATTiny is surface mounted directly to a custom PCB design, we will need to figure out a different way to program it.

TBD

### Firmware

There are several firmware written to test various features of the microcontrollers in different stages of development. They can be found in the `firmware` folder.

Arduino sketches that start with `attiny202` are meant to be run on the ATTiny202, though they can also fit on the ATTiny402 if you change the microcontroller to the right value. Sketches that start with `attiny402` are meant to run on the ATTiny402, though some sketches that aren't memory intensive could run on the ATTiny202.

```
firmware
├───attiny202-serial-test
├───attiny202-tca0-pwm-test
├───attiny202-updi-led-blink
├───attiny402-eeprom-test
├───attiny402-i2c-pwm-generator
└───attiny402-i2c-reg
```

## References/Further Reading

TBD
