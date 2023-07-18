# ap5726-lcd-backlight-driver

This is a design using the AP5726 integrated chip to drive backlights on small (cellphone-sized) LCD screens. The requirements for many screens are idiosyncratic, requiring support for high voltages of up to 18-30V and 40mA of *constant* current to drive two parallel strings of 4-6 white LEDs in series.

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 7.0.2.

## Description

![Working PCB with breadboard LED mock fixture](/images/ap5726-lcd-backlight-driver/working-breadboard.jpg?raw=true)

The backlight in LCD displays ranging from 3-7 inches in size consist of two rows of white surface mount LCDs in parallel, spanning the top and bottom edges of the screen. Each branch contains 3-6 LEDs. The LCD screen also has special diffuse material that lets the light from the LEDs spread out across the surface of the screen. One may think of the backlight in a schematic as such:

![Schematic of LED array](/images/ap5726-lcd-backlight-driver/led-schematics.png?raw=true)

The specifications here claim that the LEDs should be driven with a maximum of 40mA of current, and the voltage drop across the entire LED circuit will be 18.2V. You can drive with lower current, and the screen will be less bright.

It is important to find a dc-converter or regulated power supply that is specifically constant current so that the screen backlight will not flicker randomly during operation.

### Schematic

![Schematic of AP5726 Breakout Board](/images/ap5726-lcd-backlight-driver/ap5726-breakout-schematic.png?raw=true)

The schematic of this breakout board is fairly minimal and based on [page 12 of the AP5726 datasheet](https://www.diodes.com/assets/Datasheets/AP5726.pdf).

My schematic has the addition of a solder jumper on the LED\_A pin and a test pad on the FB pin. This is because the intention is to dim the lights using the ENABLE pin, but in case I wanted to use a filtered pwm signal, I would be able to splice a voltage and resistor with the solder jumper.

For more information about this, see the "LED Dimming" section below.

#### Inductor Selection

The datasheet recommends inductors between 10 and 22 uH (micro henries). The inductor should have low core losses at 1.2 MHz and low DCR (copper wire resistance). Lastly the inductor's current saturation should exceed the peak input current of the design.

The recommended parts from the AP5726 datasheet were mostly obsolete or out of stock. I ended up using the [LQH44PN220](https://www.digikey.com/en/products/detail/murata-electronics/LQH44PN220MPRL/16516913), which seems to satisfy all of the constraints described above. (290milliOhm DCR and saturation current at 1.05A)

##### Calculating Peak Input Current and Minimum Input Current

The calculation of peak and minimum input current is partially described [here](https://components101.com/articles/boost-converter-basics-working-design#:~:text=a%20boost%20converter.-,Boost%20Converter%20Design,and%20current%20independently%2C%20the%20product%20of%20which%20is%20our%20output%20power.,-STEP%20%E2%80%93%202).

> STEP – 1
> 
> To begin with, we need a thorough understanding of what our load requires. It is highly recommended (from experience) that if you attempt to build a boost converter at the beginning it is very important to know the output voltage and current independently, the product of which is our output power.
> 
> STEP – 2
> 
> Once we have the output power, we can divide that by the input voltage (which should also be decided) to get the average input current needed.
> 
> We increase the input current by 40% to account for ripple. This new value is the peak input current.
> 
> Also the minimum input current is 0.8 times the average input current, so multiply the average input current by 0.8.
> 
> Now that we have peak and minimum current, we can calculate the total change in current by subtracting the peak and minimum current.

We can work backwards for our application. We know that we need to deliver 40mA of current to the LEDs, which will have approximately 18.2V across them.

`P = i * v = 0.04 * 18.2 = 0.728 Watts`

Now for step 2, dividing the output power by input voltage, which will be 3.3V here. (5V will work too, but my design is going to be powered by a 3.3V supply).

`I_avg = 0.728 / 3.3 = 0.221 Amps`

So next nominal peak input current is this average value times 140%:

`I_peak_nom = 0.221 * 1.4 = 0.309 Amps`

But we must also take into account the efficient of our converter (it's about 75-80% according to the datasheet). So lets go with 70% to be safe, meaning we need to multiply by 1.43:

`I_peak = 0.309 * 1.43 = 0.441 Amps`

And to get minimum input current, we can multiply average input current by 0.8 and 1.43 to account for converter efficiency:

`I_min = 0.221 * 0.8 * 1.43 = 0.252 Amps`

Now that we know peak and minimum input current, we can see that the saturation current of the inductor, which is 1.05A, is well above anything we can expect.

Also, the 18.2V @ 40mA screen is one of a few I'm intending to drive, with the most power intensive being a screen that has a voltage drop of 30V @ 40mA. This screen has a peak current of 0.8A-ish. So it's good that this inductor seems to be able to handle quite a few different situations.

#### Capacitor Selection

Small X5R or X7R ceramic surface mount capacitors are recommended for this. An output and input capacitor of 1uF should be sufficient, according to the datasheet, however, for high current applications, one may want to use 2.2uF - 4.7uF for the output capacitor to minimize output ripple.

Generic [1uF ceramic SMD capacitors in 0805 (2012 metric) from digikey](https://www.digikey.com/en/products/detail/samsung-electro-mechanics/CL21B105KAFNNNE/3886724) were sufficient.

[What does X5R, X7R, Y5V, Z5U mean for capacitors?](https://www.allaboutcircuits.com/technical-articles/x7r-x5r-c0g...-a-concise-guide-to-ceramic-capacitor-types/)

#### Diode Selection

Schottky diodes were recommended for their low forward voltage drop and fast reverse recovery. Look for a diode with low capacitance to reduce switching losses at 1.2MHz. A diode rated for 200-300 mA is supposed to be sufficient.

[BAT-43 form factor SMD schottky diodes](https://www.digikey.com/en/products/detail/micro-commercial-co/BAT43W-TP/717224) were used in my assembly.

#### R\_set Selection

The output current of the AP5726 is configurable by setting the value of Rset, on the FB pin. There is a chart showing how to calculate this value on page 9 of the [AP5726 datasheet](https://www.diodes.com/assets/Datasheets/AP5726.pdf). For us, we want 7.5 Ohms for 40 mA output. The datasheet also recommends 1% precision resistors.

A [1% 7.87 Ohm 0805 SMD resistor](https://www.digikey.com/en/products/detail/vishay-dale/CRCW08057R87FKEA/1961753) was chosen for this. This is an E96 series resistor and the closest single component you can find to 7.5 Ohms. Remember, getting slightly above the 7.5 value will translate to slightly less current (and slightly less maximum brightness). It is preferred to err on this side, rather than having slightly too much current.

### PCB layout

There are some EMI and high frequency considerations for this particular design. The [LT1932 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/1932f.pdf) has some really good tips on page 9. The LT1932 is a very similar chip to the AP5726.https://www.analog.com/media/en/technical-documentation/data-sheets/1932f.pdf

![LT1932 PCB layout tips](/images/ap5726-lcd-backlight-driver/lt1932-layout-tips.png?raw=true)

In KiCAD, I used copper fill zones to encapsulate VCC and GND, as depicted in the LT1932 datasheet. The entire back of the PCB is a ground copper pour.

![AP5726 PCB Layout](/images/ap5726-lcd-backlight-driver/ap5726-breakout-layout.png?raw=true)

### Testing

![The AP5726 is physically very small](/images/ap5726-lcd-backlight-driver/ap5726-scale.jpg?raw=true)
The AP5726 is physically *very* small.

![Mostly assembled breakout board](/images/ap5726-lcd-backlight-driver/assembled-pcb.jpg?raw=true)

![KiCAD Visualizer vs real life board](/images/ap5726-lcd-backlight-driver/visualizer-vs-irl.png?raw=true)

The components are small and it is recommended to assemble the board using flux and a soldering iron with an SMD tip.

![Working PCB with breadboard LED mock fixture](/images/ap5726-lcd-backlight-driver/working-breadboard.jpg?raw=true)

Using an Arduino Pro Micro as a USB power supply (at 4.5V), the backlight driver powers 2 parallel branches of 6 LEDs in series. The basic PCB functionality is successful!

### LED Dimming

There are several methods described by the AP5726 datasheet that can be used to partially dim the LEDs:

1. Tie EN directly to VCC or GND to enable/disable respectively. There is no gradual dimming with this method.
1. Using a PWM signal to EN. The EN pin expects a PWM signal with frequency <= 2 kHz. The average LED current increases proportionally to the duty cycle of the PWM. 0% duty cycle corresponds to "off" and 100% duty cycle is maximum brightness.
1. Using a DC Voltage (or a filtered PWM signal). This is done by supplying a DC voltage to the FB pin via a 100k resistor, and hooking up a 5k resistor to the LED anode. There is a diagram on page 11 of the AP5726 datasheet portraying this. A PWM signal filtered with a low pass filter can be used to supply the DC voltage.
1. Logic signal. If one needs only a few discrete brightness level, then using discrete transistors and digital signals into their gates may work.

Using a PWM signal seems like the easiest, most flexible way to provide dimming. One may use the PWM output capabilities of an Arduino or Raspberry Pi. Unfortunately for this design, it is intended to be connected to a Raspberry Pi that is already using the analog outputs for speaker audio. Thus, the PWM signal generation must be handled by separate hardware. The [555-timer-pwm-generator](../555-timer-pwm-with-digipot) project was created to serve this purpose.

[To be continued...]

## References/Further Reading

* [ap5726 Datasheet](https://www.diodes.com/assets/Datasheets/AP5726.pdf)
* [lt1932 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/1932f.pdf)
* [Calculation of Peak and Minimum Input Current](https://components101.com/articles/boost-converter-basics-working-design#:~:text=a%20boost%20converter.-,Boost%20Converter%20Design,and%20current%20independently%2C%20the%20product%20of%20which%20is%20our%20output%20power.,-STEP%20%E2%80%93%202)
* [What does X5R, X7R, Y5V, Z5U mean for capacitors?](https://www.allaboutcircuits.com/technical-articles/x7r-x5r-c0g...-a-concise-guide-to-ceramic-capacitor-types/)
