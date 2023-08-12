# 555-timer-pwm-with-digipot

This is a hardware pwm signal generator using a 555 timer. The design used here is one that uses two diodes attached to both ends of a potentiometer to make the duty cycle adjustable without changing the frequency. A twist on the usual design is that this project also uses a digital potentiometer (a.k.a. digipot) so that the duty cycle can be adjusted programmatically with a microcontroller via an I2C interface.

The purpose of this project is to provide digital access to an adjustable PWM signal to dim an lcd backlight driver. (See [ap5726-lcd-backlight-driver](../ap5726-lcd-backlight-driver).) The output signal of this project can be fed into the enable signal of an lcd backlight driver.

## Disclaimer

**This project is not recommended for real usage. This is not a very good way to generate a PWM signal compared to other methods.** See [Post Mortem](#post-mortem) for more detailed information.

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 7.0.2.

## Description

![50% Duty Cycle Output Signal](/images/555-timer-pwm-with-digipot/50p-duty-cycle.jpg?raw=true)

![555 Timer Real PCB](/images/555-timer-pwm-with-digipot/555-timer-real-pcb.jpg?raw=true)

### PWM Generation using 555-timer in Astable Multivibrator Mode


A good explanation of how 555-timers work and how they operate in astable-multivibrator mode can be found at [How to Generate PWM Signal using 555 Timer IC](https://www.electronicshub.org/555-timer-pwm/), also listed as [1] in References. The author does a good job of explaining the functional logic and how it works. Basically, a 555-timer can be used to emit signal pulses at very specific, user-settable delays. This is known as "monostable" mode. A 555-timer can also be induced to oscillate like a clock signal by using feedback loops. This mode of operation is known as "astable" because it changes value by itself and "multi-vibrator" because it assumes different values.

![Frequency Independent 555-oscillator using a potentiometer](/images/555-timer-pwm-with-digipot/freq-independent-555.gif?raw=true)

Unfortunately, the author of [1] doesn't explain the PWM oscillator circuit in detail and really drops the ball at the end. I found a second page, [555 Circuits Part 1](https://www.electronics-tutorials.ws/waveforms/555-circuits-part-1.html#:~:text=Fully%20Independent%20555%20Oscillator&text=The%20timing%20period%20for%20both,without%20affecting%20the%20output%20frequency.), ([2] under References) that describes creating a PWM generator step-by-step starting from 555-timer astable mode. I am basing this project's circuit off of the "Fully Independent 555 Oscillator". [2] goes into much more detail about the circuit design. These two webpages together, I feel, are a very good primer into 555 circuits.

### Schematic for Project Design

![555 Timer with Digipot Schematic in KiCAD](/images/555-timer-pwm-with-digipot/schematic.png?raw=true)

The schematic was created by following the block diagram for the frequency independent 555-oscillator shown above. The [DS3502](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3502.pdf) digipot is incorporated using the layout copied from the [Adafruit breakout board](https://www.adafruit.com/product/4286). The SDA and SCL signals require 10k pull up resistors. The A0 and A1 pins are hooked up to solder jumper pads for configurability. To change the I2C address, bridge the solder pins. The digipot also needs decoupling capacitors close to the V+ and Vin pins.

### PCB Layout Notes

![555 Timer with Digipot PCB layout in KiCAD](/images/555-timer-pwm-with-digipot/pcb-layout.png?raw=true)

* Decoupling caps for VCC and V+ pins of the digipot must be kept in the vicinity of their respective pins.
* SDA and SCL may benefit from ferrite beads. For a similar reason, I made sure that the ground plane on the back copper is behind both these traces as well as the digipot. Not sure if that's how it works though...

### Component Selection

#### Selection of C, R\_A, and R\_B

As explained by [1] and [2], the frequency and duty cycle that the oscillator will assume can be adjusted by selecting values for the three main components, the capacitor C, the fixed resistor R\_A, and the potentiometer R\_B.

![Duty Cycle = R\_A / (R\_A + R\_B)](/images/555-timer-pwm-with-digipot/formula-duty-cycle.png?raw=true)

The duty cycle is solely determined by the ratio of both resistors. Where R\_A is the "top" resistor, in series with the capacitor charging path, and R\_B is the bottom resistor, in series with the discharge path. As you can see from the formula, the duty cycle can theoretically go up to 100%, but R\_A is constraining the lower limit. We want this to be small.

* R\_A = 499 ohms
* R\_B = 10k ohms (total potentiometer resistance value)

These resistor values can give us a minimum 5% duty cycle. This most likely is not enough to completely turn off the LEDs. That is something I wish I knew before I started, that this particular design is not able to achieve a full duty cycle range. (As a note, other methods of generating PWM signals can achieve a full 0-100% duty cycle range.)

The last component to select is the value of the charging/discharging capacitor. This capacitor has the most influence on frequency.

![Frequency = 1.25 / ((R\_A + R\_B) * C)](/images/555-timer-pwm-with-digipot/formula-frequency.png?raw=true)

The frequency we want should be <= 2 kHz, because this design is being made to control the enable signal of the AP5726 white led driver, and that chip is expecting an unfiltered PWM signal to be under 2 kHz. Going with the previous select resistor values, and a capacitor that is 0.1 uF, we get an expected frequency of 1190 Hz.

NOTE: One can still use this circuit and insert different valued components to change the duty cycle and frequency. This PCB layout uses 0805 (2012 metric) surface mount size resistors and capacitors.

The 1.25 constant in this formula comes from the inverse of 0.8, which itself had been changed from its original value of 0.693. This value is the ln(2), derived from some sort of manipulation of an exponential growth/decay formula. The site [2] mentions that the value was changed to 0.8 to account for using diodes with lower than usual threshold voltage. The derivation of this is not something I was able to find or come up with myself... In my experiment, this does not accurately reflect the empirically observed frequency that ended up being half the estimated value.

I was able to take that measurement and deduce that in my real component, the 1.25 factor is actually 0.546 for some reason.

![50% Duty Cycle Output Signal](/images/555-timer-pwm-with-digipot/50p-duty-cycle.jpg?raw=true)

This was done by taking the frequency measurement here and plugging that back into the frequency formula to solve for the "1.25" term. To get back to an expected 1190 Hz, one could halve the capacitor value to use 0.05 uF (50 nF) for a frequency of 1040.10 Hz. This is if 5V input voltage is used. If 3.3V will be used, the capacitor value may have to be reduced by a factor of 30%. (See [Post Mortem](#duty-cycle-range-is-unacceptable) for more information about this...)

#### 555-timer

The [TLC555CDR](https://www.digikey.com/en/products/detail/texas-instruments/TLC555CDR/276979) 555-timer is recommended for it's small form factor, low pin count, and ability to run at 3.3V.

#### Digipot

The [DS3502](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3502.pdf) was used because it was available in breakout board form from Adafruit. However, in retrospect, the [MCP4561T-103E (Alternative digipot with low wiper resistance)](https://ww1.microchip.com/downloads/en/DeviceDoc/22107B.pdf) is a much better part with similar features. (See [Post Mortem](#digipot-wiper-resistance-cannot-be-ignored) for why this is.) **This PCB layout is not designed to work with the MCP4561T-103E as-is. The pins differ between the two chips.**

#### Diodes

This design needs fast switching small signal diodes, preferably with a low threshold voltage. The [BAT34W-TP schottky diodes](https://www.digikey.com/en/products/detail/micro-commercial-co/BAT43W-TP/717224) were chosen.

### Using a Digital Potentiometer

#### Pinout and Hookup

![Raspberry PI 3 I2C Pinout](/images/555-timer-pwm-with-digipot/rpi-i2c-pinout.png?raw=true)

The SDA and SCL pins of the PCB should be hooked up to the corresponding SDA and SCL pins of your microcontroller of choice. I used a Raspberry Pi 3, as depicted in the above picture. One also needs to make sure the [I2C peripheral is enabled](https://www.raspberrypi-spy.co.uk/2014/11/enabling-the-i2c-interface-on-the-raspberry-pi/) and reboot the RPi. And that's all for the interfacing setup!

#### I2C interaction

One may use the available raspberry pi package "i2c-tools":

`sudo apt-get install i2c-tools`

This package provides a basic way to talk to I2C devices, using `i2cdetect`, `i2cset`, and `i2cget`.

```
#> i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- 28 -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --

#> i2cget -y 1 0x28 0x00
0x40

#> i2cset -y 1 0x28 0x00 0x00                 # set the digipot to 0 (shortest duty cycle)
#> i2cset -y 1 0x28 0x00 0x7F                 # set the digipot to 127 (longest duty cycle)
```

If everything is working correctly, you should see a device on I2C port 1 address 0x28. (This may differ if you've bridged the A0 and/or A1 pins). Upon intial start-up you should see a read to the digipot register will return 0x40, which is halfway to 128, corresponding to a 50% duty cycle.
 
## Post Mortem

This design was originally going to be used together with an [ap5726-lcd-backlight-driver](../ap5726-lcd-backlight-driver) to dim LEDs. Unfortunately, this design was not performing to expectations.

### Problems with this design

#### Duty Cycle Range is Unacceptable

![Min and Max Duty Cycle of Built 555-timer](/images/555-timer-pwm-with-digipot/555-timer-min-max.mp4?raw=true)

Here is a video of me writing the min and max values to the digipot and the subsequent duty cycle generated by those values. The difference is not very noticeable...

I measured the duty cycle of both min and max values. First, here is the total period measurement:

![555-timer Period Measurement: 398 Hz at 3.3V](/images/555-timer-pwm-with-digipot/pwm-freq-measurement.jpg?raw=true)

My oscilloscope shows 398 Hz frequency or 2.520 ms period. Here is the minimum duty cycle:

![555-timer Minimum Duty Cycle Measurement: 960us](/images/555-timer-pwm-with-digipot/min-duty-cycle-measurement.jpg?raw=true)

The high pulse is 960 us long. This corresponds to a duty cycle of 38.1%! Terrible!

And the max duty cycle:

![555-timer Maximum Duty Cycle Measurement: 2.040ms](/images/555-timer-pwm-with-digipot/max-duty-cycle-measurement.jpg?raw=true)

The high pulse is 2.040 ms long. This corresponds to a duty cycle of 81%! Also terrible!

For reference, here is a pwm cycle coming from a Raspberry Pi, that is able to go fully from 0 to 100% duty cycle:

![Raspberry Pi PWM signal demonstration](/images/555-timer-pwm-with-digipot/rpi-pwm-breathe.mp4?raw=true)

#### Frequency is Unpredictable

Using a fudge factor of 0.8 instead of 0.692 (a.k.a. the ln(2)) is due to adding in fast switching diodes that have a threshold voltage of about 0.4-0.5 volts instead of 0.7 volts. In the real world, the diode characteristics can still vary enough to make the actual frequency different. After putting together this circuit in real life, the frequency of the PWM signal was observed to be around 520 Hz @ 5V VCC and a 1190 Hz signal was expected.

The frequency is also dependent on input voltage. So while the frequency was in the 500 Hz range at 5V, it gets lower at 3.3V. At such voltage, the frequency was recorded to be around 390 Hz.

It is unknown exactly why there is a discrepancy...

#### Digipot Wiper Resistance Cannot be Ignored

The DS3502 digipot has a nominal wiper resistance of 5000 ohms. This is quite large and cannot be ignored when doing frequency calculations. This large resistance shows up on both the charge and discharge timing, reducing the duty cycle range from 5% - 100% to around 30% - 80%. This is not good, and the qualitative difference in LED brightness is barely perceptable. The lower duty cycle range is especially important to being able to dim the LEDs to near shutoff.

If I disconnect the digipot and use a 10k resistor and a short circuit to connect the RL, RW, and RH pins into the 555-timer, I can see what the actual duty cycle limits are without the digipot characteristics getting in the way.

If I put the 10k resistor across RW and RH, this will simulate minimum duty cycle:

![Minimum Duty Period Debug](/images/555-timer-pwm-with-digipot/debug-min-period-measurement.jpg?raw=true)
![Minimum Duty Cycle Debug](/images/555-timer-pwm-with-digipot/debug-min-cycle-measurement.jpg?raw=true)

The period here is 910 us and the duty cycle is high for 50 us. That corresponds to a 5.47% duty cycle. Very close to the calculated value!

Putting the 10k resistor across RW and RL will be equivalent to maximum duty cycle:

![Maximum Duty Cycle Debug](/images/555-timer-pwm-with-digipot/debug-max-cycle-measurement.jpg?raw=true)

This is very, very close to 100%. So this is pretty definitive evidence that the 5000 ohm wiper resistance is messing things up.

#### Design Uses Too Many Components

This design uses too many expensive components. The DS3502 (and similar) digipots are about $1.80 USD. This design also requires a 555-timer, 2 expensive diodes, 5 resistors, and 4 ceramic capacitors. Overall, the design costs around $5-7 USD for this single functional block. (Compare to using an ATTiny25, which is ~$1 USD and aside from 2 pull up resistors for I2C signals, is the only part you need.)

### Possible Mitigations

There are a few things one can implement to improve upon this design, however, none of them change the fact that it is unsuitable for real world usage.

* **Replace DS3502 digipot with one that has smaller wiper resistance**  
  For instance, the [MCP4561T-103E](https://ww1.microchip.com/downloads/en/DeviceDoc/22107B.pdf) not only has a nominal wiper resistance of 75 ohms but it has 257 steps instead of the DS3502's 127. This will probably improve the duty cycle range drastically. Combined with using a slightly smaller charging resistor (R\_A), you could get to the theoretical 5% minimum duty cycle.
     
* **Implement negative resistance op amp circuit**  
  Negating the wiper resistance of a digipot has [been thought of before](https://electronics.stackexchange.com/questions/159212/very-low-wiper-resistance-digital-pots-anyone-know-of-any). One may attempt to use an op amp to create a circuit that would negate the wiper resistance of the digipot. Article [here](https://www.electronicdesign.com/technologies/components/article/21758411/negative-resistance-nulls-potentiometers-wiper-resistance).  
    
  This was not done for this project. An alternate method will be chosen to dim the LEDs.

### Possible Alternatives

* Use a microcontroller to generate the PWM signal. Using an extra wire from a Raspberry Pi and generating a PWM signal from it works really well, at the cost of requiring an entire Raspberry Pi and an extra wire to generate the signal. If one wants to embed an microcontroller into the PCB so that the user will not have to supply their own microcontroller, one can use an [ATTiny25](https://www.microchip.com/en-us/product/attiny25). This microcontroller does not require any external components and can run off 3.3V. So the user simply needs to incorporate the footprint onto the PCB and then write fireware to configure I2C, non-volatile memory storage, and enable PWM generation on one of the pins.

* Use [multi-op-amps in an astable vibrator configuration](https://www.ti.com/lit/an/sboa212a/sboa212a.pdf?ts=1691788479469). This is similar to using a 555-timer, except the design is more complicated. The benefits are that the frequency will be more stable with the op-amps and the wiper resistance of the digipot will not matter because you can use a comparator op-amp to adjust to the digipot's output range.

## References/Further Reading

1. [How to Generate PWM Signal using 555 Timer IC?](https://www.electronicshub.org/555-timer-pwm/)
1. [555 Circuits Part 1](https://www.electronics-tutorials.ws/waveforms/555-circuits-part-1.html#:~:text=Fully%20Independent%20555%20Oscillator&text=The%20timing%20period%20for%20both,without%20affecting%20the%20output%20frequency.)
1. [NE555DR Datasheet](https://www.ti.com/lit/ds/symlink/ne555.pdf)
1. [DS3502 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3502.pdf)
1. [DS3502 Adafruit Breakout Board](https://learn.adafruit.com/ds3502-i2c-potentiometer/overview)
1. [MCP4561T-103E (Alternative digipot with low wiper resistance)](https://ww1.microchip.com/downloads/en/DeviceDoc/22107B.pdf)
1. [Negating Wiper Resistance with Negative Resistance](https://www.electronicdesign.com/technologies/components/article/21758411/negative-resistance-nulls-potentiometers-wiper-resistance)
1. [ATTiny25](https://www.microchip.com/en-us/product/attiny25)
