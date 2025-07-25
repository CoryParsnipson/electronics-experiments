# mipi-dsi-breakout 

This contains multiple pcb designs for a MIPI DSI LCD screen test fixture. This project allows you to take "raw" LCD screen modules with MIPI DSI interfaces and connect them to a somewhat general purpose breakout board to interface with a microcontroller. The aim is to help prototype screens and capacitive touch screens before committing toadding these devices to your final PCB design.

## KiCAD Version

Current version: KiCAD 9.0.3

This was originally made with KiCAD 9.0.3.

## Description

This design optimizes for flexibility and ease of swapping out different screen modules for quick comparison between different parts.

### Project Layout

Unlike simpler designs in this repository, this folder contains multiple different KiCAD project (each project should correspond to 1 PCB design).

1. host-side-interface - connects to microcontroller via 22 pin FPC cable. (Designed for the Waveshare RPi CM3 I/O board plus DSI connectors)

1. lcd-side-interface - connects to the LCD screen module

1. attiny-pogo-pin-conn - breakout board with pogo pins to let you program the ATTiny402 on the host-side-interface board. This is an experiment to see if this is a convenient way to program the ATTiny402 *in situ*.

The host-side-interface and lcd-side-interface boards connect to each other via pin header. While there are high speed differential signals on the boards, pin header was used to make it easier to swap out the lcd-side-interfaces for prototyping. It is assumed that the integrity of the signals should not suffer too much with the pin headers in place. Also, since this is a prototype, the absolute cleanliness of the signal should not be a concern. That should be worked out when a part is chosen and incorporated into the final design.

## TBD

The rest of this document is pending.
