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

> NOTE: don't forget to add 4.7 kOhm pull up resistors on the SDA and SCL wires. Use the 5V Arduino signal, same as the VCC input. These pull of resistors are mandated by the I2C spec and you will risk undefined behavior if they are not included.

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

> IMPORTANT: you now also need to add a large (1 MOhm) pull down resistor to ground on pin 7. This is because on power-up, before the PWM output starts, this pin is floating and that will cause the EN input of the AP5726 to assume a random value otherwise. (This is not pictured in the image above.)

### Firmware

There are several sketches written to test various features of the microcontrollers in different stages of development. They can be found in the `firmware` folder.

The final PWM generator firmware is called `attiny402-i2c-pwm-generator`. This one is the "final product" you should flash to the ATTiny if you want the described behavior in the introduction. The rest of the sketches are intermediate tests or copied from tutorials/documentation as incremental attempts to construct the final product.

It is recommended to try `attiny202-updi-led-blink` first, to check if the UPDI and LED wiring is correct. This program simply blinks the LED at 1 Hz.

```
firmware
├───attiny202-eeprom-dump
├───attiny202-eeprom-erase
├───attiny202-eeprom-test-pattern
├───attiny202-serial-test
├───attiny202-tca0-pwm-test
├───attiny202-updi-led-blink
├───attiny402-eeprom-test
├───attiny402-i2c-pwm-generator
└───attiny402-i2c-reg
```
Arduino sketches that start with `attiny202` are meant to be run on the ATTiny202, though they can also fit on the ATTiny402 if you change the microcontroller to the right value. Sketches that start with `attiny402` are meant to run on the ATTiny402, though some sketches that aren't memory intensive may fit on the ATTiny202.

Don't forget to check the `Board` and `Chip` values under the Arduino IDE `Tools` menu when you switch sketches.

#### UART Debug Messages  (attiny202-serial-test)

This is a quick and simple test program to see if the serial UART debug messages are working. It prints "Serial init done." to the console. Flash this to the ATTiny, and make sure the CP2102 is hooked up and the puTTy session is running on the correct COM port and baud rate, and you should see that message.

> NOTE: If you look at this program it will take up around 2KB of flash memory, which is a lot of space for 20 lines of code. This is due to the printf library inclusion (triggered by the `Serial.println` statement), which is very large. This will become a problem for larger sketches, like `attiny402-i2c-pwm-generator`,

#### PWM generation on ATTiny (attiny202-tca0-pwm-test)

This sketch is basically copied verbatim from the PWM example in the megaTinyCore documentation on [TakingOverTCA0](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/TakingOverTCA0.md#example-2-variable-frequency-and-duty-cycle-pwm).

The rest of the document talks about how to program the TCA0 timer and its different execution modes. The PWM code uses the TCA0 timer on the microcontroller to give an extremely accurate, 16 bit resolution PWM signal. You do not need to generate the signal manually, simply configure the TCA0 register. This also handles value changing as well, by flopping the previous value until the cycle finishes and then changing when a new period starts, for greater accuracy.

![PWM signal generation using microcontrollers](/images/attiny-402-i2c-pwm-generator/timer-pwm-generation.jpeg?raw=true)
*How microcontrollers generate PWM signals with perfect timing and full duty cycle range.*

Another interesting page to read is [PWM and Timer usage](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PWMandTimers.md) for more general PWM related information.

A less interesting page to read is the [Microchip TCA0 User Manual](https://ww1.microchip.com/downloads/en/Appnotes/TB3217-Getting-Started-with-TCA-DS90003217.pdf) for ATTiny. This reference doc will fill in some obscure blanks not covered by the megaTinyCore documentation, if you need it for some reason.

There were a couple slight modifications from the example in this sketch. The first is that the `setDutyCycle()` function is called from `setFrequency()` for the users convenience, because once the duty cycle is changed, the frequency will be in an invalid state.

The second is that the TCA0 mode is set to dual slope, overflow on bottom instead of single slope like in the example. This means, when the PWM counter reaches the max value (0xFFFF), it will start counting down instead of wrapping around immediately back to 0x0000. This halves the maximum achievable frequency of the PWM signal, but increases precision. Since we only need to generate a 2kHz signal, this was an acceptable trade-off.

The prescaler calculation in the `setFrequency()` function has been slightly renamed to try and make the algorithm a little more clear. [This article](https://www.linkedin.com/pulse/stm32-pwm-using-timers-timesaving-formula-gurajapu-raja-sumant/) has a slightly better but still mostly inscrutable explanation of how it works.

#### EEPROM (attiny402-eeprom-test)

The EEPROM library is extremely simple, appearing to the programmer as a 128-entry array of bytes you can read and write to. The values in this array are non-volatile, meaning they are preserved across power cycles, so you can store information long term in here.

There are other EEPROM related test programs that might come in handy for debugging:

* `attiny202-eeprom-dump` - This prints the values of all EEPROM cells once on power up
* `attiny202-eeprom-erase` - This "erases" all EEPROM cells. This is equivalent to writing 0xFF to all of them. This is useful for resetting the EEPROM to a known state.
* `attiny202-eeprom-test-pattern` - Writes a predictable pattern to all cells (their index). Useful for debugging other programs which may read/write to cells, so you can use this program to make it easier to figure out which cells are being read.

#### I2C Reading and Writing and Register Modeling (attiny402-i2c-reg)

The [introductory page in the megaTinyCore documentation about I2C](https://github.com/SpenceKonde/megaTinyCore/tree/master/megaavr/libraries/Wire) is pretty illuminating.

> NOTE: here is it called "Wire", as in "Two Wire Interface (TWI)" instead of I2C because I2C is a trademarked term. So that's good to know for later.

This sketch is taken basically verbatim from the example [here](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/libraries/Wire/examples/register_model/register_model.ino).

In this sketch, the I2C register file is only 2 registers large. We store these as a volatile global variable called `device_registers`.

The TWI library requires you to initialize it with `Wire.begin` and supplying an I2C device address. You must supply two callbacks, `onReceive` and `onRequest`, which are called when an I2C write is received and when an I2C read is received, respectively.

##### I2C Write Flow

The write flow is simpler. When a write is received, we get one paramter, `num_bytes`. The first byte is the I2C register address to write. This updates a global `serial_write_pointer` that will be auto incremented (and wrap around back to zero) on every subsequent byte in this write request. The remaining bytes are considered data, written into the auto incremented write pointer address.

The write doesn't necessarily have to behave like this, but it is quite common and expected, so it's better not to break convention.

##### I2C Read Flow

The read flow is more weird. Before every read, a zero byte write is issued. This means the I2C Write Flow is executed before every read. This is to update the write pointer to the read address. The read command re-uses the `serial_write_pointer`. No data is modified, because this write always has length one (zero data bytes).

Once the `serial_write_pointer` is set, we read out **one** byte at the specified device register address and send it back to the I2C host using the `Wire.write()` command. We do implement the ability to write multiple bytes unlike the write command. (You are free to modify it to do so.)

#### I2C PWM Generator (attiny402-i2c-pwm-generator)

This sketch is based off the i2c register file example, but includes changes for generating a PWM signal using TCA0, and writes the values of these registers back to the EEPROM periodically. On power up, it will also restore the values of the I2C registers from EEPROM.

There are 5 registers, with an elaborate control register:

1. `REG_ADDR_CTRL` 0x00
1. `REG_ADDR_PWM_FREQ_LOW` 0x01
1. `REG_ADDR_PWM_FREQ_HIGH` 0x02
1. `REG_ADDR_PWM_DUTY_LOW` 0x03
1. `REG_ADDR_PWM_DUTY_HIGH` 0x04

The frequency and duty cycles are both stored as 16 bit values, requiring you to do 2 I2C writes each to update them.

There is a "writeback timer" that will write the I2C registers back to EEPROM every 10 seconds of inactivity, or every minute regardless of activity.

The 16 bit frequency storage has custom encoding scheme to give you a range from 0 to 16Mhz, with the most resolution at 0 to 16384 Hz, where it matters most. You can find the description of how it works in the [comments](https://github.com/CoryParsnipson/electronics-experiments/blob/main/attiny-402-i2c-pwm-generator/firmware/attiny402-i2c-pwm-generator/attiny402-i2c-pwm-generator.ino#L300-L303).

For the control register, the top most 6 bits are currently unused. The LSB is used to [set the duty cycle resolution (8 or 16 bits)](https://github.com/CoryParsnipson/electronics-experiments/blob/main/attiny-402-i2c-pwm-generator/firmware/attiny402-i2c-pwm-generator/attiny402-i2c-pwm-generator.ino#L53), and bit[1] is used for ["invert mode"](https://github.com/CoryParsnipson/electronics-experiments/blob/main/attiny-402-i2c-pwm-generator/firmware/attiny402-i2c-pwm-generator/attiny402-i2c-pwm-generator.ino#L62), which will make 0 duty cycle full brightness and 100 duty cycle off.

## References/Further Reading

* [Introduction into simple yet powerful tinyAVR 0-series ATtiny microcontrollers](https://daumemo.com/introduction-into-simple-yet-powerful-tinyavr-0-series-attiny-microcontrollers/)
* [Adafruit SMT Breakout PCB for SOIC-8](https://www.adafruit.com/product/1212)
* [Programming the new ATtiny from Arduion using UPDI \[Beginner Tutorial\]](https://www.youtube.com/watch?v=AL9vK_xMt4E)
* [MicroUPDI - Pro-Micro Based UPDI Programmer](https://www.electronics-lab.com/microupdi-pro-micro-based-updi-programmer/)
* [jtag2updi](https://github.com/ElTangas/jtag2updi)
* [megaTinyCore Installation Instructions](https://github.com/SpenceKonde/megaTinyCore/blob/master/Installation.md#boards-manager-installation-now-strongly-recommended-unless-helping-with-core-development)
* [megaTinyCore: TakingOverTCA0](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/TakingOverTCA0.md#example-2-variable-frequency-and-duty-cycle-pwm).
* [megaTinyCore: PWM and Timer usage](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PWMandTimers.md) 
* [Microchip TCA0 User Manual](https://ww1.microchip.com/downloads/en/Appnotes/TB3217-Getting-Started-with-TCA-DS90003217.pdf) 
* [stm32: PWM using Timers - The timesaving formula](https://www.linkedin.com/pulse/stm32-pwm-using-timers-timesaving-formula-gurajapu-raja-sumant/) 
* [megaTinyCore: Wire (TWI/I2C)](https://github.com/SpenceKonde/megaTinyCore/tree/master/megaavr/libraries/Wire) 
