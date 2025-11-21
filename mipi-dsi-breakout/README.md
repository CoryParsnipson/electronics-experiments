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

On the right, you can see an illustration for an LCD screen (imaging the OSOYOO 5 inch DSI screen).

Described in 4 numbered functional blocks are the peripheral circuitry required to connect the LCD screen to the microcontroller.

Assuming that everything is built and working, we should be able to see the operating system graphical output on the LCD screen as if we were using it as a computer monitor. Getting this working is the goal of this project.

### LCD Screens Overview

The LCD screens this project is concerned with are mostly *Thin Film Transistor* (TFT) screens. See [this page](https://www.topwaydisplay.com/en/blog/how-tft-lcd-works-explained) to learn more about how TFT screens work. Essentially, TFT LCDs are a VLSI design that prints individual transistors on a transparent substrate that act as individual pixels. So the takeaway here is that it is not very simple or inexpensive to produce these, and that there are very expensive foundries needed to manufacturer these like one would need to make CPUs and other integrated circuits, though a TFT screen does not require as small transistors nor is the design as complex as a CPU.

![LCD screen types](/images/mipi-dsi-breakout/lcd-screen-types.jpg?raw=true)
*Left: segmented (monochrome, big segments) Middle: dot matrix (monochrome, medium resolution) Right: TFT (graphical, transistors, RGB color)*

When searching for LCD screens one must be careful to specify which kind, between a segmented display, dot matrix, or a fully graphical screen. Here we want graphical displays.

Within the graphical LCD displays are Twisted Neumatic (TN) and In-Plane Switching (IPS) type displays with the difference being that IPS displays are more vibrant, brighter, and have a better viewing angle range than TN displays, which are older and cheaper than IPS displays.

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

This might be the most popular screen interface. With an HDMI controller, the screen hooks up to a host using an HDMI cable. This makes it compatible with anything that supports the HDMI protocol, and not just a specific device. It also supports audio and input signals as well. HDMI circuitry is relatively complicated, however, so it was not considered for this project.

#### MIPI DSI

MIPI DSI stands for *Mobile Industry Processor Interface Display Serial Interface*. For this project specifically, we have chosen to look for DSI screens exclusively because the DSI interface is convenient for Raspberry Pi microcontrollers. Having a DSI screen uses less power, supports higher resolutions and refresh rates, low physical profile, and does not take up GPIO or other ports that could be used for other things. The DSI protocol is also meant for internal connectors for mobile and low power and physical small devices.

#### LVDS

"Low Voltage Differential Signaling" actually refers to the transmission method and not an entire protocol, so it is not quite equivelant to other things on this list. In fact, MIPI DSI is a protocol that sits on top of LVDS. However, you may see this and "Enhanced LVDS" (eLVDS) when searching for screens and this just means that it uses LVDS and then a proprietary protocol on top to transmit data. This is common for larger screens such as for laptops and tablets, and only found buried in commercially manufactured devices. It is generally not recommended to use a screen that claims to use LVDS because it is hard to reverse engineer or find documentation on proprietary protocols, especially since they would be completely different across screens.

#### SPI

Serial Peripheral Interface (SPI) is a protocol used for many devices, not just screens. This is common for graphical LCD screens 3-4 inches and smaller, since the maximum data rate for SPI is not very fast. Even then, you may observe that the framerate is not very high due to the transfer rate limitations. Furthermore, an SPI screen will probably require more CPU overhead and power because the protocol is not designed specifically to transmit video. One good thing is that the interface and protocol is very simple, so it is very easy to integrate into a device as long as the screen is small and the framerate and resolution is not too large.

SPI devices usually hook in through GPIO pins.

#### RGB/MCU/Parallel/DPI

Display Parallel Interface (DPI) is similar to SPI but is faster so supports higher resolutions. It still suffers from similar power and CPU overhead limitations. It also uses GPIO pins, but a very large amount. For example, in a Raspberry Pi, hooking up a DPI screen may use almost all of the 40 available GPIO pins. This is not convenient if one wants to integrate multiple peripherals to the same microcontroller.

### Differential High Speed Signal Background

MIPI DSI protocol relies on high speed differential pair signals. High speed, meaning signals that switch with frequency higher than 50 MHz. For the DSI protocol, it can reach speeds up to 480 MHz, for higher resolutions and refresh speeds. In this project, we are targeting screens around 5 inches in size and slightly lower than HD resolution (480x800 or 1280x1024), so speeds will not be as high, but around the 50 - 100 MHz range.

The term differential pair refers to the fact that each signal is split into two complementary signals that are transmitted together. In other words, when one signal is high, the other signal is low. The destination device will receive both signals and take the difference between their values to recover the original signals.

![Illustration of Differential pair signals](/images/mipi-dsi-breakout/differential-pair-signals.png?raw=true)

Doing things this way allows one to transmit at lower voltages since the amplitude of each individual signal can be half the maximum value required, and also allows for higher data rates, and protection against electromagnetic interference (EMI).

The tradeoff is that it requires more board space for extra traces, specific geometric and PCB design constraints to accomodate the differential pair layout, and protocol overhead to recover the original signal.

What is of particular interest for this project are all the ways including differential pairs change the PCB layout constraints compared to "ordinary" non-high speed and non-differential signals. A design that is meant to include differential pairs must be done at the very start, because it involves all aspects of the board and layout.

#### How to Route Differential Signals in KiCAD

TBD (link to youtube video tutorial about differential pair signal routing)

#### Differential Pair Signals Background

TBD (link video about differential pair ground plane coupling and stuff)

#### PCB Considerations when using Differential Pair Signals

TBD (link to Sierra Circuits impedance calculator and PCBway 4 layer stackup)

## Part Selection

### LCD Screen Parts

TBD

### Raspberry Pi Host Motherboard

TBD

### Zif Connector Selection

TBD

### Peripheral Electronics

TBD (describe Attiny PWM microcontroller and ap5762 backlight controller). These circuits hook into this board using 6 wide pin header.

## PCB Layout

TBD

## PCB assembly

It is highly recommended to use a hot plate or reflow oven for ease of use. A hot air rework station may work as well, but will be more difficult especially for soldering the wider, higher-pin connectors.

TBD

## Software Setup

### OS environment

This was developed on a Raspbian OS (Debian 11/Bullseye) running on a Raspberry Pi Compute Module 3+ and [Waveshare Compute Module IO Board Plus](https://www.waveshare.com/wiki/Compute_Module_IO_Board_Plus).

### Driver development

TBD

### Usage?

TBD
