# ap5726-lcd-backlight-driver

This is a design using the AP5726 integrated chip to drive backlights on small (cellphone-sized) LCD screens. The requirements for many screens are idiosyncratic, requiring support for high voltages of up to 18-30V and 40mA of *constant* current to drive two parallel strings of 4-6 white LEDs in series.

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 7.0.2.

## Description

The backlight in LCD displays ranging from 3-7 inches in size consist of two rows of white surface mount LCDs in parallel, spanning the top and bottom edges of the screen. Each branch contains 3-6 LEDs. The LCD screen also has special diffuse material that lets the light from the LEDs spread out across the surface of the screen. One may think of the backlight in a schematic as such:

![Schematic of LED array](/images/ap5726-lcd-backlight-driver/led-schematics.png?raw=true)


The specifications here claim that the LEDs should be driven with a maximum of 40mA of current, and the voltage drop across the entire LED circuit will be 18.2V. You can drive with lower current, and the screen will be less bright.

It is important to find a dc-converter or regulated power supply that is specifically constant current so that the screen backlight will not flicker randomly during operation.

[To be continued...]

## References/Further Reading

* [ap5726 Datasheet](https://www.diodes.com/assets/Datasheets/AP5726.pdf)
