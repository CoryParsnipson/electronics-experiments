# as01808ao-enclosure

This is an experiment to mount an AS01808AO micro speaker to a PCB and have 3d printed enclosure parts bolted on. It works surprisingly well.

![AS01808AO Speaker Enclosure](/images/as01808ao-enclosure/enclosure-finished.jpg?raw=true)

### KiCAD Version

Current version: KiCAD 7.0.2.

This was originally made with KiCAD 6.0.7.

## Description

The theory behind this is that speaker enclosures need very specific conditions to make the sound quality good. This enclosure is a basic attempt to create a sealed cavity behind the speaker to let the soundwaves resonate. This in theory should let low frequency signals propagate better. The bigger the cavity, the better. And the better sealed and isolated the back half is from the front, the better. In professional settings, the enclosure shape and size is carefully simulated and calculated using specialized software that models the reflection of soundwaves.

![AS01808AO Speaker Enclosure Concept](/images/as01808ao-enclosure/cad-exploded-view.png?raw=true)

The result was actually noticeable! Compared to the same speaker mounted flush to a piece of wood, the sound volume was qualitatively louder and had a more rounded propagation quality. The bass was improved as well.

AS01808AO-WP-R speaker used for this: https://www.digikey.com/en/products/detail/pui-audio-inc/AS01808AO-WP-R/5958415

Read [here](https://forums.somethingawful.com/showthread.php?threadid=3947328&pagenumber=11#post528300706) for a detailed description of the results.

## References/Further Reading

* [Why Do Loudspeakers Need Enclosures?](https://mynewmicrophone.com/why-do-loudspeakers-need-enclosures/)  
  Very good explanation of everything for beginners.

* [Fitting a Sound Box to a Sugar Cube Speaker](https://www.caledoniancouplers.com/fitting-sound-box-sugar-cube-speaker/)  
  Some examples of fitted enclosures for microspeakers.
