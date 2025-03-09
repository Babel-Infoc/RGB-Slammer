/*
RGB Slammer
Written by Tully Jagoe 2025
MIT License

This script is best edited in VSCode, using the following extension:
https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token
*/

// MARK: ------------------------------ Variables and config ------------------------------
#include <stdint.h>
#include "types.h"
#include "swatches.h"

// Define numLEDs here instead of in types.h to avoid multiple definitions
const uint8_t numLEDs = 2;

// The `leds` array contains the pin configurations for different LEDs.
// Each element in the array represents a different LED with its associated pins.
ledSegment leds[2] = {
    {PD5, PD3, PD4},
    {PC5, PC6, PC4}
};

// Define the pins for the animation and color select buttons
const uint8_t colorBtn = PC3;
const uint8_t animBtn = PC0;

// Maximum brightness modifier, 0-100
const float maxBrightness = 50;
// ToDo: Add an ambient brightness modifier

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
const luminance red       = {8, 52};
const luminance green     = {5, 163};
const luminance blue      = {3, 18};

// How many animations in the loop
extern const uint8_t numAnimations = 4;

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    // Calculate the luminosity modifiers
    calculateLuminance();

    // Set up all LED segments
    for (uint8_t segment = 0; segment < 2; segment++) {
        pinMode(leds[segment].red, OUTPUT);
        pinMode(leds[segment].green, OUTPUT);
        pinMode(leds[segment].blue, OUTPUT);
    }
    // Set up the color and animation buttons
    pinMode(colorBtn, INPUT_PULLUP);
    pinMode(animBtn, INPUT_PULLUP);

    // Show the bootup animation
    startup();
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/

void loop() {
    startup();
    /*
    switch (animIndex) {
        case 0: pulseColor(200); break;
        case 1: neonFlicker(10); break;
        case 2: progressiveFade(500); break;
        case 3: randomFade(500, 200); break;
        default: pulseColor(200); break;
    }
    */
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: --- Animation loops ---
/*
    pulseColor
        Pulses from highlight through to background colors in the active swatch, over <speed> milliseconds

    progressiveFade
        Fades through all colors in the active swatch in order, over <speed> milliseconds

    randomFade
        Fades between random colors in the active swatch, each fade for a random speed defined by <min> and <max>
        (<min>, <max>);

    strobe
        Rapidly flashes between the highlight and background colors in the active swatch, over <speed> milliseconds

    neonFlicker
        Simulates a failing neon/fluorescent light flicker
        By default shows <standard>, flickers to <background>, with <chance> of flickering

    startup
        A predefined startup animation with no variables, modify this to your liking
*/

// MARK: pulseColor -------------------------------------------------------------------------------------------------
void pulseColor(const uint8_t speed){

}

// MARK: progressiveFade --------------------------------------------------------------------------------------------
void progressiveFade(const uint8_t speed) {

}

// MARK: randomFade -------------------------------------------------------------------------------------------------
void randomFade(const uint8_t min, const uint8_t max) {

}

// MARK: neonFlicker ------------------------------------------------------------------------------------------------
void neonFlicker(const uint8_t chance) {

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

    // Calculate the fade ratio
    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        for (uint8_t pin = 0; pin < 3; pin++) {
            float fadeSpeed = (float)(millis() - startTime) / fadeTime;
            output[0][pin] = startColor[0][pin] + (color1[pin] - startColor[0][pin]) * fadeSpeed;
            output[1][pin] = startColor[1][pin] + (color2[pin] - startColor[1][pin]) * fadeSpeed;
            sendToRGB(0, output[0]);
        }
    }
}
// ------------------------------------------------------------------------------------------------------------------
