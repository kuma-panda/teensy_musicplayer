# teensy_musicplayer

## Overview

This is [Teensy4.1](https://www.pjrc.com/teensy/ "PJRC Homepage") based music player gadget.  

## Features

- All C++ codes were writtern for [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html "Teensyduino").
- Music source (.mp3 or .aac) can be provided via Teensy4.1's build-in SD card (up to 32GByte).    
    Standard 44.1kHz/16bit source is available and supports bitrate up to 320kbps (No high-res source available). 
- I2S stereo sound output is available. PCM5102A is suitable for DAC.
- Implements resistive touch display (480 * 320 pixles, 16bit color) that displays track information, album coverart image, artist thumbnail image, spectrum animation etc.  
    Japanese text can be displayed. All text is displayed using beautiful, anti-aliased, proportional font.

