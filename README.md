# electronics-experiments

Educational and experimental PCB designs for Raspberry pi related products and peripherals

## Project List

### 555-timer-pwm-with-digipot

![50% Duty Cycle Output Signal](/images/555-timer-pwm-with-digipot/50p-duty-cycle.jpg?raw=true)

This is a hardware PWM output generator. This will give you a fixed frequency, variable duty-cycle PWM signal without using a microcontroller. This uses a 555-timer in astable oscillation mode that provides a 5% - 100% duty cycle. This allows you to control small motors and dim LED lights (see `ap5726-lcd-backlight-driver`).

### ap5726-lcd-backlight-driver

![AP5726 LED driver with LED mock fixture](/images/ap5726-lcd-backlight-driver/working-breadboard.jpg?raw=true)

This is a breakout board for testing the AP5726 white-LED driver (used for driving 5-7in LCD screen modules). This DC-DC converter is specifically designed to drive LEDs in series while supplying a constant 40mA current and up to 35V.

### as01808ao-enclosure

![AS01808AO Enclosure](/images/banner/as01808ao-enclosure.jpg?raw=true)

This is an experiment to mount an AS01808AO micro speaker to a PCB and have 3d printed enclosure parts bolted on. It works surprisingly well.

### attiny-402-i2c-pwm-generator

![ATTiny 402 I2C PWM Generator](/images/banner/attiny-402-i2c-pwm-generator.jpg?raw=true)

This is a project to use the new ATTiny series microcontroller to make an I2C controlled PWM signal that can be varied by frequency and duty cycle.

### max98357-amp

![max98357 Amplifier](/images/banner/max98357-amp.png?raw=true)

This is a design for a Class D speaker amplifier based on a [product by Adafruit](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp).

### retrocution-usb-hub

![Retrocution USB Hub](/images/banner/retrocution-usb-hub.jpg?raw=true)

The usb hub design uses an FE1.1s integrated USB 2.0 controller with the board layout heavily referenced from [Retrocution](https://www.retrocution.com/2020/01/15/easy-diy-tiny-usb-hub-for-raspberry-pi-projects/). Many thanks to Retrocution!
