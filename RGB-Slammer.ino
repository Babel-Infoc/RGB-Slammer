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

// Define your LED segment pins, in the order {Red, Green, Blue}
int ledSegment[2][3] = {
    {PD5, PD3, PD4},
    {PC5, PC6, PC4}
};

// Define the pins for the animation and color select buttons
const int colorBtn = PC3;
const int animBtn = PC0;

// Maximum brightness modifier, 0-100
const int maxBrightness = 50;
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
// C2942981 - 20mA - R240, G=450, B=130
//https://www.lcsc.com/datasheet/lcsc_datasheet_2410121913_Lite-On-LTST-S33FBEGW-SN_C2942981.pdf
const luminance red       = {8, 52};
const luminance green     = {5, 163};
const luminance blue      = {3, 18};

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    // Calculate the luminosity modifiers
    calculateLuminance();

    // Get the number of swatches in the swatchArray
    int swatchArraySize = sizeof(swatchArray) / sizeof(swatch);

    // Set up all LED segments
    for (int segment = 0; segment < 2; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            pinMode(ledSegment[segment][pin], OUTPUT);
        }
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
    //if (animIndex == 0) {
    //    startup();
    //} else if (animIndex == 1) {
    //    neonFlicker(50, 10);
    //} else if (animIndex == 2) {
        pulseColor(200);
    //} else if (animIndex == 3) {
    //    progressiveFade(500);
    //}
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
    fadeToColor(currentSwatch->highlight,   currentSwatch->highlight,   speed/6);
    fadeToColor(currentSwatch->primary,     currentSwatch->primary,     speed);
    fadeToColor(currentSwatch->accent,      currentSwatch->accent,      speed);
    fadeToColor(currentSwatch->primary,     currentSwatch->primary,     speed*3);
}

// MARK: progressiveFade
void progressiveFade(const int speed) {
   // Over <speed> milliseconds, fade through all colors in currentSwatch in order
    const int fadeTime = speed / 4; // 4 colors in the swatch
    fadeToColor(currentSwatch->highlight,   currentSwatch->highlight,   fadeTime);
    fadeToColor(currentSwatch->primary,     currentSwatch->primary,     fadeTime);
    fadeToColor(currentSwatch->accent,      currentSwatch->accent,      fadeTime);
    fadeToColor(currentSwatch->background,  currentSwatch->background,  fadeTime);
}

// MARK: randomFade
void randomFade(const int maxspeed, const int minspeed) {
    char* randColorArray[4] = {
        currentSwatch->highlight,
        currentSwatch->primary,
        currentSwatch->accent,
        currentSwatch->background
    };
    int rand1 = random(0, swatchSize);
    int rand2 = random(0, swatchSize);
    int randSpeed = random(minspeed, maxspeed + 1);
    fadeToColor(randColorArray[rand1], randColorArray[rand2], randSpeed);
}

// MARK: neonFlicker
void neonFlicker(const int intensity, const int chance) {
    int highlight[3];
    int primary[3];
    int accent[3];
    int background[3];

    int flickerOutput[3];

    rgbStringToArray(currentSwatch->highlight, highlight);
    rgbStringToArray(currentSwatch->primary, primary);
    rgbStringToArray(currentSwatch->accent, accent);
    rgbStringToArray(currentSwatch->background, background);

    for (uint8_t segment = 0; segment < 2; segment++) {
        if (random(100) < chance) {
            int flickerInt = random(1, intensity + 1); // Ensure non-zero value
            // Mix primary and background colors, at intensity
            for (int channel = 0; channel < 3; channel++) {
                flickerOutput[channel] = constrain(primary[channel] / flickerInt + background[channel], 0, 255);
            }
            sendToRGB(segment, flickerOutput);
        }
        // If the flicker is not successful, stay on the primary color
        sendToRGB(segment, primary);
    }
}

void startup(){
    // Count the number of colors in the swatch (until NULL)
    int speed = 50;
    for (int pin = 0; pin < 3; pin++) {
        for (int k = 0; k < 5; k++) {
            int speedAdj = (pin == 2) ? speed*2 : speed;
            fadeToColor(bootswatch[k], bootswatch[(k + 1) % 5], speedAdj);
        }
    }
    fadeToColor("rgb(0,0,0)", "rgb(0,0,0)", speed*6);
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

    neonFlicker
        Simulates a neon / fluorescent light flicker
        (<mainColor>, <flickerColor>, <intensity>, <chance>, <duration>);
*/

// MARK: showColor
// Displays the color for the specified hangTime
void showColor(const char* color1, const char* color2, const int hangTime) {
    int rgbColor1[3];
    int rgbColor2[3];
    rgbStringToArray(color1, rgbColor1);
    rgbStringToArray(color2, rgbColor2);

    unsigned long startTime = millis();
    while (millis() - startTime < hangTime) {
        sendToRGB(0, rgbColor1);
        sendToRGB(1, rgbColor2);
    }
}

// MARK: fadeToColor
void fadeToColor(const char* color1, const char* color2, const int fadeTime) {
    int startColor[2][3];
    int endColor[2][3];
    int output[2][3];

    rgbStringToArray(color1, endColor[0]);
    rgbStringToArray(color2, endColor[1]);

    // Copy handoverColor to startColor
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
