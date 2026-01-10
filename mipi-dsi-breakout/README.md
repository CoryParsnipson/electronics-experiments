# mipi-dsi-breakout 

This design is intended to interface with different MIPI DSI LCD screens and route the DSI signals to a Raspberry Pi.

## KiCAD Version

Current version: KiCAD 9.0.3

This was originally made with KiCAD 9.0.3.

## Sponsorship

The PCB fabrication for this project is sponsored by [PCBWay](https://www.pcbway.com/). PCBWay is a manufacturer of printed circuit boards and they also offer services such as PCB assembly, 3D printing plastic and metal, and much more. Their online quote system and thorough customer service made this design a breeze to order.

## Description

![Assembled MIPI DSI breakout PCBs](/images/mipi-dsi-breakout/assembled-breakout.jpg?raw=true)

When working with microcontrollers, you can buy "all-inclusive" LCD screen kits from electronics hobby stores that are basically plug and play. With this, the user experience is quite nice, needing only to plug in a video cable going from the LCD screen circuitry directly to the microcontroller. However, these kits leave little flexibilty terms of physical form factor, size and type of LCD screen, and price.

This project is an attempt to go beyond the beginner LCD peripherals offered by microcontroller hobby stores and interface directly with an LCD screen using custom peripheral circuitry. The hope is to increase the available LCD screens one can use beyond the small selection specifically available for a particular family of microcontroller products.

### Background and Context

![OSOYOO 5 inch DSI LCD Screen for Raspberry Pi](/images/mipi-dsi-breakout/osoyoo-5in-dsi-screen-1.jpg?raw=true)

An existing product like the [OSOYOO 5 inch DSI LCD Screen for Raspberry Pi](https://www.amazon.com/OSOYOO-Touchscreen-Monitor-Raspberry-800%C3%97480/dp/B0CWL7R8T4) was used as a reference base. This is a 5 inch screen that is designed to work out of the box with the Raspberry Pi family of microcontrollers.

It is desirable for the following feaures:

* Built in touch screen
* Uses the DSI interface to transfer data (no need to use up an HDMI, USB, or GPIO pins to hook up)
* Supported out of the box by Raspberry Pi OS (no need to install external drivers)

It falls short due to:

* Relatively high price (~$50, too high for production runs)
* Large physical form factor (it is approximately 10 mm thick, permanently glued to a PCB, and contains metal standoffs which limit including it in a portable electronics device)
* Limited to 5 inch LCD screen and resolution from manufacturer

The objective of this project is to prototype something that could potentially replace this product while allowing one to swap the screen out and adjust the PCB design.

### Design Overview

For context into how this fits into a larger project, refer to the following block diagram:

![System block diagram](/images/mipi-dsi-breakout/system-block-diagram.png?raw=true)

Here we can see a motherboard for a Raspberry Pi Compute Module. This is a microcontroller board that you can use just like a regular computer once you hook up a keyboard, mouse, monitor, and power cord into it.

On the right, there is a section labeled "LCD Screen" (imagine that it is the OSOYOO screen). Described in 4 numbered functional blocks surrounding the screen component is the peripheral circuitry required to connect the LCD screen to the microcontroller.

Assuming that everything is built and working, we should be able to see the operating system graphical output on the LCD screen as if we were using it as a computer monitor. This project is a PCB implementation of the functional blocks labelled (1) and (4).

For information about implemting blocks (2) and (3), see the other projects in this repository, [ap5726-lcd-backlight-driver](/ap5726-lcd-backlight-driver) and [attiny-402-i2c-pwm-generator](/attiny-402-i2c-pwm-generator) respectively.

### LCD Screens Overview

The LCD screens this project is concerned with are mostly *Thin Film Transistor* (TFT) screens. See [this page](https://www.topwaydisplay.com/en/blog/how-tft-lcd-works-explained) to learn more about how TFT screens work. Essentially, TFT LCDs are a VLSI design that prints individual transistors on a transparent substrate that act as individual pixels. So the takeaway here is that it is not very simple or inexpensive to produce these, and that there are very expensive foundries needed to manufacturer these like one would need to make CPUs and other integrated circuits, though a TFT screen does not require as small transistors nor is the design as complex as a CPU.

![LCD screen types](/images/mipi-dsi-breakout/lcd-screen-types.jpg?raw=true)
*Left: segmented (monochrome, big segments) Middle: dot matrix (monochrome, medium resolution) Right: TFT (graphical, transistors, RGB color)*

When searching for LCD screens one must be careful to specify which kind, between a segmented display, dot matrix, or a fully graphical screen. Here we want graphical displays.

Within the graphical LCD displays are Twisted Neumatic (TN) and In-Plane Switching (IPS) type displays with the difference being that IPS displays are more vibrant, brighter, and have a better viewing angle range than TN displays, which are older and cheaper than IPS displays.

TBD
There are also Organic LED (OLED) screens now, which are a different technology where VLSI LEDs are printed directly onto a substrate instead of whatever circuitry and electrodes TFT screens use. OLED screens are even more vibrant, have true black color, and supposedly use less energy because each pixel that is turned off does not consume power. These screens are better in every way, but are still relatively new and thus more expensive and less available. Now you have AMOLED screens which are minor improvements upon an OLED screen but mostly the same thing.

> NOTE: as of the time of this writing, if you try to buy an OLED screen from an electronics supplier, you may find very (physically) small, extremely low resolution, monochrome screens that have more in common with segmented displays or something you'd see in an old calculator. If you want a *graphical* OLED screen, you would need to specifically search for it, or try to get an AMOLED screen instead. Still, these are not as common as IPS displays.

### Overview of LCD Interfaces

If that wasn't complicated enough, one must also have to worry about the interface of the LCD screen, and there are many different interface protocols. Some of the more common ones:

* HDMI
* MIPI DSI
* LVDS
* SPI
* RGB/MCU/Parallel/DPI

#### HDMI

This might be the most popular screen interface. With an HDMI controller, the screen hooks up to a host using an HDMI cable. This makes it compatible with anything that supports the HDMI protocol, and not just a specific device. It also supports audio and input signals as well. This seems to be the most popular interface type for LCD screens around 5 inches in size, so it would give us the best options for screens.

Some of the disadvantages are the relatively complicated (and physically large) circuitry required, and the extra power consumption from converting from whatever format the LCD screen is in natively to HDMI. For these reasons, the search for screens for this project was limited to MIPI DSI.

> NOTE: in a "real" product development environment, what probably happens is that the designer will look for screens with the required attributes, and then integrate whatever interface the desired product was using afterwards. Going forward, this is probably a much better attitude than choosing the interface first.

#### MIPI DSI

MIPI DSI stands for *Mobile Industry Processor Interface Display Serial Interface*. For this project specifically, we have chosen to look for DSI screens exclusively because the DSI interface is convenient for Raspberry Pi microcontrollers. Having a DSI screen uses less power, supports higher resolutions and refresh rates, low physical profile, and does not take up GPIO or other ports that could be used for other things. The DSI protocol is also meant for internal connectors for mobile and low power and physical small devices.

#### LVDS

"Low Voltage Differential Signaling" actually refers to the transmission method and not an entire protocol, so it is not quite equivelant to other things on this list. In fact, MIPI DSI is a protocol that sits on top of LVDS. However, you may see this and "Enhanced LVDS" (eLVDS) when searching for screens and this just means that it uses LVDS and then a proprietary protocol on top to transmit data. This is common for larger screens such as for laptops and tablets, and only found buried in commercially manufactured devices. It is generally not recommended to use a screen that claims to use LVDS because it is hard to reverse engineer or find documentation on proprietary protocols, especially since they would be completely different across screens.

#### SPI

Serial Peripheral Interface (SPI) is a protocol used for many devices, not just screens. This is common for graphical LCD screens 3-4 inches and smaller, since the maximum data rate for SPI is not very fast. Even then, you may observe that the framerate is not very high due to the transfer rate limitations. Furthermore, an SPI screen will probably require more CPU overhead and power because the protocol is not designed specifically to transmit video. One good thing is that the interface and protocol is very simple, so it is very easy to integrate into a device as long as the screen is small and the framerate and resolution is not too large.

SPI devices usually hook in through GPIO pins.

#### RGB/MCU/Parallel/DPI

Display Parallel Interface (DPI) is similar to SPI but is faster so supports higher resolutions. It still suffers from similar power and CPU overhead limitations. It also uses GPIO pins, but a very large amount. For example, in a Raspberry Pi, hooking up a DPI screen may use almost all of the 40 available GPIO pins. This is not convenient if one wants to integrate multiple peripherals to the same microcontroller.

## Part Selection

### LCD Screen Parts

Finding LCD screen modules that are not part of a kit directly from manufacturers is tough. Lots of the supply is only available from China.

There were three main websites explored in my search for screens:

1. [Aliexpress](https://www.aliexpress.us)
   While not the most user friendly place, it contained the largest and relevant selection. I ended up buying all screens I wanted to try out from here. The "negotiation" process of talking to different salespeople to get a list of parts with the required specs was long and tedious. The pricing shown is not always accurate, but you can haggle with them. The price is dependent on the quantity you buy, which is usually a minimum of 500 or 1000 pieces, but you can order a few samples for a higher price for development (which is what I did).

1. [Waveshare](https://www.waveshare.com)
   This is more of a general electronics shop like Adafruit or SparkFun. For places like these, screen selection is very small, but the quality of parts is much better. The price is retail price, so not really ideal, but would still work for prototyping and development.

1. [BuyDisplay](https://www.buydisplay.com)
   This is another website that specializes in displays. The selection is large (but not as large as Aliexpress), and contains lots of unique parts.

#### Search Criteria

I was looking for a graphical, full color LCD screen that was around 5 inches across. Resolution around 480x800 or slightly higher is fine. The difficult part was finding parts with a MIPI DSI interface, which aren't very common especially on smaller screens. Looking at nits (>= 200) is a nice to have, to get an idea of whether or not the screen would be visible outdoors. A capactive touch screen was also a nice to have. If the screen comes with a touch panel, you can also ask them to customize the shape of the front lens/sticker if you order in bulk. This could be useful when putting together a complete product.

#### 4.3 Inch 480x800 MIPI DSI Display with CTP

The New Nintendo 3DS (non XL) top screen is about 4 inches across, and I think that is a nice size, so 4.3 inches wasn't out of the question. I bought one display I found at this size just to see if it would feel too small or not.

![4.3 Inch DSI Display with Capacitive Touch Screen](/images/mipi-dsi-breakout/4.3-inch-dsi-display.jpg?raw=true)

This was purchased from Dongguan Team off Aliexpress. The part number is `TST043WVBI-130` (or `TST043WVBI-130C` for the touch screen variant).

> NOTE: none of these screens are available anymore, but the suppliers probably have very similar products available for purchase. Suppliers are selling surplus screens from bigger customer orders, so inventory goes quickly and then parts change very often.

[Datasheet (without CTP)](/mipi-dsi-breakout/docs/TST043WVBI-130.pdf?raw=true)<br />
[Datasheet (just schematic with CTP)](/mipi-dsi-breakout/docs/TST043WVBI-130C.pdf?raw=true)

#### 5 Inch 720x1280 MIPI DSI Display with CTP

![5 Inch DSI Display with Capacitive Touch Screen](/images/mipi-dsi-breakout/5-inch-dsi-display-with-ctp.jpg?raw=true)

The TST050HDBS-80C was also purchased from Dongguan Team off Aliexpress.

An interesting thing about this is how large and lopsided the bezels on the touch panel are. It looks like this came from a small, budget tablet device.

Another aspect of this part is that it is a 4 lane MIPI DSI device. The Raspberry Pi I am using has one DSI port with 2 lanes and another with 4 lanes. Supposedly, if you hook up a 4 lane device to a 2 lane host port, it should still work, but the bandwidth/framerate will be lower. I bought 2 different 5 inch screens with differing resolutions to see if you could notice the difference.

[Datasheet](/mipi-dsi-breakout/docs/TST050HDBS-80C.pdf?raw=true)

#### 5 Inch 480x854 MIPI DSI Display

![5 Inch DSI Display](/images/mipi-dsi-breakout/5-inch-dsi-display.jpg?raw=true)

The ET050FW04-T was purchased from Shenzhen Eurotech Technology Co. Ltd. off Aliexpress.

The odd screen resolution comes from old cellphones, I think, which have a particularly elongated aspect ratio. This is the 2 lane MIPI DSI counterpart to the other DSI screen, and it doesn't come with a capacitive touch panel.

[Datasheet](/mipi-dsi-breakout/docs/ET050FW04-T.pdf?raw=true)

#### 6 Inch 720x1280 MIPI DSI Display

![6 Inch DSI Display with Capacitive Touch Screen](/images/mipi-dsi-breakout/6-inch-dsi-display.jpg?raw=true)

The ET060HD02-T was purchased from Shenzhen Eurotech Technology Co. Ltd. off Aliexpress.

> NOTE: while all the other screens on this list have a 0.5mm pitch FPC data cable, this screen has a 0.3mm pitch data cable!

The salesperson told me that they had a lot of 6 inch screens (an unusual size) that were from a 3d printer. Judging from the size and resolution, I'd say it was for a resin printer.

After playing on the Nintendo Switch for a while, I got the idea that maybe a larger screen would be nice too. The size of this screen is probably around the same as the Nintendo Switch Lite (though not nearly as nice) or slightly smaller than the original Nintendo Switch screen.

[Datasheet](/mipi-dsi-breakout/docs/ET060HD02-T.pdf?raw=true)

### Raspberry Pi Host Motherboard

The host for this project is a [Raspberry Pi CM3+ with 32 GB](https://www.raspberrypi.com/products/compute-module-3-plus/) of on board eMMC memory. This is the version of the Raspberry Pi intended for industrial applications and so cannot be used by itself.

![Waveshare Compute Module IO Board Plus](/images/mipi-dsi-breakout/compute-module-io-board-plus-5.jpg?raw=true)

The Raspberry Pi is paired with the [Compute Module IO Board Plus by Waveshare](https://www.waveshare.com/compute-module-io-board-plus.htm).

This project can be replicated with a different microcontroller or PC, as long as the device supports DSI display ports. The compute module io board plus has a 22 pin DSI port with a specific pinout, so using a different microcontroller would most likely require modifying the designs in this repository to match.

[Here is a schematic](https://files.waveshare.com/upload/7/75/CM3-board.pdf) of the compute module IO board plus with the DSI port pinout.

### Zif Connector Selection

On the custom PCB, zif (Zero-Insertion Force) connectors comprise the majority of components. Aside from pin header, the only other components are 4.7 kOhm resistors for the I2C lines.

* [8pos 0.5mm connector (for 5 inch CTP cable)](https://www.digikey.com/en/products/detail/te-connectivity-amp-connectors/2328702-8/9565576)
* [10pos 0.5mm connector (for 4.3 inch CTP cable)](https://www.digikey.com/en/products/detail/te-connectivity-amp-connectors/1-2328702-0/9565577)
* [20pos 0.5mm connector (for 4.3 inch DSI cable)](https://www.digikey.com/en/products/detail/cvilux-usa/CF4220FH0R0-05-NH/15793107)
* [22pos 0.5mm connector (for RPI DSI port connector)](https://www.digikey.com/en/products/detail/cvilux-usa/CF4222FH0R0-05-NH/15793179)
* [25pos 0.3mm connector (for 6 inch DSI cable)](https://www.digikey.com/en/products/detail/cvilux-usa/CF5825DH0RA-NH/15793009)
* [30pos 0.5mm connector (for 5 inch DSI cable)](https://www.digikey.com/en/products/detail/amphenol-cs-fci/F3311A7H121030E200/15287516)
* [40pos 0.5mm connector (for 5 inch DSI cable)](https://www.digikey.com/en/products/detail/cvilux-usa/CF4240FH0R0-05-NH/15793119)

Exact parts are not necessary, many of the parts with the same specs have similar physical dimensions.

> WARNING: Careful about the 25pos connector, which is the only one that is 0.3mm pitch and its very different from the others.

### Other PCB components

You will also need a [22-pin 0.5mm FPC flx cable](https://www.digikey.com/en/products/detail/adafruit-industries-llc/6036/25589362), preferably at least 4.

Male pin header 2.54mm pitch. (At least 4 of 1x6, and optionally, 4 of 1x3 for the output signals).

And [4.7 kOhm surface mount resistors, 0805 (2012 Metric)](https://www.digikey.com/en/products/detail/te-connectivity-passive-product/CRGCQ0805F4K7/8576359), tolerance not important.

### Peripheral Electronics

To hook up all the input signals on this PCB via pin header, you will need:

* [ap5726-lcd-backlight-driver](/ap5726-lcd-backlight-driver)
* [attiny-402-i2c-pwm-generator](/attiny-402-i2c-pwm-generator)

These two circuits can be placed on a breadboard and hooked up to this PCB via jumper cables and pin header.

## PCB Layout

The main novelty of this project was learning to route high speed differential signals and complicated surface mount components.

This part in paricular was a very useful reference for MIPI DSI signal routing:

[PCB Artists - MIPI DSI PCB Layout Notes](https://pcbartists.com/design/pcb-design/mipi-dsi-pcb-layout-notes/)

### Differential High Speed Signal Background

The MIPI DSI protocol relies on high speed differential pair signals. To create a MIPI DSI LCD interface board, we will need to know how to route these signals. High speed, meaning signals that switch with frequency higher than 50 MHz. For the DSI protocol, it can reach speeds up to 480 MHz, for higher resolutions and refresh speeds. In this project, we are targeting screens around 5 inches in size and slightly lower than HD resolution (480x800 or 1280x1024), so speeds will not be as high, but probably around the 50 - 100 MHz range.

The term differential pair refers to the fact that each signal is split into two complementary signals that are transmitted together. In other words, when one signal is high, the other signal is low. The destination device will receive both signals and take the difference between their values to recover the original signals.

![Illustration of Differential pair signals](/images/mipi-dsi-breakout/differential-pair-signals.png?raw=true)

Doing things this way allows one to transmit at lower voltages since the amplitude of each individual signal can be half the maximum value required, and also allows for higher data rates, and protection against electromagnetic interference (EMI).

The tradeoff is that it requires more board space for extra traces, specific geometric and PCB design constraints to accomodate the differential pair layout, and protocol overhead to recover the original signal.

What is of particular interest for this project are all the ways including differential pairs change the PCB layout constraints compared to "ordinary" non-high speed and non-differential signals. A design that is meant to include differential pairs must be done at the very start, because it involves all aspects of the board and layout.

#### Coupling

To be coupled means that when one item changes in a property (like voltage or current), the other will be influenced to some degree to change in the same manner. Both signals in a differential pair are meant to be coupled to each other. Unfortunately, because we need to rely on this coupling, differential pair signals are susceptible to EMI, which usually means unintentional coupling to outside signals.

[![Differential Pairs PCB Design Basics](/images/mipi-dsi-breakout/diff-pairs-pcb-design-basics.jpg?raw=true)](https://www.youtube.com/watch?v=xwrdvhm4vgw)

This video is particularly informative on the theoretical side. In particular, these two slides were pretty illuminating:

![Twisted Pair Coupling](/images/mipi-dsi-breakout/twisted-pair-coupling.jpg?raw=true)

A true coupling comes about when electrical signals are in proximity with each other and their electromagnetic fields influence each other.

![PCB Differential Pair](/images/mipi-dsi-breakout/pcb-differential-pair.jpg?raw=true)

On a PCB, the coupled pair is actually an abstraction. Here the differential pair signals aren't coupled with each other, but are both coupled to ground, and then routed next to each other to symbolize the intention that they are coupled. And the source of these digital signals makes sure to set their values to be complementary at all times.

The main takeaway here is how important the ground/reference plane is to differential pairs routed on a PCB. The reference plane must be aligned with the signals at all points, or else you will end up with a large cross sectinoal area susceptible to EMI. Thus it is important not to route signals where the ground plane cannot follow.

I found some of the principles described in this video were also helpful to reason about how certain kinds of layout features could affect the signal. The two main considerations for signal integrity are reducing EMI (by reducing opportunities for outside signals to couple with the diff pair) and to reduce skew--both intrapair and inter-channel skew--meaning that all diff pair wires should be around the same exact length so the signal arrives at the destination at the same time.

#### How to Route Differential Signals in KiCAD

[![KiCAD Differential Pairs from Basics to Memory](/images/mipi-dsi-breakout/routing-differential-signals-in-kicad-thumbnail.jpg?raw=true)](https://www.youtube.com/watch?v=M13QxtPVrXY)

I found this video to be very helpful overview of what to do. The rough steps are as follows:

1. Set the PCB stackup and manufacturer
1. Create and assign a netclass for differential pair wires (they need a specific thickness and spacing).
1. Use calculator to find the diff pair parameters
1. Edit the netclass for differential pair signals
1. Route
1. Tune length
1. Tune skew

> NOTE: I've reversed the first two steps compared to the video because I think it makes more sense this way.

#### How to Target the Stackup for a Specific PCB Manufacturer

The very presence of high speed differential pair signals influences the board even before anything is created. The thickness and spacing of the signal wires is influenced by the geometry of the PCB, and so one must target a specific PCB company from the beginning. To switch manufacturers means make small modifications to match the new manufacturer's fabrication process.

[PCBWay has a helpful page](https://www.pcbway.com/multi-layer-laminated-structure.html) that shows their fabrication specifications.

![PCBWay 4 layer stackup](/images/mipi-dsi-breakout/pcbway-4-layer-stackup.png?raw=true)

Here is a stack up for a 4 layer board that is 1.6mm thick (standard) with 1oz outer and inner copper. PCBWay specifically gives you values for different residual copper. 100% means completely covered in copper and 0% means completely devoid of copper. The actual value varies per design since it is determined by all the gaps and fill of copper in the layout. Using 50% might be a good rule of thumb, but 70% is shown here because this design has large ground fills on all layers.

Most large fabs should have similar information on their websites or through customer service.

#### Using Impedance Calculator to figure out differential pair signal spacing and thickness

Once a manufacturer and stackup are selected, we can use the Sierra Circuits impedance calculator to find the trace width and spacing.

[Sierra Circuits Impedance Calculator](https://impedance.app.protoexpress.com)

1. Choose "Uncoated Microstrip" and "Differential Pair (Non coplanar)". The reference plane will not be on the same layer as the differential signals, so it is considered non coplanar.

1. Switch units from mils to mm.

1. Fill in "Dielectric Height and Dielectric Constant" from the stackup page. These values will probably be taken from the "Prepreg 1" layer, since it is the dielectric layer between the copper layer and the reference plane.

1. Set an arbitrary but sensible starting value for "Trace Separation". In the video he used 0.2mm.

1. Set the target differential impedance, which is 90 Ohms here. The [MIPI DSI PCB Layout Notes](https://pcbartists.com/design/pcb-design/mipi-dsi-pcb-layout-notes/) mentions that the spec calls for 90 Ohms but also has a margin of error of 15%.

1. Click "Calculate W" and see what value appears for "Trace Width". We want this value to be entered into the trace width for our netclass in KiCAD.

#### Making a NetClass in KiCAD

This can be done in board properties the same was it is done in the video. Use the default value for everything except for the values you selected for Trace Separation and Trace Width. They go in the entries for "DP Gap" and "DP Width", respectively.

Lastly, in the same netclass window, make sure that the right signals are associated with the proper netclass by adding entries and mapping the signal names to it. (You can use wildcards to make this easier.)

## Physical assembly and Test Setup

### PCB assembly

It is highly recommended to use a hot plate or reflow oven for ease of use. A hot air rework station may work as well, but will be more difficult especially for soldering the wider, higher-pin connectors.

I also used a regulated soldering iron with a 0.3mm conical smd tip to do some touch ups and to tin the solder pads without a stencil. (Obviously, a stencil would be easier, if available.)

> WARNING: this design has very large ground pours on all 4 layers of the PCB. This is something to take into consideration when trying to assemble the board.

Here is a picture of one of the circuits with the zif connectors soldered on:

![Assembled 4.3 inch screen board](/images/mipi-dsi-breakout/tst043-assembled.jpg?raw=true)

This picture shows 4 copies of the boards with all circuits populated with all components. Note that only the six middle signals are mandatory, with the 4 other signals, INT, RESET, TEO, and 1.8FS left unconnected. Though they can certainly be wired up later if debugging makes it necessary.

![Assembled MIPI DSI breakout PCBs](/images/mipi-dsi-breakout/assembled-breakout.jpg?raw=true)

### 3D Printed Parts and Mounting Brackets

There are optional (but highly recommended) 3d printable mounting brackets for whatever your testing needs require. They consist of 6 parts (4 parts, one for each screen and 2 types of mounting bases).

You can find the files in [/mipi-dsi-breakout/mounting-brackets]:

* dsi-base-angled (a 60 degree angled holder to mount a screen to a surface via wood screws)

  ![The angled DSI base mounting bracket design](/images/mipi-dsi-breakout/dsi-base-angled.png?raw=true)

* dsi-base-upright (same as above but 90 degrees perpendicular to surface)

  ![The upright DSI base mounting bracket design](/images/mipi-dsi-breakout/dsi-base-upright.png?raw=true)

* dsi-holder-4.3inch (for the TST043WVBI-130, 4.3 inch screen)
* dsi-holder-5inch (for the ET050FW04-T, 5 inch screen without capacitive touch screen)
* dsi-holder-5inch-ctp (for the TST050HDBS-80C, 5 inch screen with capacitive touch screen)
* dsi-holder-6inch (for the ET060HD02-T, 6 inch screen)

For each of these points, there is an stl file (ready to be sliced and printed), and a f3d/f3z file that should be able to be opened in Autodesk Fusion360.

Here are pictures of the 4.3 inch screen holder (all the other ones are minimal variations on the same idea).

![4.3 inch DSI holder front](/images/mipi-dsi-breakout/4.3-inch-bracket-front.png?raw=true)
![4.3 inch DSI holder back](/images/mipi-dsi-breakout/4.3-inch-bracket-back.png?raw=true)

### Complete Component Assembly

There are several manual steps and necessary "ingredients" to fully replicate this design.

Additional parts required (beyond the 3d printed mounting bracket, assembled PCB, and screen):

1. Conductive tape (I used copper tape from Amazon, but you can use Z tape or 3M conductive double sided tape, or even aluminum foil and glue combined with kaptop or electrical tape for insultion).

1. M2.5 machine screws. Two each per screen. Specifically, [MPFX02505-100M1 from McMaster-Carr](https://www.mcmaster.com/92010A014/). These screws are 5mm in total length with M2.5 size threads (metric). The head juts out at 45 degrees from the shaft and is approximately 1.5mm tall.

1. Scotch tape for holding the screen to the mounting bracket. Can use some other light adhesive, or something more permanent if desired. In a real product, it is common to use some sort of low melting point glue or 3M double sided adhesive tape.

1. #6 x 1/2 inch flat head philips wood screws for mounting. I bought a 100 pk from Home Depot.

   ![100pk of #6 1/2 inch Wood Screws from Home Depot](/images/mipi-dsi-breakout/wood-screws.jpg?raw=true)

1. [20 cm long 22pin FPC cable (from Adafruit)](https://www.adafruit.com/product/6036)

Assembly Instructions:

1. Complete the PCB assembly for one screen design (see the above PCB Assembly section for more information).
1. Print out the appropriate dsi base and dsi holder.
1. Apply the scotch tape to the back of the screen.

   ![Assemble process](/images/mipi-dsi-breakout/bracket-assembly-1.jpg?raw=true)

1. Place the screen into the dsi holder, making sure to thread the FPC cable through the hole on the left and gently but firmly press the screen into place so the tape will stick.

1. Attach the copper tape to the SUS cable stiffener (as specified by the datasheet) and the appropriate ground pad on the PCB. Some of the datasheets do not specify any specific grounding, but I attached tape to those parts anyway out of caution. Basically, wherever there is a stainless steel or conductive cable stiffener, I connected it to ground.

   ![Assemble process](/images/mipi-dsi-breakout/bracket-assembly-2.jpg?raw=true)

1. Use the M2.5 screws to mount the PCB to the back of the mounting bracket.

1. Carefully insert all the zif cables of the DSI screen into the corresponding zif connectors on the PCB. This may be easier with the appropriate plyers or tweezers.

1. Insert one end of the 22 pin 20cm DSI cable into the bottom zif connector of the PCB. The other end goes to the Raspberry Pi breakout board, but do not connect this end yet.

1. Use the wood screws to mount the base brackets to a permanent surface.

   ![Example of mounted brackets](/images/mipi-dsi-breakout/test-setup-1.jpg?raw=true)

### Physical Test Bench Description

The final testbench consists of a 3/4 inch thick piece of plywood with the waveshare compute module io board plus (mentioned earlier) mounted to it. There are additional peripherals mounted and connected to the Raspberry Pi.

![Test setup overview](/images/mipi-dsi-breakout/test-setup-2.jpg?raw=true)

Two angled bases were mounted to the board, in addition to three upright base brackets to store screens that aren't being used. The brackets are mounted close to the DSI ports on the RPi breakout board.

On top of the angled brackets is a breadboard for external peripheral circuitry.

1. The 20cm long DSI cable is attached to the bottom zif connector of the PCB and the other end is slotted into the DISP0 or DISP1 port of the Raspberry Pi breakout board.

1. The [ap5726-lcd-backlight-driver](/ap5726-lcd-backlight-driver) and [attiny-402-i2c-pwm-generator](attiny-402-i2c-pwm-generator) designs are reproduced on the breadboard and wired up to the VCC, GND, SDA, SCL, LEDA, and LEDK pins on the PCB. Do not forget the 1 million ohm pull down resistor on the enable pin of the backlight.

   ![Fully connected peripheral circuitry](/images/mipi-dsi-breakout/test-setup-3.jpg?raw=true)

Now that everything is connected, all that is left is to set up the software and turn on the board!

## Software Setup

### OS environment

This was developed on a Raspbian OS (Debian 11/Bullseye) running on a Raspberry Pi Compute Module 3+ and [Waveshare Compute Module IO Board Plus](https://www.waveshare.com/wiki/Compute_Module_IO_Board_Plus).

### Driver development

TBD

### Usage

TBD
