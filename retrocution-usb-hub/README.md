# retrocution-usb-hub

The usb hub design uses an FE1.1s integrated USB 2.0 controller with the board layout heavily referenced from [Retrocution](https://www.retrocution.com/2020/01/15/easy-diy-tiny-usb-hub-for-raspberry-pi-projects/). Many thanks to Retrocution!

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 5.

## Description

![USB Hub With Enclosure](/images/retrocution-usb-hub/usb-hub-with-enclosure.jpg?raw=true)

This project was to learn how to create a PCB using KiCAD. The USB hub project was chosen because it was a good balance of simple and complex and produced a useful item at the end. This pcb actually contains three designs that were separated by mouse bites and edge cuts.

![Schematic](/images/retrocution-usb-hub/usb-hub-schematic.png?raw=true)

![Bare PCB](/images/retrocution-usb-hub/bare-pcbs.jpg?raw=true)

### USB Hub Design

This design uses the FE1.1s integrated chip and the design is heavily inspired by retrocution.

#### CAUTION: USB ports symbols have pins that are mirrored inadvertently!

It was found out in the assembly phase that the symbols for the USB sockets have pins that are in the wrong orientation. This means that the USB sockets must be wired to the *back* of the pcb (on the opposite side of the surface mount components) for it to work properly. Not doing this will result in the USB ports shorting VCC to ground and that will damage the host computer if power surge protection is not present on the host machine ports.

![USB Hub Plugged In](/images/retrocution-usb-hub/hub-plugged-in.jpg?raw=true)

This was... quite a learning experience.

#### CAUTION: This usb hub requires a male USB A to USB A cable.

For those who don't know, a male USB A to A cable is really dangerous. Because many USB devices are designed to be hosts, plugging two hosts together with this type of cable may do damaged to one or both devices. Most cables have a female and male side, for good reason. There is rarely a good reason to have this type of cable around.

This was another learning experience... The board was designed and the upstream USB port was made to be a female socket when it should have been a male socket.

### DPad Tactile Switch Design

This is the second design (lower left on the schematic pictured above). This is a pcb footprint for game controllers that spread out four tactile switches in a cross pattern for the directional pad. The two sides of the switches are broken out into pin header at the side so they can be wired to GPIO of a microcontroller. The dimensions of this dpad layout is taken from the New Nintendo 3Ds and is similar (but slightly smaller than) that of the Nintendo Switch.

![Tactile ABXY Button Cross](/images/retrocution-usb-hub/tactile-abxy.jpg?raw=true)

The tactile switches in this design were [Omron B3F-1000](https://www.digikey.com/en/products/detail/omron-electronics-inc-emc-div/B3F-1000/33150).

![Tactile Switch Button Cross with 3d Printed Middle](/images/retrocution-usb-hub/tactile-switch-button-cross-with-3d-printed-middle.jpg?raw=true)

This design also requires a custom designed 3d printed part that sits in the middle. You can find a Fusion 360 model of this part in `retrocution-usb-hub/lib/models/tactile-switch-abxy/abxy-pcb-jig v1.3fd`. The purpose of the 3d printed part is so that the soft silicoe membrane can rest flush to the top of the switches. It is meant for new nintendo 3ds ABXY buttons to go on top of the membrane. A properly shaped enclosure should hold everything in place.

![ABXY Enclosure 1](/images/retrocution-usb-hub/abxy-enclosure-1.jpg?raw=true)

![ABXY Enclosure 2](/images/retrocution-usb-hub/abxy-enclosure-2.jpg?raw=true)

Example pictures. Models are not included in this repo.

### DPad Copper Footprint Design

![DPad Board Assembled](/images/retrocution-usb-hub/footprint-dpad.jpg?raw=true)

The third design, imo, is most interesting. This is a footprint for traces that can interface with "metal dome switches" for compact controller buttons. (See References for more information).

This requires many parts in addition to just the PCB:

* New 3DS Metal Dome Switch Membrane (https://www.aliexpress.us/item/2251832833816917.html)
* New 3DS Replacement DPad (https://www.aliexpress.us/item/3256802328844124.html)
* New 3DS Silicone Membrane (https://www.aliexpress.us/item/2251832866142244.html)
* Right Angle male pin header

![Controller Jigs](/images/retrocution-usb-hub/controller-jigs.jpg?raw=true)

Despite all the window dressing, the point of this project is to see whether or not one could reproduce the copper pcb trace to interface with the metal dome switches. The footprint consists of a ring with a rectangular protrusion on the side that is a custom imported SVG image made in a graphics program like Inkscape. The second half of the pin is a smaller round pad in the center. In the outer footprint, this center pad is connected to the back copper using a via and all the back traces are connected together in a diamond shape and then routed to the pin header as pin #5. And, obviously, the precise sizes of the middle circles and rings and their relative spacing is very important to get correct to fit to the 3ds dpad.

## References/Further Reading

* [A Quick Guide to Tactile Metal Dome Switches](https://www.snaptron.com/2022/12/quick-guide-to-tactile-metal-domes-switches-snaptron/)  
  Metal dome switches are used to make compact electronics with nice buttons. This is especially common in portable game consoles and game controllers. The nintendo 3ds uses metal dome switches embedded in a sticker that is adhered to a trace on a PCB.

* [Circular pad in KiCAD](https://forum.kicad.info/t/circular-pad-in-kicad/12689)  
  This is a really useful stackexchange question about how to make a custom pin footprint. This is specifically about how to make a pin that resembles the trace you need to make contact with a "metal dome switch". This advice was used to make a working dpad footprint.
