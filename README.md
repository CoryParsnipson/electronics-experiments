# electronics-experiments

Educational and experimental PCB designs for Raspberry pi related products and peripherals

## Project List

### max98357-amp

![Amp PCB 3d preview](https://github.com/CoryParsnipson/electronics-experiments/blog/main/max98357-amp-1.png?raw=true)

![Amp PCB image](https://github.com/CoryParsnipson/electronics-experiments/blog/main/max98357-amp-2.png?raw=true)

This is a PCB for a speaker amplifier heavily based on a [product by Adafruit](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp). The PCB design is essentially exactly the same but with extra resistors wiring to bias certain input pins for convenience of testing different configurations.

The IC used here is a variant of the MAX98357 Audio Power Amplifier [MAX98357AETE+T](https://www.newark.com/maxim-integrated-products/max98357aete-t/audio-power-amp-d-40-to-85deg/dp/82AC2949), a 16 pin TQFN package that must be soldered using surface mount techniques. (I used a hot air gun and solder paste).

#### SD\_MODE

The first large modification is three jumpers with resistors to bias input voltage going into the SD\_MODE input. SD\_MODE controls whether or not the amplifier outputs a stereo average, left channel, or right channel of the input sound data. The resistor values are calculated based on 3.3V input rail. Refer to the [datasheet](https://cdn-shop.adafruit.com/product-files/3006/MAX98357A-MAX98357B.pdf) for more information.

#### Differential Output Test Pins

This PCB also includes two pin holes going to the + and - outputs of the amplifier. Mainly for "test pins" so you can probe into the signal for whatever reason. Originally this was so that a 3.5mm headphone jack socket could be connected to it later.

For more detailed explanation, refer to [the blog posts here](https://forums.somethingawful.com/showthread.php?threadid=3947328&pagenumber=11#post527927218).

### as01808ao-enclosure

![AS01808AO Speaker Enclosure Concept](https://github.com/CoryParsnipson/electronics-experiments/blog/main/as01808ao-enclosure-1.png?raw=true)

![AS01808AO Speaker Enclosure](https://github.com/CoryParsnipson/electronics-experiments/blog/main/as01808ao-enclosure-2.jpg?raw=true)

This is an experiment to mount an AS01808AO micro speaker to a PCB and have 3d printed enclosure parts bolted on. It works surprisingly well.

The theory behind this is that speaker enclosures need very specific conditions to make the sound quality good. This enclosure is a basic attempt to create a sealed cavity behind the speaker to let the soundwaves resonate. This in theory should let low frequency signals propagate better. The bigger the cavity, the better. And the better sealed and isolated the back half is from the front, the better. In professional settings, the enclosure shape and size is carefully simulated and calculated using specialized software that models the reflection of soundwaves.

The result was actually noticeable! Compared to the same speaker mounted flush to a piece of wood, the sound volume was qualitatively louder and had a more rounded propagation quality. The bass was improved as well.

AS01808AO-WP-R speaker used for this: https://www.digikey.com/en/products/detail/pui-audio-inc/AS01808AO-WP-R/5958415

Read [here](https://forums.somethingawful.com/showthread.php?threadid=3947328&pagenumber=11#post528300706) for a detailed description of the results.
