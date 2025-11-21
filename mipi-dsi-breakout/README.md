# mipi-dsi-breakout 

This design is intended to interface with different MIPI DSI LCD screens and route the DSI signals to a Raspberry Pi.

## KiCAD Version

Current version: KiCAD 9.0.3

This was originally made with KiCAD 9.0.3.

## Sponsorship

The PCB fabrication for this project is sponsored by [PCBWay](https://www.pcbway.com/). PCBWay is a manufacturer of printed circuit boards and they also offer services such as PCB assembly, 3D printing plastic and metal, and much more. Their online quote system and thorough customer service made this design a breeze to order.

## Description

![Assembled MIPI DSI breakout PCBs](/images/mipi-dsi-breakout/assembled-breakout.jpg?raw=true)

When working with microcontrollers, you can buy "all-inclusive" LCD screen kits from electronics hobby stores that are basically plug and play. With this, the user experience is quite nice, needing only to plug in a video cable going from the LCD screen circuitry directly to the microcontroller. However, these kits leave little flexibilty in the physical form factor, size and type of LCD screen, and price.

This project is an attempt to go beyond the beginner LCD peripherals offered by microcontroller hobby stores and interface directly with an LCD screen using custom peripheral circuitry.

### Background and Context

TBD

### LCD Screens Overview

TBD

### MIPI DSI Interface

TBD

### Differential High Speed Signal Background

TBD

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
