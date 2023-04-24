# max98357-amp

This is a PCB for a speaker amplifier heavily based on a [product by Adafruit](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp). The PCB design is essentially exactly the same but with extra resistors wiring to bias certain input pins for convenience of testing different configurations.

## KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 6.0.7.

## Description

![Amp PCB 3d preview](/images/max98357-amp/kicad-viewer.png?raw=true)

The IC used here is a variant of the MAX98357 Audio Power Amplifier [MAX98357AETE+T](https://www.newark.com/maxim-integrated-products/max98357aete-t/audio-power-amp-d-40-to-85deg/dp/82AC2949), a 16 pin TQFN package that must be soldered using surface mount techniques. (I used a hot air gun and solder paste).

### SD\_MODE

![Amp PCB image](/images/max98357-amp/schematic.png?raw=true)

The first large modification is three jumpers with resistors to bias input voltage going into the SD\_MODE input. SD\_MODE controls whether or not the amplifier outputs a stereo average, left channel, or right channel of the input sound data. The resistor values are calculated based on 3.3V input rail. Refer to the [datasheet](https://cdn-shop.adafruit.com/product-files/3006/MAX98357A-MAX98357B.pdf) for more information.

### Differential Output Test Pins

This PCB also includes two pin holes going to the + and - outputs of the amplifier. Mainly for "test pins" so you can probe into the signal for whatever reason. Originally this was so that a 3.5mm headphone jack socket could be connected to it later.

For more detailed explanation, refer to [the blog posts here](https://forums.somethingawful.com/showthread.php?threadid=3947328&pagenumber=11#post527927218).

### Getting Volume Control to work on Raspberry Pi OS (Raspbian)

You may notice that the volume control does not actually change the volume coming out of the speaker, even after ALSA has been set up. Mute may or may not work. That is because Linux has been gradually switching over to use Pulse Audio and something broke over the years. Even the Adafruit documentation does not have a solution for this.

Luckily, I spent like two weeks trying to figure out what went wrong and I have a fix for it.

Create a file on the system called `/etc/asound.conf`:

```
pcm.speakerbonnet {
   type hw card 0
}

pcm.dmixer {
   type dmix
   ipc_key 1024
   ipc_perm 0666
   slave {
     pcm "speakerbonnet"
     period_time 0
     period_size 1024
     buffer_size 8192
     rate 48000
     channels 2
   }
}

ctl.dmixer {
    type hw card 0
}

pcm.softvol {
    type softvol
    slave.pcm "dmixer"
    control.name "PCM"
    control.card 0
}

ctl.softvol {
    type hw card 0
}

pcm.!default {
    type             plug
    slave.pcm       "softvol"
}
```

This will give you a software controlled volume on your operating system output.

Next change `/usr/share/pulseaduio/alsa-mixer/paths/analog-output.conf.common` from:

```
[Element PCM]
switch = mute
volume = merge
override-map.1 = all
override-map.2 = all-left,all-right  
```

to

```
[Element PCM]
switch = mute
volume = ignore
```

What I think this does is tell pulse to ignore the volume controls on the alsa softvol device. Then because that happened, pulse's internal software mixer kicks in.

## References/Further Reading

* [max98357 Datasheet by Analog](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX98357A-MAX98357B.pdf)  
  IC datasheet

* [Adafruit Documentation for MAX98357 I2S Class-D Mono Amp](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp/overview)  
  Extremely useful documentation for usage of this amplifier. The design in this repo is basically copied from this.

* [What is a Class-D Amplifier and What Are they Useful For?](https://www.howtogeek.com/792135/what-is-a-class-d-amplifier-and-what-are-they-useful-for/)  
  This is an informative page for briefly explaining Class-D Amps.

* [Volume Control for MAX98357 class D amp not working](https://forums.raspberrypi.com/viewtopic.php?t=341471)  
  A link to my thread on the Raspberry Pi forums where I debug why volume control wasn't working for this amplifier when using Raspberry Pi OS (Raspbian).
