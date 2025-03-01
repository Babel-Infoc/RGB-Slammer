#include <types.h>
#include <swatches.h>
#include <rgbProcessor.h>

// -------- Constants --------
// Define each of the RGB Array pins, { number, R, G, B }
const int numSegments = 2;
ledSegment led1 = {0, PD5, PD3, PD4};
ledSegment led2 = {1, PC5, PC6, PC4};

// Array of ledSegments
const ledSegment leds[] = {led1, led2};

// Global variable definitions
int handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}}; // Definition for extern variable from types.h
float ambientBrightness = 1.0; // Definition of the extern variable from types.h

// Max output brightness modifier (between 0.2 and 1)
const float maxBrightness   = 0.5;

// Luminosity modifiers
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values at different currents
// C2942981 - 20mA - R240, G=450, B=130
//https://www.lcsc.com/datasheet/lcsc_datasheet_2410121913_Lite-On-LTST-S33FBEGW-SN_C2942981.pdf
const luminance redLum       = {8, 52};
const luminance greenLum     = {5, 163};
const luminance blueLum      = {3, 18};

// Declare the gamma correction array
extern const uint8_t gamma8[];

// MARK: ------------------------------ Setup ------------------------------
void setup() {
    for (int i = 0; i < numSegments; i++) {
        pinMode(leds[i].redPin, OUTPUT);
        pinMode(leds[i].greenPin, OUTPUT);
        pinMode(leds[i].bluePin, OUTPUT);
    }
   startup();
}

// MARK: ------------------------------ Loop ------------------------------
void loop() {
    // Update the sequence for each LED segment
    showColor           ("rgb(255,0,0)",    "rgb(0,0,255)",     1000);
    showColor           ("rgb(0,0,255)",    "rgb(255,0,0)",     1000);
    fadeToColor         ("rgb(255,0,0)",    "rgb(0,0,255)",     1000);
    fadeToColor         ("rgb(0,0,255)",    "rgb(255,0,0)",     1000);
    //fadeToColor         ("rgb(255,69,0)", "rgb(0,206,209)", 2000);
    //strobe              ("rgb(255,0,0)", "rgb(105,105,105)", 70, 2);
    //randomFade          (Anodize, 20, 400, 5);
    //startup();
    //pulseColor          ("rgb(75,0,130)", "rgb(0,0,128)", 200, 2);
    //pulseColor          ("rgb(175,238,238)", "rgb(0,128,128)", 200, 2);
    //progressiveFade     (BisexualLighting, 1, 150, 1);
    //pulseColor          ("rgb(128,0,128)", "rgb(255,69,0)", 200, 2);
    //randomFade          (Anodize, 200, 2000, 10);
    //progressiveFade     (BisexualLighting, 1, 150, 2);
}

// MARK: ------------------------------ Sequence control ------------------------------
void startup(){
    unsigned long startTime = millis();
    for (int i = 0; i < numSegments; i++) {
        unsigned long segmentStartTime = startTime + (i * 300);
        while (millis() < segmentStartTime) {
            // Wait until the staggered start time for this segment
        }
        for (int j = 0; j < 3; j++) {
            fadeToColor("rgb(0,0,0)",           "rgb(255,255,255)",     50);            // #FFFFFF
            fadeToColor("rgb(255,255,255)",     "rgb(255,69,0)",        50);            // #ff4500
            fadeToColor("rgb(255,69,0)",        "rgb(75,0,130)",        50);            // #4B0082
            fadeToColor("rgb(75,0,130)",        "rgb(0,0,128)",         50);            // #000080
            fadeToColor("rgb(0,0,128)",         "rgb(0,0,0)",           50);            // #000000
            fadeToColor("rgb(0,0,0)",           "rgb(0,0,0)",           50);            // #FFFFFF
        }
    }
}

void pulseColor(const char* highColor, const char* lowColor, const int speed, const int reps){
    for (int j = 0; j < reps; j++) {
        fadeToColor(highColor, highColor, speed/6);
        fadeToColor(lowColor, lowColor,  speed);
        fadeToColor(lowColor, lowColor,  speed*3);
    }
}

void randomFade(const char* swatch[], const int maxspeed, const int minspeed, const int reps) {
    int swatchSize = sizeof(swatch) / sizeof(swatch[0]);
    for (int i = 0; i < numSegments; i++) {
        for (int j = 0; j < reps; j++) {
            int colorIndex1 = random(0, swatchSize);
            int colorIndex2 = random(0, swatchSize);
            int fadeTime = random(minspeed, maxspeed);
            fadeToColor(swatch[colorIndex1], swatch[colorIndex2], fadeTime);
        }
    }
}

void progressiveFade(const char* swatch[], const int offset, const int speed, const int reps) {
    int swatchSize = sizeof(swatch) / sizeof(swatch[0]);
    for (int j = 0; j < reps; j++) {
        for (int k = 0; k < swatchSize; k++) {
            fadeToColor(swatch[k], swatch[(k + offset) % swatchSize], speed);
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

// MARK: ------------------------------ Animation Elements ------------------------------
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

void holdColor(const ledSegment& segment, const int hangTime) {
    unsigned long startTime = millis();
    while (millis() - startTime < hangTime) {
        sendToRGB(segment, handoverColor[segment.ledNum - 1]);
    }
}

// Fades from the startColor to the endColor over the time specified
void slamFade(const ledSegment& segment, const char* startColor, const char* endColor, const int fadeTime) {
    int startColorArray[3];
    int endColorArray[3];
    rgbStringToArray(startColor, startColorArray);
    rgbStringToArray(endColor, endColorArray);

    int rgbColor[3];
    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int i = 0; i < 3; i++) {
            rgbColor[i] = startColorArray[i] + (endColorArray[i] - startColorArray[i]) * fadeRatio;
        }
        for (int i = 0; i < numSegments; i++) {
            sendToRGB(leds[i], rgbColor);
        }
    }
}

// Fades from the most recent color displayed before this function, to the endColor, over the fadeTime
void fadeToColor(const char* color1, const char* color2, const int fadeTime) {
    const int color1Array[3];
    const int color2Array[3];
    int startColor1[3];
    int startColor2[3];
    int rgbProgress1[3];
    int rgbProgress2[3];
    unsigned long startTime = millis();

    // Convert the color strings to RGB arrays
    rgbStringToArray(color1, color1Array);
    rgbStringToArray(color2, color2Array);

    // Pull the start color from the most recent handover, once
    for (int i = 0; i < 3; i++) {
        startColor1[i] = handoverColor[0][i];
        startColor2[i] = handoverColor[1][i];
    }

    while (millis() - startTime < fadeTime) {
        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int i = 0; i < 3; i++) {
            rgbProgress1[i] = startColor1[i] + (color1Array[i] - startColor1[i]) * fadeRatio;
            rgbProgress2[i] = startColor2[i] + (color2Array[i] - startColor2[i]) * fadeRatio;
        }
        sendToRGB(leds[0], rgbProgress1);
        sendToRGB(leds[1], rgbProgress2);
    }
}
