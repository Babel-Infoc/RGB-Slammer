<p align="center">
  <a href="" rel="noopener">
 <img width=200px height=200px src="https://avatars.githubusercontent.com/u/129355944?v=4" alt="Bot logo"></a>
</p>

# <p align="center">RGB Slammer</p>
<p align="center">Run RGB LED animations on fuckin anything</p>

## About <a name = "about"></a>

This is my solution to getting RGB animations on the CH32V003 with the Arduino IDE, but you can use this to run RGB LEDs on any chip that has 3 GPIO pins without dependancies or other libraries.

 ## Features <a name = "features"></a>
- ### Colors stored as RGB Strings
    Makes the script compatible with the <a href="https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token">json-color-token</a> plugin (see Useful tools below)
- ### Multiple simultaneous LED segments
    Can be configured for separate colors and animations, which will all run simultaneously, no `delay` pauses
- ### Automatic color tuning
    All RGB LEDs's have slightly different percieved brightness and between the red green and blue elements when provided the same power which can affect the white tone and colour accuracy. Automatic color tuning is achieved by entering the typical mA and Luminance value for redLum, greenLum, and blueLum according to your LED's datasheet.
- ### Gamma correction
    Fix the percieved LED brightness curve using <a href="https://learn.adafruit.com/led-tricks-gamma-correction/">Phillip Burgess' fix</a>
- ### Preconfigured animations
    Included is a number of preconfigured animation elements and longer animation cycles, which are easily configured using main loop arguments
- ### Swatch arrays
    Iterate or randomize animation colors through customisable swatches. Create or modify swatches in the swatches.h header file.
- ### Max output brightness
    `maxBrightness` float that can be configured between 0 and 1
- ### Ambient brightness
    (feature in progress)
    `ambientBrightness` float that can be modified by a light sensor if you have one
- ### Button input color / animation
    (feature in progress)
- ### Handover color
    Some animation subroutines are designed to fade in or interact with the last color displayed by the previous subroutine, as such all subroutines store their last RGB color in segment specific variables called handoverColor

## Useful tools (optional)
Edit the script in VSCode, and install the <a href="https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token">json-color-token</a> plugin to display an interactive colored square next to each RGB color value, allowing for very easy color selection and adjustment with visual representation.
Color strings are automatically converted to RGB values by the `rgbStringToArray` subroutine

## Reason for development
I wanted to run LED animations off the CH32V03.
Currently, neither the Neopixel or FastLED libraries support RISK-V chips, and WCH Board drivers have had bugs with AnalogWrite functions for nearly a year.

Due to these limitations, I wanted a way to drive RGB animations using purely digitalWrite, to slam PWM on standard GPIO pins, without using any library dependancies. Therefore, this script can be used on any chip that can be proframmed via the ArduinoIDE, as long as you have at least 3 GPIO pins to assign to RGB.
