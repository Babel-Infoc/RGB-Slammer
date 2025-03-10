<p align="center">
  <a href="" rel="noopener">
 <img width=200px height=200px src="https://avatars.githubusercontent.com/u/129355944?v=4" alt="Bot logo"></a>
</p>

# <p align="center">RGB Slammer</p>
<p align="center">Run RGB LED animations on fuckin anything</p>

## About <a name = "about"></a>

This is my solution to getting RGB animations on the CH32V003 with the Arduino IDE, but you can use this to run RGB LEDs on any chip that has 3 GPIO pins without dependancies or other libraries.

 ## Features <a name = "features"></a>
- ### VSCode Color Decorator compatibility
    Allows VSCode to display RGB values as colored squares, allowing for easy visual selection and adjustment<br>
    RGB Strings are automatically converted to RGB values by the `#define rgb(r, g, b) {r, g, b}` macro
- ### Button selection for active Animation and Color swatch
    Two buttons are configured to swap between animations and color swatches on the fly
- ### Swatch arrays
    Iterate or randomize animation colors through customisable and expandable swatches<br>Color swatches are configured with a standard structure:
    ```cpp
    swArr[0].highlight;
    swArr[0].primary;
    swArr[0].accent;
    swArr[0].background;
    ```
    Use `[0]` or `[1]` to set the color for the first or second led segment<br>Create or modify swatches in `swatches.cpp`
- ### Automatic color tuning
    All RGB LEDs's have slightly different percieved brightness and between the red green and blue elements when provided the same power, which can affect the white tone and colour accuracy. Automatic color tuning is achieved by entering the typical mA and Luminance value for `red`, `green`, and `blue` according to your LED's datasheet.
- ### Gamma correction
    Fix the percieved LED brightness curve using <a href="https://learn.adafruit.com/led-tricks-gamma-correction/">Phillip Burgess' fix</a>
- ### Dual simultaneous output
    Both segments can be configured for separate colors and animations, which will all run simultaneously
- ### Preconfigured animations
    Included is a number of preconfigured animation elements and longer animation cycles, which are easily configured using main loop arguments
- ### Max output brightness
    `maxBrightness` float that can be configured between 0 and 1
- ### Ambient brightness
    (feature in progress)
    `ambientBrightness` float that can be modified by a light sensor if you have one
- ### Handover color
    Some animation subroutines are designed to fade in or interact with the last color displayed by the previous subroutine, as such all subroutines store their last RGB color in segment specific variables called handoverColor

## Reason for development
I wanted to run LED animations off the CH32V03.
Currently, neither the Neopixel or FastLED libraries support RISK-V chips, and WCH Board drivers have had bugs with AnalogWrite functions for nearly a year.

Due to these limitations, I wanted a way to drive RGB animations using purely digitalWrite, to slam PWM on standard GPIO pins, without using any library dependancies. Therefore, this script can be used on any chip that can be proframmed via the ArduinoIDE, as long as you have at least 3 GPIO pins to assign to RGB.
