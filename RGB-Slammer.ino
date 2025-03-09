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
const float maxBrightness = 0.5;

// ToDo: Add an ambient brightness modifier

// Slow down all animations by this amount (in milliseconds)
const uint8_t slowDown = 0;

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
extern const uint8_t numAnimations = 4;

void loop() {
    switch (animIndex) {
        case 0: progressiveFade(15000); break;
        case 1: randomFade(20, 2000); break;
        case 2: pulseColor(700); break;
        case 3: neonFlicker(10, 80, 30); break;
        case 4: bounceBoot(50); break;
        case 5: holdYourColorPrimary(); break;
        case 6: holdYourColorAccent(); break;
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: pulseColor -------------------------------------------------------------------------------------------------
// Pulses from highlight through to background colors in the active swatch, over <speed> milliseconds
void pulseColor(const uint8_t duration){
    int speed = duration/swatchSize;
    fadeToColor(swatchArray[swatchIndex].highlight,  swatchArray[swatchIndex].highlight,   (speed/3));
    fadeToColor(swatchArray[swatchIndex].primary,    swatchArray[swatchIndex].primary,     (speed));
    fadeToColor(swatchArray[swatchIndex].ambient,    swatchArray[swatchIndex].ambient,     (speed));
    fadeToColor(swatchArray[swatchIndex].highlight,  swatchArray[swatchIndex].highlight,   (speed/3));
    fadeToColor(swatchArray[swatchIndex].primary,    swatchArray[swatchIndex].primary,     (speed));
    fadeToColor(swatchArray[swatchIndex].ambient,    swatchArray[swatchIndex].ambient,     (speed));
    fadeToColor(swatchArray[swatchIndex].accent,     swatchArray[swatchIndex].accent,      (speed));
    fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].background,  (speed*6));
}

// MARK: progressiveFade --------------------------------------------------------------------------------------------
// Fades through all colors in the active swatch in order, over <duration> milliseconds
void progressiveFade(const unsigned int duration) {
    fadeToColor(swatchArray[swatchIndex].highlight,  swatchArray[swatchIndex].highlight,   (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].primary,    swatchArray[swatchIndex].primary,     (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].ambient,    swatchArray[swatchIndex].ambient,     (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].accent,     swatchArray[swatchIndex].accent,      (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].background,  (duration/swatchSize));
}

// MARK: randomFade -------------------------------------------------------------------------------------------------
// Fades between random colors in the active swatch, each fade for a random speed defined by <min> and <max>
void randomFade(const uint8_t min, const uint8_t max) {
    // Get random color indices (0-3) for each LED channel
    uint8_t color1 = random(0, swatchSize);
    uint8_t color2 = random(0, swatchSize);

    // Arrays to hold the selected colors for each LED
    uint8_t* firstColor;
    uint8_t* secondColor;

    // Select the first color based on the random index
    switch (color1) {
        case 0: firstColor = swatchArray[swatchIndex].highlight; break;
        case 1: firstColor = swatchArray[swatchIndex].primary; break;
        case 2: firstColor = swatchArray[swatchIndex].ambient; break;
        case 3: firstColor = swatchArray[swatchIndex].accent; break;
        case 4: firstColor = swatchArray[swatchIndex].background; break;
    }

    // Select the second color based on the random index
    switch (color2) {
        case 0: secondColor = swatchArray[swatchIndex].highlight; break;
        case 1: secondColor = swatchArray[swatchIndex].primary; break;
        case 2: secondColor = swatchArray[swatchIndex].ambient; break;
        case 3: secondColor = swatchArray[swatchIndex].accent; break;
        case 4: secondColor = swatchArray[swatchIndex].background; break;
    }

    // Fade between the two random colors with a random duration
    fadeToColor(firstColor, secondColor, random(min, max));
}

// MARK: neonFlicker ------------------------------------------------------------------------------------------------
// Simulates a failing neon/fluorescent light flicker
// By default shows <standard>, flickers to <background>, with <chance> of flickering
void neonFlicker(const uint8_t chance, const uint8_t intensity, const uint8_t speed) {
    uint8_t flickerOutput[2][3];
    uint8_t flickerTime = 20;

    unsigned long startTime = millis();
    for (uint8_t segment = 0; segment < numLEDs; segment++) {
        bool flickerChance = random(0, 100) < chance;
        if (flickerChance) {
            uint8_t flickerInt = random(1, intensity + 1); // Ensure non-zero value
            for (uint8_t pin = 0; pin < 3; pin++) {
                // Mix primary color and background color with flickerInt
                flickerOutput[segment][pin] = swatchArray[swatchIndex].primary[pin] + (swatchArray[swatchIndex].background[pin] - swatchArray[swatchIndex].primary[pin]) * flickerInt;
            }
            sendToRGB(segment, flickerOutput[segment]);
        } else {
            sendToRGB(segment, swatchArray[swatchIndex].primary);
        }
    }
    delay(flickerTime);
}


// MARK: bounceBoot ----------------------------------------------------------------------------------------------------
void bounceBoot(uint8_t speed){
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = 300;
        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            fadeToColor(swatchArray[swatchIndex].highlight,  swatchArray[swatchIndex].background,   (speed));
            fadeToColor(swatchArray[swatchIndex].primary,    swatchArray[swatchIndex].highlight,    (speed));
            fadeToColor(swatchArray[swatchIndex].ambient,    swatchArray[swatchIndex].primary,      (speed));
            fadeToColor(swatchArray[swatchIndex].accent,     swatchArray[swatchIndex].ambient,      (speed));
            fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].accent,       (speed));
            fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].background,   (speed));
        }
    }
}

// MARK: holdYourColor ------------------------------------------------------------------------------------------------
// Shows the primary color only
void holdYourColorPrimary(){
    sendToRGB(0, swatchArray[swatchIndex].primary);
    sendToRGB(1, swatchArray[swatchIndex].primary);
}

// Shows the primary and accent colors
void holdYourColorAccent(){
    sendToRGB(0, swatchArray[swatchIndex].primary);
    sendToRGB(1, swatchArray[swatchIndex].accent);
}

// MARK: --- Animation processor
// MARK: fadeToColor ------------------------------------------------------------------------------------------------
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const uint8_t fadeTime){
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
// ------------------------------------------------------------------------------------------------------------------
