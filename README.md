# ProtonPack

## Introduction

So my kids wanted to be Ghostbusters for Halloween.  So we built some proton-packs out of bits of cardboard, PVC pipe, wires, black spray paint and some old backpack straps hot glued on.  Then they wanted them to look "just like the ones in the movie" with the LED "power" indicators that pulsed up and down.  So here we go ...

In the end, I used a Atmel ATTiny 2313 with 10 LEDs hooked up to the GPIO pins.  A voltage regulator/smoothing caps provide power, a push-button switch cycles the "display".  Overkill?  probably, but it won the best costume at Pedro's Judo's Halloween party!

## Building

You'll need the `avr-gcc` tools package installed, run `make` to build and then flash to your ATtiny with your favorite ISP programmer.  I used Adafruit's [USBtinyISP](http://www.adafruit.com/products/46) programmer.

## Notes

Since the ATtiny doesn't have sin/cosine etc, the waveform is driven by static data which was generated using the `sin_wave.rb` script.  You can cut and paste the resulting data into the `test.c` file for testing.

