#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "rgbProcessor.h"

/*
    RGB Slammer
    Written by Tully Jagoe 2025
    MIT License

    This script is best edited in VSCode, using the following extensions:
    For visualising / selecting color strings:
    https://marketplace.visualstudio.com/items?itemName=yechunan.json-color-token
*/

// MARK: ------------------------------ Variables and config ------------------------------

// The `leds` array contains the pin configurations for different LEDs.
// Each element in the array represents a different LED with its associated pins.
led_t leds[] = {
    {PD5, PD3, PD4},  // LED segment 0
    {PC5, PC6, PC4}   // LED segment 1
};

// Animation and Color select button pins
int animBtn = PC0;
int colrBtn = PC3;

// Automatically calculate the number of segments
// Note: If you have more than 2, some animation subroutines will need to be manually modified with more color arguments
const int numLEDs = sizeof(leds) / sizeof(leds[0]);

// Global variable definitions
int handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}}; // Definition for extern variable from types.h
const float maxBrightness = 0.5; // Maximum brightness modifier, 0-100
// MARK: ToDo: Add an ambient brightness modifier

// Current color swatch
const int mainSwatchSize = 11;
int mainSwatchIndex = 0;

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

// MARK: ------------------------------ Startup operations ------------------------------
void setup() {
    for (int i = 0; i < numLEDs; i++) {
        pinMode(leds[i].redPin, OUTPUT);
        pinMode(leds[i].greenPin, OUTPUT);
        pinMode(leds[i].bluePin, OUTPUT);
    }
    pinMode(animBtn, INPUT_PULLUP);
    pinMode(colrBtn, INPUT_PULLUP);
    startup();
    //delay(1000);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/

void loop() {
    // Check for button presses
    checkColorButton();

    //progressiveFade     (bootswatch, 10000, 20);
    //strobe              ("rgb(255,0,0)",        "rgb(30,30,50)", 70, 2);
    neonFlicker         ("rgb(180, 234, 255)", 50,  10,  5000);
    //randomFade          (Anodize,   50,     500,   20);
    //pulseColor          ("rgb(175,238,238)",    "rgb(0,128,128)",   200, 2);
    //startup();
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// MARK: --- Animation loops ---
/*
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

// MARK: pulseColor
void pulseColor(const char* highColor, const char* lowColor, const int speed, const int reps){
    for (int j = 0; j < reps; j++) {
        fadeToColor(highColor, highColor, speed/6);
        fadeToColor(lowColor, lowColor,  speed);
        fadeToColor(lowColor, lowColor,  speed*3);
    }
}

// MARK: progressiveFade
void progressiveFade(const char* swatch[], const int speed, const int reps) {
    // Count the number of colors in the swatch (until NULL)
    int swatchSize = 0;
    while (swatch[swatchSize] != NULL) {
        swatchSize++;
    }

    const int fadeTime = speed / swatchSize;
    for (int j = 0; j < reps; j++) {
        for (int k = 0; k < swatchSize; k++) {
            fadeToColor(swatch[k], swatch[(k + 1) % swatchSize], fadeTime);
        }
    }
}

// MARK: randomFade
void randomFade(const char* swatch[], const int maxspeed, const int minspeed, const int reps) {
    // Count the number of colors in the swatch (until NULL)
    int swatchSize = 0;
    while (swatch[swatchSize] != NULL) {
        swatchSize++;
    }

    // Perform random fades for the specified number of reps
    for (int j = 0; j < reps; j++) {
        // Select two random colors from the swatch
        int randomIndex1 = random(0, swatchSize);
        int randomIndex2 = random(0, swatchSize);

        // Calculate a random fade time between minspeed and maxspeed
        int randomSpeed = random(minspeed, maxspeed + 1);

        // Fade between the two random colors
        fadeToColor(swatch[randomIndex1], swatch[randomIndex2], randomSpeed);
    }
}

// MARK: strobe
void strobe(const char* color1, const char* color2, const int speed, const int reps) {
    for (int i = 0; i < numLEDs; i++) {
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

// MARK: neonFlicker
void neonFlicker(const char* mainColor, const int intensity, const int chance, const int duration) {
    int mainRGB[3];
    int flickerOutput[3];
    int flickerTime = 20;
    rgbStringToArray(mainColor, mainRGB);

    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int j = 0; j < numLEDs; j++) {
            bool flickerChance = random(0, 100) < chance;
            if (flickerChance) {
                int flickerInt = random(1, intensity + 1); // Ensure non-zero value
                for (int r = 0; r < 3; r++) {
                    // Reduce brightness by dividing by flickerInt
                    flickerOutput[r] = constrain(mainRGB[r] / flickerInt, 0, 255);
                }
                sendToRGB(j, flickerOutput);
            } else {
                sendToRGB(j, mainRGB);
            }
        }
        delay(flickerTime);
    }
}

void startup(){
    // Count the number of colors in the swatch (until NULL)
    int swatchSize = 0;
    while (bootswatch[swatchSize] != NULL) {
        swatchSize++;
    }

    int speed = 50;
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < swatchSize; k++) {
            int speedAdj = (j == 2) ? speed*2 : speed;
            fadeToColor(bootswatch[k], bootswatch[(k + 1) % swatchSize], speedAdj);
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
        for (int i = 0; i < numLEDs; i++) {
            sendToRGB(i, output[i]);
        }
    }
}

// MARK: --- Button Handling ---

// Variables for button debouncing
unsigned long lastColorButtonDebounceTime = 0;
int lastColorButtonState = HIGH; // Using INPUT_PULLUP, so default is HIGH
const int debounceDelay = 50; // Debounce time in milliseconds

// Function to check if color button has been pressed
void checkColorButton() {
    // Read the current button state
    int currentButtonState = digitalRead(colrBtn);

    // Check if the button state has changed
    if (currentButtonState != lastColorButtonState) {
        // Reset the debouncing timer
        lastColorButtonDebounceTime = millis();
    }

    // Check if enough time has passed since the last state change
    if ((millis() - lastColorButtonDebounceTime) > debounceDelay) {
        // If the button state has changed and is now LOW (pressed)
        if (currentButtonState == LOW && lastColorButtonState == HIGH) {
            // Increment the swatch index
            mainSwatchIndex++;

            // Wrap around if exceeded the maximum
            if (mainSwatchIndex >= mainSwatchSize) {
                mainSwatchIndex = 0;
            }

            // You could add some visual feedback here to indicate the swatch changed
            // For example, a brief flash of the new color
        }
    }

    // Save the current button state for next comparison
    lastColorButtonState = currentButtonState;
}
