/*
RGB Slammer
Written by Tully Jagoe 2025
MIT License

This script is best edited in VSCode, using the following extension:
https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token
*/

// MARK: ------------------------------ Variables and config ------------------------------
#include <Arduino.h>
#include "types.h"
#include "swatches.h"

// Define numLEDs here instead of in types.h to avoid multiple definitions
const uint8_t numLEDs = 2;

// The `led` array contains the pin configurations for different LEDs.
// Each element in the array represents a different LED with its associated pins.
ledSegment led[2] = {
    {PD5, PD3, PD4},
    {PC5, PC6, PC4}
};

// Define the pins for the animation and color select buttons
const uint8_t colorBtn = PC3;
const uint8_t animBtn = PC0;

// Maximum brightness modifier, 0-100
const float maxBrightness = 0.6;

// ToDo: Add an midtone brightness modifier

// Slow down all animations by this amount (in milliseconds)
const uint8_t slowDown = 1;

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
const luminance red       = {8, 52};
const luminance green     = {5, 163};
const luminance blue      = {3, 18};

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    // Set up all LED segments
    for (uint8_t segment = 0; segment < 2; segment++) {
        pinMode(led[segment].red, OUTPUT);
        pinMode(led[segment].green, OUTPUT);
        pinMode(led[segment].blue, OUTPUT);
    }
    // Set up the color and animation buttons
    pinMode(colorBtn, INPUT_PULLUP);
    pinMode(animBtn, INPUT_PULLUP);

    // Calculate the luminosity modifiers
    calculateLuminance();

    // Set up random seeds
    float randSeed1(analogRead(0));
    float randSeed2(analogRead(1));

    // Show the bootup animation
    bounceBoot(50);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/

// How many animations in the loop
extern const uint8_t numAnimations = 7;

void loop() {
    switch (animIndex) {
        case 0: flicker(50, 150); break;
        //case 0: glitchLoop(70, 10, 1000); break;
        case 1: progressiveFade1(5000); break;
        case 2: gradientWave(5000); break;
        case 3: progressiveFade2(700); break;
        case 4: progressiveFade3(900); break;
        case 5: randomFade(20, 2000); break;
        case 6: pulseColor(2500); break;
        case 7: bounceBoot(50); break;
        case 8: holdYourColorPrimary(); break;
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: pulseColor -------------------------------------------------------------------------------------------------
// Pulses from primary through to background colors in the active swatch, over <speed> milliseconds
void pulseColor(const int duration){
    fadeToColor(swatch[swNum].primary,      swatch[swNum].primary,      (duration/2));
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       (duration));
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      (duration));
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     (duration));
    fadeToColor(swatch[swNum].primary,      swatch[swNum].primary,      (duration/2));
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       (duration));
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      (duration*2));
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     (duration*4));
    fadeToColor(swatch[swNum].background,   swatch[swNum].background,   (duration*6));
}

// MARK: progressiveFade --------------------------------------------------------------------------------------------
// Fades through all colors in the active swatch in order, over <duration> milliseconds
void progressiveFade1(const int duration) {
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     duration);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      duration);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       duration);
    fadeToColor(swatch[swNum].primary,      swatch[swNum].primary,      duration);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       duration);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      duration);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     duration);
    fadeToColor(swatch[swNum].background,   swatch[swNum].background,   duration);
}

void progressiveFade2(const int duration) {
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].accent,       duration);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      duration);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].contrast,     duration);
    fadeToColor(swatch[swNum].primary,      swatch[swNum].background,   duration);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].contrast,     duration);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      duration);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].accent,       duration);
    fadeToColor(swatch[swNum].background,   swatch[swNum].primary,      duration);
}

void progressiveFade3(const int duration) {
    fadeToColor(swatch[swNum].primary,      swatch[swNum].background,   duration);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].primary,      duration);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].accent,       duration);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].midtone,      duration);
    fadeToColor(swatch[swNum].background,   swatch[swNum].contrast,     duration);
}

// MARK: randomFade -------------------------------------------------------------------------------------------------
// Fades between random colors in the active swatch, each fade for a random speed defined by <min> and <max>
void randomFade(const int min, const int max) {
    // Get random color indices (0-3) for each LED channel
    uint8_t color1 = random(0, swatchSize);
    uint8_t color2 = random(0, swatchSize);

    // Arrays to hold the selected colors for each LED
    uint8_t* firstColor;
    uint8_t* secondColor;

    // Select the first color based on the random index
    switch (color1) {
        case 0: firstColor = swatch[swNum].primary;     break;
        case 1: firstColor = swatch[swNum].accent;      break;
        case 2: firstColor = swatch[swNum].midtone;     break;
        case 3: firstColor = swatch[swNum].contrast;    break;
        case 4: firstColor = swatch[swNum].background;  break;
    }

    // Select the second color based on the random index
    switch (color2) {
        case 0: secondColor = swatch[swNum].primary;    break;
        case 1: secondColor = swatch[swNum].accent;     break;
        case 2: secondColor = swatch[swNum].midtone;    break;
        case 3: secondColor = swatch[swNum].contrast;   break;
        case 4: secondColor = swatch[swNum].background; break;
    }

    // Fade between the two random colors with a random duration
    fadeToColor(firstColor, secondColor, random(min, max));
}

// MARK: glitchLoop ------------------------------------------------------------------------------------------------
// Advanced neon flicker with 3 different animation patterns selected randomly
void glitchLoop(const uint8_t flickerChance, const uint8_t effectChance, const uint8_t duration) {
    // For <duration> milliseconds, both LED segments will either play a special animation or the normal flicker
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    bool effectTrigger = random(0, 100) < effectChance;
    while (currentTime - startTime < duration) {
        if (effectTrigger) {
            // Apply a special effect
            uint8_t flickerSegment = random(0, numLEDs);
            // Pick a random glitch effect
            switch (random(0, 3)) {
                case 0:
                    glitch1(flickerSegment);
                    break;
                case 1:
                    glitch2(swatch[swNum].midtone ,swatch[swNum].contrast, flickerSegment);
                    break;
                case 2:
                    glitch3(flickerSegment, swatch[swNum].primary, 20, 3);
                    break;
            }
            currentTime = millis();
        } else {
            // Normal flicker on both segments
            flicker(flickerChance, 0.2);
            currentTime = millis();
        }
    }
}

// MARK: bounceBoot ----------------------------------------------------------------------------------------------------
void bounceBoot(uint8_t speed){
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = speed * 3;
        fadeToColor(swatch[swNum].primary,      swatch[swNum].background,   speed);
        fadeToColor(swatch[swNum].accent,       swatch[swNum].primary,      speed);
        fadeToColor(swatch[swNum].midtone,      swatch[swNum].accent,       speed);
        fadeToColor(swatch[swNum].contrast,     swatch[swNum].midtone,      speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].contrast,     speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].background,   speed);
    }
    showColor(swatch[swNum].background, swatch[swNum].background, speed*5);
}

// MARK: holdYourColor ------------------------------------------------------------------------------------------------
// Shows the accent color only
void holdYourColorPrimary(){
    showColor(swatch[swNum].accent, swatch[swNum].accent, 100);
}

// MARK: --- Animation processor
// MARK: fadeToColor ------------------------------------------------------------------------------------------------
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const int fadeTime){
    uint8_t startColor[2][3];
    uint8_t output[2][3];

    // Copy handoverColor to startColor
    for (int segment = 0; segment < numLEDs; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            startColor[segment][pin] = handoverColor[segment][pin];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
            for (int pin = 0; pin < 3; pin++) {
                output[0][pin] = startColor[0][pin] + (color1[pin] - startColor[0][pin]) * fadeRatio;
                output[1][pin] = startColor[1][pin] + (color2[pin] - startColor[1][pin]) * fadeRatio;
            }
        sendToRGB(0, output[0]);
        sendToRGB(1, output[1]);
    }
}

// MARK: showColor -----------------------------------------------------------------------------------------------
void showColor(uint8_t color1[3], uint8_t color2[3], uint8_t duration){
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        sendToRGB(0, color1);
        sendToRGB(1, color2);
    }
}

// MARK: flicker ---------------------------------------------------------------------------------------------
void flicker(const uint8_t chance, const uint8_t intensity){
    uint8_t outputColor[3];
    for (uint8_t segment = 0; segment < numLEDs; segment++) {
        uint8_t flickerIntensity = random(intensity, 255);
        gradientPosition(flickerIntensity, outputColor);
        sendToRGB(segment, outputColor);
    }
}

// MARK: glitch1 ------------------------------------------------------------------------------------------
void glitch1(const uint8_t segment){
    uint8_t otherSegment = 0;
    uint8_t flickerTime = 50;
    if (segment == 0) {
        showColor(swatch[swNum].contrast, swatch[swNum].accent,50);
        showColor(swatch[swNum].contrast, swatch[swNum].background,50);
    } else {
        showColor(swatch[swNum].accent, swatch[swNum].contrast,50);
        showColor(swatch[swNum].background, swatch[swNum].contrast,50);
    }
}

// MARK: glitch2 ------------------------------------------------------------------------------------------
void glitch2(uint8_t color1[3], uint8_t color2[3], uint8_t duration) {
    // Part 1: Rapidly flash between black and background for 1 second
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < 500) { // 1 second flash period
        rapidPulse(color1, color2, 50);
    }

    // Part 2: Rapidly fade through all swatch colors from background to primary
    uint8_t fadeTime = 50; // Quick fade time between colors
    // Fade through the colors in sequence: background → contrast → midtone → accent → primary
    fadeToColor(swatch[swNum].background,   swatch[swNum].background,   fadeTime);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     fadeTime);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      fadeTime);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       fadeTime);
    fadeToColor(swatch[swNum].primary,      swatch[swNum].primary,      fadeTime);
}

// MARK: glitch3 ------------------------------------------------------------------------------------------
void glitch3(uint8_t segment, uint8_t color2[3], uint8_t duration,  uint8_t reps) {
    uint8_t startColor[3] = {handoverColor[segment][0], handoverColor[segment][1], handoverColor[segment][2]};
    uint8_t otherSegment = 0;
    if (segment == 0) {otherSegment = 1;}
    // Hold otherSegment at its handoverColor, and flash segment between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        if (segment == 0) {
            showColor(startColor, handoverColor[1], duration);
            showColor(color2, handoverColor[1], duration);
        } else {
            showColor(handoverColor[0], startColor, duration);
            showColor(handoverColor[0], color2, duration);
        }
    }
}

// MARK: rapidPulse ------------------------------------------------------------------------------------------
void rapidPulse(uint8_t color1[3], uint8_t color2[3], uint8_t speed){
    showColor(color1, color1, speed);
    showColor(color2, color2, speed);
}

// MARK: gradientWave ------------------------------------------------------------------------------------------
void gradientWave(int speed){
    unsigned long start = millis();
    uint8_t outputColor[3];
    // Fade up over <speed> milliseconds
    while (millis() - start < speed) {
        // count up from 0 to 255 over speed milliseconds
        uint8_t progress = (uint8_t)((millis() - start) * 255 / speed);
        gradientPosition(progress, outputColor);
        showColor(outputColor, outputColor, 10);
    }
    // Fade down over <speed> milliseconds
    start = millis(); // Reset start time for fade down
    while (millis() - start < speed) {
        // count down from 255 to 0 over speed milliseconds
        uint8_t progress = (uint8_t)(255 - ((millis() - start) * 255 / speed));
        gradientPosition(progress, outputColor);
        showColor(outputColor, outputColor, 10);
    }
}

// ------------------------------------------------------------------------------------------------------------------
