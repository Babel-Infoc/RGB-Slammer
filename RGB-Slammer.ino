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

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

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

// Ambient brightness sensor setup
const bool sensorEnabled = false;   // Is the ambient brightness sensor enabled?
const uint8_t lightSensor = PC3;    // TEMT6000 light sensor pin
const float maxBrightness = 0.6;    // Master brightness attenuation, between 0 and 1
float brightnessMod = 0;            // Stores the sensor value

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
const luminance red       = {8, 52};
const luminance green     = {5, 163};
const luminance blue      = {3, 18};

// How many animations in the loop
extern const uint8_t numAnimations = 4;

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

    // Set up the light sensor
    pinMode(lightSensor, INPUT);

    // Calculate the luminosity modifiers
    calculateLuminance();

    // Set up random seeds
    float randSeed1(analogRead(0));
    float randSeed2(analogRead(1));

    // Show the bootup animation
    startup();
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/

void loop() {
    switch (animIndex) {
        case 0: pulseColor(200); break;
        case 1: neonFlicker(10, 50, 20); break;
        case 2: progressiveFade(5000); break;
        case 3: randomFade(20, 1000); break;
        default: pulseColor(200); break;
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
    fadeToColor(swatchArray[swatchIndex].accent,     swatchArray[swatchIndex].accent,      (speed));
    fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].background,  (speed*6));
}

// MARK: progressiveFade --------------------------------------------------------------------------------------------
// Fades through all colors in the active swatch in order, over <duration> milliseconds
void progressiveFade(const unsigned int duration) {
    fadeToColor(swatchArray[swatchIndex].highlight,  swatchArray[swatchIndex].highlight,   (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].primary,    swatchArray[swatchIndex].primary,     (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].accent,     swatchArray[swatchIndex].accent,      (duration/swatchSize));
    fadeToColor(swatchArray[swatchIndex].background, swatchArray[swatchIndex].background,  (duration/swatchSize));
}

// MARK: randomFade -------------------------------------------------------------------------------------------------
// Fades between random colors in the active swatch, each fade for a random speed defined by <min> and <max>
void randomFade(const uint8_t min, const uint8_t max) {
    // Get random color indices (0-3) for each LED channel
    uint8_t color1 = random(0, 4);
    uint8_t color2 = random(0, 4);

    // Arrays to hold the selected colors for each LED
    uint8_t* firstColor;
    uint8_t* secondColor;

    // Select the first color based on the random index
    switch (color1) {
        case 0: firstColor = swatchArray[swatchIndex].highlight; break;
        case 1: firstColor = swatchArray[swatchIndex].primary; break;
        case 2: firstColor = swatchArray[swatchIndex].accent; break;
        case 3: firstColor = swatchArray[swatchIndex].background; break;
    }

    // Select the second color based on the random index
    switch (color2) {
        case 0: secondColor = swatchArray[swatchIndex].highlight; break;
        case 1: secondColor = swatchArray[swatchIndex].primary; break;
        case 2: secondColor = swatchArray[swatchIndex].accent; break;
        case 3: secondColor = swatchArray[swatchIndex].background; break;
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


// MARK: startup ----------------------------------------------------------------------------------------------------
void startup(){
    uint8_t speed = 50;
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = 300;
        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            fadeToColor(bootswatch[0], bootswatch[1], speed);
            fadeToColor(bootswatch[1], bootswatch[2], speed);
            fadeToColor(bootswatch[2], bootswatch[3], speed);
            fadeToColor(bootswatch[3], bootswatch[4], speed);
            fadeToColor(bootswatch[4], bootswatch[4], speed);
        }
    }
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
