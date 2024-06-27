# attiny-402-i2c-pwm-generator

This is a project to use the new ATTiny series microcontroller to make an I2C controlled PWM signal that can be varied by frequency and duty cycle.

The PWM generator will be a sub-component needed for another project, the [ap5726-lcd-backlight-driver](/ap5726-lcd-backlight-driver), that can take a PWM signal <= 2 kHz to dim the backlight output.

## Versioning

Current Arduino IDE Version: 1.8.13

This is not the latest version as of the time of this writing, but according to Spence Konde, the maintainer of the megatinycore library, this is the best version to use for stability and compatibility reasons.

## Description

![ATTiny 402 I2C PWM Generator](/images/attiny-402-i2c-pwm-generator/test-blink.jpg?raw=true)

Unlike Arduino, which is extremely user-friendly, the ATTiny microcontrollers require a bit of setup to use. However, they are inexpensive, need very few additional components, and have a small physical footprint, making them excellent choices for certain applications--such as this one, which needs to be glued to the back of an LCD screen inside an enclosure with very limited space.

The requirements of this project:

* Variable duty cycle between exactly 0% (completely off) to 100% (full brightness)
* Frequency should be slightly less than 2 kHz
* Write duty cycle via I2C
* The duty cycle should be written to non-volatile memory so it can be restored on power-cycle
* The PWM signal should start automatically, with no outside input on power up (we want the LCD screen to turn on during power-up and not have to worry about programming the LCD backlight on start up)

As of the time of this writing, I was able to implement a firmware that could give the above capabilities and more:

* Able to communicate to and from the device using I2C. The I2C device address is programmable.
* Duty cycle and frequency are programmable via I2C.
* Duty cycle has full range of 0 to 100, since it uses the microcontroller timer
* Duty cycle and frequency are written to EEPROM and automatically restored on power up
* There is an additional configuration register that allows you to configure other variables.
  * You can invert the duty cycle (0 = full brightness, 100 = off)
  * You can change the duty cycle resolution between 8 bits and 16 bits. In 8 bit mode, you only need to write the lower register of the duty cycle.
  * There is plenty of extra space for more configuration additions later if necessary
* EEPROM is written in intervals to avoid wearing out the EEPROM prematurely

For some context, prior to this project, considerable research was done into trying to find a dedicated part that will generate a somewhat configurable PWM signal. Many dedicated ICs you can buy are domain specific, with the most common being for power supply switching or modulating colors in RGB LEDs. In the former application, those parts were expensive and overkill and for the latter application, all parts were constrained to low frequencies (<=152 Hz) and could not be configured to remember the duty cycle and automatically restore it after power cycling.

### ATTiny Family Overview

The ATTiny, also known as tinyAVR, family is confusing because AVR released many generations of these devices. Popular *older* generation devices have names like ATTiny85, with the two suffix digits being the old naming scheme.

ATTiny released a new family of devices around 2018, that we will call "the 0-series", since the naming scheme follows a pattern of `ATTiny<x>0<y>`:

![ATTiny 0 series overview chart](/images/attiny-402-i2c-pwm-generator/attiny-0-series-chart.png?raw=true)

The number before the zero indicates how many KB of flash memory is included in the chip and the number after the zero indicates how many pins are on the chip. Very logical!

This family of microcontrollers contains many improvements over the old generation, including the price, unification of features, and UPDI programming interface. Every entry in this family has almost exact feature parity with all other part numbers, making it much simpler to understand.

More information about the 0-series can be found in [this article](https://daumemo.com/introduction-into-simple-yet-powerful-tinyavr-0-series-attiny-microcontrollers/).

#### ATTiny Part Selection for This Project

Since we are concerned with space and price, we start at the smallest, cheapest part: the ATTiny202. This tiny chip has 8 pins and 2 KB of flash memory.

It would turn out, over the course of programming the firmware, that we would need to upgrade to at least the ATTiny402. At 4KB of memory, with everything else being identical, we have just enough memory to be comfortable (though, tbh, more memory would be helpful for debugging).

The megatinycore I2C library that I use itself takes up about 1.7 KB just from the inclusion, ruling out any possibility of staying on the ATTiny202. While the bespoke code of the firmware itself is almost negligible, we also need more space for the EEPROM library, PWM functions, registers, and the printf library...

### How to Program a 0-series ATTiny

The newer ATTiny microcontrollers use the Unified Program and Debug Interface (UPDI) to flash firmware. Fortunately, we are able to use several variants of the Arduino as the programmer. And the UPDI physical connection only requires 1 pin.

The Arduino Nano is a popular choice to use as a programmer, though the Arduino Uno works too and it is what this project uses. The Arduino Pro Micro will also work, but [requires modifications](https://www.electronics-lab.com/microupdi-pro-micro-based-updi-programmer/), so it is recommended to avoid if possible.

#### Mount SMT ATTiny402 Part to a Breakout Board for Breadboard Prototyping

The final product will be surface mount, so a surface mount ATTiny402 was obtained. For breadboarding purposes, this complicates things by requiring an extra step for mounting the SMT part to a breakout board. I used the [SOIC-8 breakout PCB from Adafruit](https://www.adafruit.com/product/1212) for this task.

![SOIC-8 PCB Breakout for SMT parts](/images/attiny-402-i2c-pwm-generator/soic-8-breakout.jpeg?raw=true)

#### Program the Arduino to act as UPDI Programmer

First we need to flash firmware to the Arduino so it can act as a UPDI programmer for the ATTiny. We will use [ElTangas' repo](https://github.com/ElTangas/jtag2updi) and clone this sketch to our local computer and flash the Arduino as normal. We want to use `jtag2updi/source/jtag2updi.ino`.

#### Hardware Pin Assignment for ATTiny UPDI Programming

To set up the programmer, we must connect 5V and GND of the Arduino to the ATTiny, then hook up the D6 pin of the Arduino to the UPDI pin of the ATTiny via a 4.7 kOhm resistor.

Once the Arduino has been programmed with the jtag2updi sketch, we also need to add a 10uF capacitor across the ground and reset pins so that it will not go low and cause the Arduino itself to be reprogrammed during flashing.

See this diagram (substitute Arduino Uno here, the UPDI pin is also D6 on that):

![Minimal UPDI set up with Arduino Nano](/images/attiny-402-i2c-pwm-generator/attiny-0-arduino-updi-programming-wiring.png?raw=true)

Since we are using the ATTIny402, the UPDI pin is 6, as we can see from this pinout chart:

![ATTiny x02 pinout](/images/attiny-402-i2c-pwm-generator/attiny-x02-pinout.jpg?raw=true)

This 8 minute youtube video by bitluni is quite helpful, if more direction is needed:

[![How to Program ATTiny using Arduino and UPDI Tutorial](https://img.youtube.com/vi/AL9vK_xMt4E/0.jpg)](https://www.youtube.com/watch?v=AL9vK_xMt4E)

#### Using the megatinycore Arduino Library for ATTiny Development

The last part of the puzzle is to set up the megatinycore library for ATTiny development. This is a collection of C libraries that really help with microcontroller abstractions for the ATTiny 0-series.

1. Install Arduino IDE, preferably version 1.8.13.
1. Follow the [instructions here](https://github.com/SpenceKonde/megaTinyCore/blob/master/Installation.md#boards-manager-installation-now-strongly-recommended-unless-helping-with-core-development) to install the megaTinyCore board libraries.

Once that is installed, go to `Tools` and make sure all the settings look like below:

![megaTinyCore board settings](/images/attiny-402-i2c-pwm-generator/megatinycore-board-settings.png?raw=true)

Some important points:

* `millis()/micros() Timer` set to TCB0 - we are using TCA0 for PWM generation
* Startup time can be anything, not just 8ms in the picture
* `printf()` - set this to minimal *if it is used in the program*, else set to default or it will use more flash
* `Programmer` - set this to `jtag2updi`

### Hardware Setup

#### Single LED

![Single LED Arduino UPDI test set up](/images/attiny-402-i2c-pwm-generator/test-setup-single-led.jpg?raw=true)
*Image of Arduino Uno being used as a UPDI programmer for the ATTiny402 (SMT soldered to a [breakout board from Adafruit](https://www.adafruit.com/product/1212))*

We are using an ATTiny402 on a breadboard, similar to the set up described in the previous section hooked up to a single discrete LED and I2C signals going to the corresponding pins of a Raspberry Pi CM3 on a development board, which is acting as the I2C host. The I2C pins are physical pin 3 for SDA and physical pin 5 for SCL.

A development computer (a Windows 10 laptop) is used to VNC into the Raspberry Pi and sent/receive I2C commands to the ATTiny via the bundled Raspberry Pi Linux utilities, `i2c-set` and `i2c-get`.

Lastly, a [CP2102 USB TTL module](https://www.aliexpress.com/w/wholesale-cp2102-usb-to-ttl-converter.html) was used to allow for serial debug through the UART interface. 

![CP2102 USB TTL module for Serial Debug](/images/attiny-402-i2c-pwm-generator/cp2102-module.webp?raw=true)

Since, we are already using the Arduino IDE for programming the Arduino Uno, we need to install puTTy and set the COM port appropriately (need to look at control panel) and use the baud rate of 115200, matching that of the firmware sketches in this repo, to view serial debug messages.

##### Pin Assignments

From the ATTiny:

![ATTiny x02 pinout](/images/attiny-402-i2c-pwm-generator/attiny-x02-pinout.jpg?raw=true)

* pin 1 -> 5V from the Arduino
* pin 2 -> to the **RX** pin of the CP2102
* pin 3 -> to the **TX** pin of the CP2102
* pin 4 -> to the SDA pin of the Raspberry Pi
* pin 5 -> to the SCL pin fo the Raspberry Pi
* pin 6 -> to pin D6 of the Arduino via 4.7 kOhm resistor as part of UPDI setup
* pin 7 -> to the positive lead of the LED, via 1k ohm resistor (the other lead to ground)
* pin 8 -> to GND from the Arduino

#### AP5726 with LED Strips

![Full setup with AP5726 and LED Strips](/images/attiny-402-i2c-pwm-generator/test-setup-ap5726.jpg?raw=true)

This setup is identical in pin assignments to the single LED setup, except for pin 7, which now goes directly into the EN pin of the AP5726. See [that project](/ap5726-lcd-backlight-driver) for more information.

The I2C signals going to the Raspberry Pi are highlighted with light green arrows. (Ignore the extra circuit elements on the upper breadboard, they are unrelated to this project).

IMPORTANT: you now also need to add a large (1 MOhm) pull down resistor to ground on pin 7. This is because on power-up, before the PWM output starts, this pin is floating and that will cause the EN input of the AP5726 to assume a random value otherwise. (This is not pictured in the image above.)

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

The final PWM generator firmware is called `attiny402-i2c-pwm-generator`. The rest of the sketches are intermediate tests or copied from tutorials/documentation as incremental attempts to construct the final product.

#### megatinycore Settings

Go over programmer settings

TBD

#### UART Debug Messages  (attiny202-serial-test)

Arduino and Attiny don't have a debugger you can step through (or at least, not to my knowledge), so debug log messages were invaluable. Unfortunately, this requires a printf library and that takes up a lot of space.

Even on the 402, you will only be able to enable one or two log messages at a time, so debug wisely...

TBD

#### PWM generation on ATTiny (attiny202-tca0-pwm-test)

The PWM code uses the TCA0 timer on the microcontroller to give an extremely accurate, 16 bit resolution PWM signal. You do not need to generate the signal manually, simply configure the TCA0 register. This also handles value changing as well, by flopping the previous value until the cycle finishes and then changing when a new period starts, for greater accuracy.

TBD

#### EEPROM (attiny402-eeprom-test)

The EEPROM is really simple. From the programmer's perspective, it looks like a 128 entry array of bytes.

TBD

#### I2C Reading and Writing and Register Modeling (attiny402-i2c-reg)

This is taken almost wholesale from the megatinycore documentation about how to write a register file I2C device. This is pretty much exactly what we want for the firmware, so the final product starts here and makes a bunch of modifications.

Describe read process.

Describe write process.

TBD

### I2C PWM Firwmare Documentation

Include usage documentation, register configuration, and interesting facts about the implementation (periodic EEPROM writeback, register file config, local variable update, etc)

TBD

## References/Further Reading

* [Introduction into simple yet powerful tinyAVR 0-series ATtiny microcontrollers](https://daumemo.com/introduction-into-simple-yet-powerful-tinyavr-0-series-attiny-microcontrollers/)
* [Adafruit SMT Breakout PCB for SOIC-8](https://www.adafruit.com/product/1212)
* [Programming the new ATtiny from Arduion using UPDI \[Beginner Tutorial\]](https://www.youtube.com/watch?v=AL9vK_xMt4E)
* [MicroUPDI - Pro-Micro Based UPDI Programmer](https://www.electronics-lab.com/microupdi-pro-micro-based-updi-programmer/)
* [jtag2updi](https://github.com/ElTangas/jtag2updi)
* [megaTinyCore Installation Instructions](https://github.com/SpenceKonde/megaTinyCore/blob/master/Installation.md#boards-manager-installation-now-strongly-recommended-unless-helping-with-core-development)
