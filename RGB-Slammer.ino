#include "types.h"
#include "swatches.h"

/*
    RGB Slammer
    Written by Tully Jagoe 2025
    MIT License

    This script is best edited in VSCode, using the following extension:
    https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token
*/

// MARK: ------------------------------ Variables and config ------------------------------

// Define numLEDs here instead of in types.h to avoid multiple definitions
const int numLEDs = 2;

// The `leds` array contains the pin configurations for different LEDs.
// Each element in the array represents a different LED with its associated pins.
ledSegment leds[2] = {
    {PD5, PD3, PD4},
    {PC5, PC6, PC4}
};

// Define the pins for the animation and color select buttons
const int colorBtn = PC3;
const int animBtn = PC0;

// Maximum brightness modifier, 0-100
const float maxBrightness = 50;
// ToDo: Add an ambient brightness modifier

// Current color swatch
int swatchIndex = 0;
// Define the currentSwatch variable (declared as extern in types.h)
swatch currentSwatch;

// Current animation index
const int numAnimations = 4;
int animIndex = 0;

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
const luminance red       = {8, 52};
const luminance green     = {5, 163};
const luminance blue      = {3, 18};

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    // Calculate the luminosity modifiers
    calculateLuminance();

    // Initialize currentSwatch with the first swatch from the array
    // Change from pointer notation (->) to direct access (.)
    currentSwatch.highlight[0] = swatchArray[0].highlight[0];
    currentSwatch.highlight[1] = swatchArray[0].highlight[1];
    currentSwatch.highlight[2] = swatchArray[0].highlight[2];

    currentSwatch.primary[0] = swatchArray[0].primary[0];
    currentSwatch.primary[1] = swatchArray[0].primary[1];
    currentSwatch.primary[2] = swatchArray[0].primary[2];

    currentSwatch.accent[0] = swatchArray[0].accent[0];
    currentSwatch.accent[1] = swatchArray[0].accent[1];
    currentSwatch.accent[2] = swatchArray[0].accent[2];

    currentSwatch.background[0] = swatchArray[0].background[0];
    currentSwatch.background[1] = swatchArray[0].background[1];
    currentSwatch.background[2] = swatchArray[0].background[2];

    // Set up all LED segments
    for (int segment = 0; segment < 2; segment++) {
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
    switch (animIndex) {
        case 0: pulseColor(200); break;
        case 1: neonFlicker(10); break;
        case 2: progressiveFade(500); break;
        case 3: randomFade(500, 200); break;
        default: pulseColor(200); break;
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: --- Animation loops ---
/*
    pulseColor
        Pulses between primary and accent colors of the swatch for <speed> milliseconds
        (<swatch>, <speed>);

    progressiveFade
        Fades through all colors in the swatch in order, over <speed> milliseconds
        (<swatch>, <speed>);

    randomFade
        Fades between random colors in the swatch
        each fade for a random speed defined by <maxspeed> and <minspeed>
        (<swatch>, <maxspeed>, <minspeed>);

    strobe
        Rapidly flashes between primary and background colors in the swatch for <speed> milliseconds
        (<swatch>, <speed>);

    neonFlicker
        Simulates a failing neon/fluorescent light flicker
        Flickers highlight color with specified <intensity> and <chance>
        (<swatch>, <intensity>, <chance>);

    startup
        A predefined startup animation with no variables, modify this to your liking
*/

// MARK: pulseColor
void pulseColor(const int speed){
    // Pulse between primary and accent colors
    fadeToColor(currentSwatch.highlight, currentSwatch.highlight, speed/6);
    fadeToColor(currentSwatch.primary, currentSwatch.primary, speed);
    fadeToColor(currentSwatch.accent, currentSwatch.accent, speed);
    fadeToColor(currentSwatch.primary, currentSwatch.primary, speed*3);
}

// MARK: progressiveFade
void progressiveFade(const int speed) {
   // Over <speed> milliseconds, fade through all colors in currentSwatch in order
    const int fadeTime = speed / 4; // 4 colors in the swatch
    fadeToColor(currentSwatch.highlight, currentSwatch.highlight, fadeTime);
    fadeToColor(currentSwatch.primary, currentSwatch.primary, fadeTime);
    fadeToColor(currentSwatch.accent, currentSwatch.accent, fadeTime);
    fadeToColor(currentSwatch.background, currentSwatch.background, fadeTime);
}

// MARK: randomFade
void randomFade(const int min, const int max) {
    // Changed to use int arrays directly
    int* randColorArray[4] = {
        currentSwatch.highlight,
        currentSwatch.primary,
        currentSwatch.accent,
        currentSwatch.background
    };
    int rand1 = random(0, 4);
    int rand2 = random(0, 4);
    int randSpeed = random(min, max);
    fadeToColor(randColorArray[rand1], randColorArray[rand2], randSpeed);
}

// MARK: neonFlicker
void neonFlicker(const int chance) {
    // Changed from pointer notation to direct access
    int* primary = currentSwatch.primary;
    int* background = currentSwatch.background;

    for (uint8_t segment = 0; segment < 2; segment++) {
        if (random(100) < chance) {
            sendToRGB(segment, background);
        } else {
            sendToRGB(segment, primary);
        }
    }
}

void startup(){
    // Count the number of colors in the swatch (until NULL)
    int speed = 50;
    for (int pin = 0; pin < 3; pin++) {
        for (int k = 0; k < 5; k++) {  // Using direct size instead of bootswatch.size()
            int speedAdj = (pin == 2) ? speed*2 : speed;
            fadeToColor(bootswatch[k], bootswatch[(k + 1) % 5], speedAdj);
        }
    }

    // Create temporary arrays for the rgb macro values
    int black1[3] = {0, 0, 0};
    int black2[3] = {0, 0, 0};
    fadeToColor(black1, black2, speed*6);
}

// MARK: --- Animation Elements ---
/*
    ========= Animation subroutine reference =========
    showColor
        Holds both LED channels at one color each for <hangTime> milliseconds
        (<color1>, <color2>, <hangTime>);

    fadeToColor
        The main workhorse of the animation suite, modify this at your peril
        Fades from  the last <handoverColor> to <colorx> over <fade time> milliseconds
        <color1> and <color2> correspond to segments led1 and led2
        (<color1>, <color2>, <fade time>);
*/

// MARK: showColor
// Displays the color for the specified hangTime
void showColor(int* color1, int* color2, const int hangTime) {
    unsigned long startTime = millis();
    while (millis() - startTime < hangTime) {
        sendToRGB(0, color1);
        sendToRGB(1, color2);
    }
}

// MARK: fadeToColor
void fadeToColor(const int (&endColor1)[3], const int (&endColor2)[3], const int fadeTime) {
    // Changed from std::array to C arrays
    int startColor[2][3];
    int output[2][3];

    // Copy handover color to startColor
    for (int segment = 0; segment < 2; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            startColor[segment][pin] = handoverColor[segment][pin];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int segment = 0; segment < 2; segment++) {
            for (int pin = 0; pin < 3; pin++) {
                output[segment][pin] = startColor[segment][pin] + (endColor[segment][pin] - startColor[segment][pin]) * fadeRatio;
            }
        }
        for (int segment = 0; segment < 2; segment++) {
            sendToRGB(segment, output[segment]);
        }
    }
}
