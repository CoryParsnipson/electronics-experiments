# attiny-402-i2c-pwm-generator

This is a project to use the new ATTiny series microcontroller to make an I2C controlled PWM signal that can be varied by frequency and duty cycle.

The PWM generator will be a sub-component needed for another project, the [ap5726-lcd-backlight-driver](/ap5726-lcd-backlight-driver), that can take a PWM signal <= 2 kHz to dim the backlight output.

The requirements of this project are:

* Duty cycle must be able to be varied between exactly 0% (completely off) to 100% (full brightness) via I2C
* Frequency should be slightly less than 2 kHz, but ideally larger than 1000 Hz to reduce signal noise
* The duty cycle should be written to non-volatile memory so it can be restored on power-cycle
* The PWM signal should start automatically, with no outside input on power up (we want the LCD screen to turn on during power-up and not have to worry about programming the LCD backlight on start up)

As of the time of this writing, I was able to implement a firmware that could give the above capabilities and more:

* Able to communicate to and from the device using I2C. The I2C device address is programmable.
* Duty cycle and frequency are both able to be programmed via I2C.
* Duty cycle has full range of 0 to 100, since it uses the microcontroller timer
* Duty cycle and frequency are written to EEPROM and automatically restored on power up
* There is an additional configuration register that allows you to configure other variables.
  * You can invert the duty cycle (0 = full brightness, 100 = off)
  * You can change the duty cycle resolution between 8 bits and 16 bits. In 8 bit mode, you only need to write the lower register of the duty cycle.
  * There is plenty of extra space for more configuration additions later if necessary
* EEPROM is written in intervals to avoid wearing out the EEPROM prematurely

For some context, prior to this project, considerable research was done into trying to find a dedicated part that will generate a somewhat configurable PWM signal. Many dedicated ICs you can buy are domain specific, with the most common being for power supply switching or modulating colors in RGB LEDs. In the former application, those parts were expensive and overkill and for the latter application, all parts were constrained to low frequencies (<=152 Hz) and could not be configured to remember the duty cycle and automatically restore it after power cycling.

## Arduino IDE Version

Current version: 1.8.13

This is not the latest version as of the time of this writing, but according to Spence Konde, the maintaining of the megatinycore library, this is the best version to use for stability and compatibility reasons.

## Description

![ATTiny 402 I2C PWM Generator](/images/attiny-402-i2c-pwm-generator/test-blink.jpg?raw=true)

Unlike Arduino, which is extremely user-friendly, the ATTiny microcontrollers require a bit of setup to use. However, they are inexpensive, need very few additional components, and have a small physical footprint, making them excellent choices for certain applications--such as this one, which needs to be glued to the back of an LCD screen inside an enclosure with very limited space.

### ATTiny Family Overview

The ATTiny, also known as tinyAVR, family is confusing because AVR released many generations of these devices. Popular *older* generation devices have names like ATTiny85, with the two suffix digits being the old naming scheme.

ATTiny released a new family of devices around 2018, that we will call "the 0-series", since the naming scheme follows a pattern of `ATTiny<x>0<y>`:

[ATTiny 0 series overview chart](/images/attiny-402-i2c-pwm-generator/attiny-0-series-chart.png?raw=true)

The number before the zero indicates how many KB of flash memory is included in the chip and the number after the zero indicates how many pins are on the chip. Very logical!

This family of microcontrollers contains many improvements over the old generation, including the price, unification of features, and UPDI programming interface. Every entry in this family has almost exact feature parity with all other part numbers, making it much simpler to understand.

More information about the 0-series can be found in [this article](https://daumemo.com/introduction-into-simple-yet-powerful-tinyavr-0-series-attiny-microcontrollers/).

#### ATTiny Part Selection for This Project

Since we are concerned with space and price, we start at the smallest, cheapest part: the ATTiny202. This tiny chip has 8 pins and 2 KB of flash memory.

It would turn out, over the course of programming the firmware, that we would need to upgrade to at least the ATTiny402. At 4KB of memory, with everything else being identical, we have just enough memory to be comfortable (though, tbh, more memory would be helpful for debugging).

The megatinycore I2C library that I use itself takes up about 1.7 KB just from the inclusion, ruling out any possibility of staying on the ATTiny202. While the bespoke code of the firmware itself is almost negligible, we also need more space for the EEPROM library, PWM functions, registers, and the printf library...

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

* [Introduction into simple yet powerful tinyAVR 0-series ATtiny microcontrollers](https://daumemo.com/introduction-into-simple-yet-powerful-tinyavr-0-series-attiny-microcontrollers/)
