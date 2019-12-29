# Hap-eRanger - Haptic Electronic Ranger (Infrared TOF)
### Touch-free visual assistance
### "Happy ranging"
Support canes used by the blind have a major drawback in that
they can whack people from behind unintentionally.  This is clearly
awkward for both the blind and the sighted.

The Hap-eRanger uses a Class 1 (safe to the naked eye) 
IR Time-of-flight (TOF) laser to detect near objects.
Tactile feedback on the range of detected objects is provided by
a Linear Resonant Actuator (LRA) haptic device.

The Hap-eRanger uses [TinyCircuits](https://tinycircuits.com) hardware
and is implemented with the [[OMilli]] multi-threaded operating system
for microcontrollers.

* [TinyZero MCU](https://tinycircuits.com/products/tinyzero-processor)
* [TinyCircuits TOF distance sensor Wireling](https://tinycircuits.com/products/tof-distance-sensor-wireling-vl53l0x)
* [TinyCircuits LRA Driver](https://tinycircuits.com/products/lra-wireling-drv2605)

### Important!

* Do not aim the wand at faces or eyes. The Class 1 laser is rated as eye-safe 
  under normal operating conditions, but close visual inspection with naked eye
  or microscope may prove hazardous.
* Do not expose to water. The wand is not water resistant.
* Do not drop. The wand has fragile electronic components.
* Not for outdoor use. It's too bright for the wand.
* Ranging sensitivity varies by material sensed. Weakly reflective surfaces
  such as glass may not be detectable at shallow angles.
  Non-reflective surfaces such as 
  [Vantablack](https://en.wikipedia.org/wiki/Vantablack) will not be 
  detected.
* Thin objects may not be detectable. E.g., chair seats are 
  much more detectable than chair legs. 
