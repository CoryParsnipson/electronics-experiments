# 555-timer-pwm-with-digipot

This is a hardware pwm signal generator using a 555 timer. The design used here is one that uses two diodes attached to both ends of a poteniometer to make the duty cycle adjustable without changing the frequency. A twist on the usual design is that this project also uses a digital potentiometer (a.k.a. digipot) so that the duty cycle can be adjusted programmatically with a microcontroller via an I2C interface.

The purpose of this project is to provide digital access to an adjustable PWM signal to dim an lcd backlight driver. (See [ap5726-lcd-backlight-driver](ap5726-lcd-backlight-driver/README.md).) The output signal of this project can be fed into the enable signal of an lcd backlight driver.

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 7.0.2.

## Description

### PWM Generation using 555-timer in Astable Multivibrator Mode

A good explanation of how 555-timers work and how they operate in astable-multivibrator mode can be found at the website of [1] in References. The author does a good job of explaining the functional logic and how it works. Basically, a 555-timer can be used to emit signal pulses at very specific, user-settable delays. This is known as "monostable" mode. A 555-timer can also be induced to oscillate like a clock signal by using feedback loops. This mode of operation is known as "astable" because it changes value by itself and "multi-vibrator" because it assumes different values.

Unfortunately, the author of [1] doesn't explain the PWM oscillator circuit in detail and really drops the ball at the end. I found a second page ([2] under References) that describes creating a PWM generator step-by-step starting from 555-timer astable mode. I am basing this project's circuit off of the "Fully Independent 555 Oscillator". [2] goes into much more detail about the circuit design.

### Selection of C, R\_A, and R\_B

As explained by [1] and [2], the frequency and duty cycle that the oscillator will assume can be adjusted by selecting values for the three main components, the capacitor C, the fixed resistor R\_A, and the potentiometer R\_B.

[TODO: insert equation for duty cycle]
[TODO: insert equation for minimum duty cycle (t\_a = 0.693 * R\_A * C) - this is the smallest pulse you will be able to make. i.e. duty cycle can never be exactly 0)]
[TODO: insert equation for calculating frequency]

The frequency we want should be <= 2 kHz, because this design is being made to control the enable signal of the AP5726 white led driver, and that chip is expecting an unfiltered PWM signal to be under 2 kHz.

C = 0.1uF
R\_A = 500 ohm
R\_B = 10k ohm digipotj

This will yield a frequency of ~1180 Hz, and a minimum duty cycle of about 5%.

### Using a Digital Potentiometer

[TBA]

[To be continued...]

## References/Further Reading

1. [How to Generate PWM Signal using 555 Timer IC?](https://www.electronicshub.org/555-timer-pwm/)
1. [555 Circuits Part 1](https://www.electronics-tutorials.ws/waveforms/555-circuits-part-1.html#:~:text=Fully%20Independent%20555%20Oscillator&text=The%20timing%20period%20for%20both,without%20affecting%20the%20output%20frequency.)
1. [NE555DR Datasheet](https://www.ti.com/lit/ds/symlink/ne555.pdf)
1. [DS3502 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3502.pdf)
1. [DS3502 Adafruit Breakout Board](https://learn.adafruit.com/ds3502-i2c-potentiometer/overview)
