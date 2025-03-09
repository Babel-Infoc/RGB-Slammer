#include <Arduino.h>
#include "swatches.h"
#include "types.h"

// Initialize the last state of the color button
int colorButtonLastState = HIGH;
int animButtonLastState = HIGH;

// Replace std::array with plain C arrays to save significant flash memory
float tuneRatio[3] = {1.0, 1.0, 1.0};
int handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}};

// Reference to the leds array defined in the main sketch
extern ledSegment leds[];

// MARK: ------------------------------ Gamma Correction ------------------------------
// Even more efficient gamma function using a lookup of just 16 values instead of 256
// We can interpolate between these values for the full range
const uint8_t gammaLUT16[] = {0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 125, 165, 195, 225, 255};

uint8_t applyGamma(uint8_t value) {
    // Simple and efficient gamma approximation using 4-bit index
    return gammaLUT16[value >> 4];
}

// MARK: ------------------------------ Luminance Calculation ------------------------------
// Function to calculate luminance ratios for consistent color output
void calculateLuminance() {
    // Convert to fixed-point to save flash by eliminating floating point operations
    // Scale values by 256 for 8-bit fixed-point math
    int rLum = (red.luminance << 8) / red.mA;
    int gLum = (green.luminance << 8) / green.mA;
    int bLum = (blue.luminance << 8) / blue.mA;

    // Find the maximum value
    int maxLum = rLum;
    if (gLum > maxLum) maxLum = gLum;
    if (bLum > maxLum) maxLum = bLum;

    // Calculate and store the tuning ratios
    tuneRatio[0] = (float)maxLum / rLum;
    tuneRatio[1] = (float)maxLum / gLum;
    tuneRatio[2] = (float)maxLum / bLum;
}

// MARK: ------------------------------ Button Handling ------------------------------
void checkButtons() {
    // Check color button
    int colorButtonState = digitalRead(colorBtn);
    if (colorButtonState != colorButtonLastState) {
        if (colorButtonState == LOW) {
            swatchIndex = (swatchIndex + 1) % numSwatches;

            // Copy the values element by element to avoid array assignment
            for (int i = 0; i < 3; i++) {
                currentSwatch.highlight[i] = swatchArray[swatchIndex].highlight[i];
                currentSwatch.primary[i] = swatchArray[swatchIndex].primary[i];
                currentSwatch.accent[i] = swatchArray[swatchIndex].accent[i];
                currentSwatch.background[i] = swatchArray[swatchIndex].background[i];
            }
        }
        colorButtonLastState = colorButtonState;
    }

    // Check animation button
    int animButtonState = digitalRead(animBtn);
    if (animButtonState != animButtonLastState) {
        if (animButtonState == LOW) {
            animIndex = (animIndex + 1) % numAnimations;
        }
        animButtonLastState = animButtonState;
    }
}

// MARK: ------------------------------ RGB Processing ------------------------------

/*
// Highly optimized RGB string parsing
void rgbStringToArray(const char* rgbString, int* rgbArray) {
    // Skip to first digit
    const char* p = rgbString;
    while (*p && (*p < '0' || *p > '9')) p++;

    // Parse the three numbers
    for (int i = 0; i < 3; i++) {
        rgbArray[i] = 0;
        // Parse digits for this component
        while (*p >= '0' && *p <= '9') {
            rgbArray[i] = rgbArray[i] * 10 + (*p - '0');
            p++;
        }
        // Skip to next digit
        while (*p && (*p < '0' || *p > '9')) p++;
    }
}
*/

// MARK: ------------------------------ RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const int segment, const int* inRgbValue) {
    // Save handover color
    handoverColor[segment][0] = inRgbValue[0];
    handoverColor[segment][1] = inRgbValue[1];
    handoverColor[segment][2] = inRgbValue[2];

    // Calculate tuned values directly
    uint8_t r = applyGamma((inRgbValue[0] * tuneRatio[0] * maxBrightness) / 100);
    uint8_t g = applyGamma((inRgbValue[1] * tuneRatio[1] * maxBrightness) / 100);
    uint8_t b = applyGamma((inRgbValue[2] * tuneRatio[2] * maxBrightness) / 100);

    // Simplified PWM implementation - this removes the inner loop for significant savings
    // Set pins based on the calculated values
    analogWrite(leds[segment].red, r);
    analogWrite(leds[segment].green, g);
    analogWrite(leds[segment].blue, b);

    // Check buttons once per frame
    checkButtons();
}
