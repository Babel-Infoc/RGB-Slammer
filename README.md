<p align="center">
  <a href="" rel="noopener">
 <img width=200px height=200px src="https://avatars.githubusercontent.com/u/129355944?v=4" alt="Bot logo"></a>
</p>

# <p align="center">RGB Slammer</p>
<p align="center">Run RGB LED animations on fuckin anything</p>

## About <a name = "about"></a>

This is my solution to getting RGB animations and DC motor control on CH32V003 with the Arduino IDE, but you can use this to run RGB LEDs on any chip that has GPIO pins without dependencies or other libraries.

Supports **3 independent RGB LED channels** and **bidirectional DC motor control** with synchronized animations.

 ## Features <a name = "features"></a>
- ### Triple LED Channel Support
    Three independent RGB LED channels with alternating color patterns (Channel 1: color1, Channel 2: color2, Channel 3: color1)<br>
    All channels run synchronized animations with gamma correction and luminance compensation
- ### Bidirectional DC Motor Control
    Two-pin micro motor control without H-bridge (polarity swap direction control)<br>
    Synchronized motor animations with LED effects including:
    - **Auto-focus mode**: Random direction changes simulating camera focus hunting
    - **Wiggle animation**: Rapid back-and-forth movements (60% chance every 800ms)
    - **Smart weighting**: 80% motor OFF, 10% forward, 10% reverse for natural behavior
    - Full digital speed control on any GPIO pins
- ### VSCode Color Decorator compatibility
    Allows VSCode to display RGB values as colored squares, allowing for easy visual selection and adjustment<br>
    RGB Strings are automatically converted to RGB values by the `#define rgb(r, g, b) {r, g, b}` macro
- ### Button selection for active Animation and Color swatch
    Two buttons are configured to swap between animations and color swatches on the fly<br>
    Hold color button for 2 seconds to enter brightness adjustment mode
- ### Flash Memory Storage
    Saves and restores settings across power cycles:
    - Current color swatch selection
    - Brightness level
    - Active animation mode
- ### Swatch arrays
    Iterate or randomize animation colors through customizable and expandable swatches<br>Color swatches are configured with a standard structure:
    ```cpp
    swatch[0].primary;
    swatch[0].accent;
    swatch[0].midtone;
    swatch[0].contrast;
    swatch[0].background;
    ```
    Animations automatically use appropriate swatch colors<br>Create or modify swatches in `swatches.cpp`
- ### Automatic color tuning
    All RGB LEDs have slightly different perceived brightness between red, green, and blue elements when provided the same power, which can affect white tone and color accuracy. Automatic color tuning is achieved by entering the typical mA and Luminance value for `red`, `green`, and `blue` according to your LED's datasheet.
- ### Gamma correction
    Fix the perceived LED brightness curve using <a href="https://learn.adafruit.com/led-tricks-gamma-correction/">Phillip Burgess' fix</a>
- ### Triple simultaneous output
    All three LED channels can be configured for separate colors and animations, running simultaneously with coordinated effects
- ### Configurable brightness modifier
    Hold the color button for 2 seconds to enter brightness adjustment mode<br>
    Brightness cycles through range (30%-60%) - release when desired brightness is reached<br>
    Settings are automatically saved to flash memory
- ### Handover color system
    Animation subroutines are designed to fade in or interact with the last color displayed by the previous subroutine<br>
    All subroutines store their last RGB color in segment-specific variables called `handoverColor`<br>
    Creates smooth transitions between different animation modes

Switch configurations by changing `activeConfig` in the main sketch.

## Motor Control
Motor uses direct pin polarity control (no PWM needed):
- **Forward**: PinA=HIGH, PinB=LOW
- **Reverse**: PinA=LOW, PinB=HIGH
- **Off**: Both pins LOW
- **WARNING**: Never set both pins HIGH simultaneously (creates short circuit)

Available motor functions:
```cpp
motorForward();           // Full speed forward
motorReverse();           // Full speed reverse
motorOff();              // Stop motor
motorForwardSpeed(speed); // Forward with speed (currently full speed on non-PWM pins)
motorReverseSpeed(speed); // Reverse with speed (currently full speed on non-PWM pins)
```

## Usage
1. Select your hardware configuration in `RGB-Slammer.ino`
2. Customize color swatches in `swatches.cpp`
3. Adjust brightness range in global settings (30%-60% default)
4. Upload to your Arduino-compatible microcontroller

**Button Controls:**
- Short press color button: Cycle through color swatches
- Short press animation button: Cycle through animation modes
- Long press color button (2s): Enter brightness adjustment mode

## Reason for development
Originally developed to run LED animations on microcontrollers without library dependencies. The script uses pure `digitalWrite` to create PWM on standard GPIO pins, making it compatible with any Arduino-compatible chip.

Now expanded to include synchronized DC motor control for kinetic light installations, camera rig animations, and interactive art projects.
