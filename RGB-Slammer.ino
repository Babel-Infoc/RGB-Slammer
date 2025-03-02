#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "rgbProcessor.h"

// MARK: ------------------------------ Variables and config ------------------------------

// Define each of the RGB Array pins, { segment number, R, G, B }
ledSegment led1 = {0, PD5, PD3, PD4};
ledSegment led2 = {1, PC5, PC6, PC4};

// Automatically calculate the number of segments
// Note: If you have more than 2, some andimation subroutines will need to be manually modified with more color arguments
extern const ledSegment leds[] = {led1, led2};
extern const int numSegments = sizeof(leds) / sizeof(leds[0]);

// Global variable definitions
extern int handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}}; // Definition for extern variable from types.h
extern const float maxBrightness = 0.8; // Maximum brightness modifier, 0-100
// MARK: ToDo: Add an ambient brightness modifier

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
// C2942981 - 20mA - R240, G=450, B=130
//https://www.lcsc.com/datasheet/lcsc_datasheet_2410121913_Lite-On-LTST-S33FBEGW-SN_C2942981.pdf
extern const luminance redLum       = {8, 52};
extern const luminance greenLum     = {5, 163};
extern const luminance blueLum      = {3, 18};

// Declare the gamma correction array
extern const uint8_t gamma8[];

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    for (int i = 0; i < numSegments; i++) {
        pinMode(leds[i].redPin,     OUTPUT);
        pinMode(leds[i].greenPin,   OUTPUT);
        pinMode(leds[i].bluePin,    OUTPUT);
    }
    startup();
    //delay(1000);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/

void loop() {
    //startup();
    //fadeToColor        ("rgb(0, 160, 223)",        "rgba(0, 0, 255, 0.84)",     200);
    //strobe              ("rgb(255,0,0)",        "rgb(30,30,50)", 70, 2);
    neonFlicker         ("rgb(122, 217, 255)",        "rgb(52, 91, 107)", 70,  5,  1000);
    //progressiveFade     (Anodize,            2000, 1);
    //randomFade          (Anodize,               50, 1000, 10);
    //pulseColor          ("rgb(175,238,238)",    "rgb(0,128,128)",   200, 2);
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: ------------------------------ Animation loops -----------------------------
/*
    ========= Animation loop reference =========

    pulseColor
        Pulses between two colors <highColor> and <lowColor> for <speed> milliseconds, for <reps> number of times
        (<highColor>, <lowColor>, <speed>, <reps>);

    progressiveFade
        Fades through <swatch> color array in order, over <speed> milliseconds, for <reps> number of times
        (<swatch>, <speed>, <reps>);

    randomFade
        Fades between two random colors in <swatch> array
        each fade for a random speed defined by <maxspeed> and <minspeed>, for <reps> number of times
        (<swatch>, <maxspeed>, <minspeed>, <reps>);

    strobe
        Rapidly flashes between two colors <color1> and <color2> for <speed> milliseconds, for <reps> number of times
        (<color1>, <color2>, <speed>, <reps>);

    neonFlicker
        Simulates a failing neon / fluorescent light flicker
        Flickers between <mainColor> and <flickerColor> with <intensity> and <chance> for <duration> milliseconds
        (<mainColor>, <flickerColor>, <intensity>, <chance>, <duration>);

    startup
        A predefined startup animation with no variables, modify this to your liking
*/

void pulseColor(const char* highColor, const char* lowColor, const int speed, const int reps){
    for (int j = 0; j < reps; j++) {
        fadeToColor(highColor, highColor, speed/6);
        fadeToColor(lowColor, lowColor,  speed);
        fadeToColor(lowColor, lowColor,  speed*3);
    }
}

void randomFade(const char* const* swatch, const int maxspeed, const int minspeed, const int reps) {
    int swatchSize = sizeof(swatch) / sizeof(swatch[0]);
    for (int j = 0; j < reps; j++) {
        int SW1 = random(0, swatchSize);
        int SW2 = random(0, swatchSize);
        int fadeTime = random(minspeed, maxspeed);
        fadeToColor(swatch[SW1], swatch[SW2], fadeTime);
    }
}

void progressiveFade(const char* const* swatch, const int speed, const int reps) {
    int swatchSize = sizeof(swatch) / sizeof(swatch[0]);
    const int fadeTime = speed / swatchSize;
    for (int j = 0; j < reps; j++) {
        for (int k = 0; k < swatchSize; k++) {
            fadeToColor(swatch[k],      swatch[(k + 1) % swatchSize], fadeTime);
        }
    }
}

void strobe(const char* color1, const char* color2, const int speed, const int reps) {
    for (int i = 0; i < numSegments; i++) {
        for (int j = 0; j < reps; j++) {
            showColor(color1,    color2,  speed);
            showColor(color2,    color2,  speed);
            showColor(color1,    color2,  speed);
            showColor(color2,    color2,  speed);
            showColor(color2,    color1,  speed);
            showColor(color2,    color2,  speed);
            showColor(color2,    color1,  speed);
            showColor(color2,    color2,  speed);
        }
    }
}

void startup(){
    int swatchSize = sizeof(bootswatch) / sizeof(bootswatch[0]);
    int speed = 50;
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < swatchSize; k++) {
            int speedAdj = (j == 2) ? speed*2 : speed;
            fadeToColor(bootswatch[k], bootswatch[(k + 1) % swatchSize], speedAdj);
        }
    }
    fadeToColor("rgb(0,0,0)", "rgb(0,0,0)", speed*6);
}

// MARK: ------------------------------ Animation Elements ------------------------------
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

    slamFade
        For both LED channels, jumps to <startColor>, then fades to <endColor>, over <fadeTime> milliseconds
        (<startColor>, <endColor>, <fadeTime>);
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
        sendToRGB(leds[0], rgbColor1);
        sendToRGB(leds[1], rgbColor2);
    }
}

/*
// MARK: fadeToColor
// Fades from the most recent color displayed before this function, to the endColor, over the fadeTime
void fadeToColor(const char* color1, const char* color2, const int fadeTime) {
    // Define the color arrays for each segment
    const int colorArray[numSegments][3];
    int startColor[numSegments][3];
    int rgbProgress[numSegments][3];

    // Convert the color strings to RGB arrays
    rgbStringToArray(color1, color[0][3]);
    rgbStringToArray(color2, color[1][3]);

    // Pull the start color from the most recent handover
    for (int r = 0; r < numSegments; r++) {
        for (int i = 0; i < 3; i++) {
            startColor[r][i] = handoverColor[r][i];
        }
    }

    // For <fadeTime>...
    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int r = 0; r < numSegments; r++) {
            for (int i = 0; i < 3; i++) {
                rgbProgress[r][i] = startColor[r][i] + (color[r][i] - startColor[r][i]) * fadeRatio;
            }
            sendToRGB(leds[r], rgbProgress[r]);
        }
    }
}*/

void fadeToColor(const char* color1, const char* color2, const int fadeTime) {
    int startColor[2][3];
    int endColor[2][3];
    int output[2][3];

    rgbStringToArray(color1, endColor[0]);
    rgbStringToArray(color2, endColor[1]);

    // Copy handoverColor to startColor
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            startColor[i][j] = handoverColor[i][j];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                output[i][j] = startColor[i][j] + (endColor[i][j] - startColor[i][j]) * fadeRatio;
            }
        }
        for (int i = 0; i < numSegments; i++) {
            sendToRGB(leds[i], output[i]);
        }
    }
}

// MARK: slamFade
// Fades from the startColor to the endColor over the time specified
void slamFade(const char* startColor, const char* endColor, const int fadeTime) {
    int startColorArray[3];
    int endColorArray[3];
    int output[3];
    rgbStringToArray(startColor, startColorArray);
    rgbStringToArray(endColor, endColorArray);

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int i = 0; i < 3; i++) {
            output[i] = startColorArray[i] + (endColorArray[i] - startColorArray[i]) * fadeRatio;
        }
        for (int i = 0; i < numSegments; i++) {
            sendToRGB(leds[i], output);
        }
    }
}

// MARK: neonFlicker
// Simulates a failing neon / fluorescent light flicker, adjust intensity and chance of flicker events
void neonFlicker(const char* mainColor, const char* flickerColor, const int intensity, const int chance, const int duration) {
    int mainRGB[3];
    int flickerRGB[3];
    int flickerOutput[3];
    rgbStringToArray(mainColor, mainRGB);
    rgbStringToArray(flickerColor, flickerRGB);

    unsigned long endTime = millis() + duration;
    while (millis() < endTime) {
        for (int j = 0; j < numSegments; j++) {
            bool shouldFlicker = random(0, 100) < chance;
            if (shouldFlicker) {
                int flickerInt = random(0, intensity);
                for (int r = 0; r < 3; r++) {
                    flickerOutput[r] = mainRGB[r] + ((flickerRGB[r] - mainRGB[r]) * flickerInt) / 100;
                }
                sendToRGB(leds[j], flickerOutput);
            } else {
                sendToRGB(leds[j], mainRGB);
            }
        }
        delay(20);
    }
}
