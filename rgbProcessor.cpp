#include <Arduino.h>
#include <algorithm>
#include "swatches.h"
#include "types.h"

// Initialize the last state of the color button
int colorButtonLastState = HIGH;
int animButtonLastState = HIGH;

// Initialize the tuned RGB array and tune ratio array
float tuneRatio[3] = {1.0, 1.0, 1.0};  // Initialize with default values

// Initialize the handover color array
int handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}}; // Definition for extern variable from types.h

// MARK: ------------------------------ luminance Calculations ------------------------------
// Attenuate RGB values by the LEDs documented luminous intensity at the specified mA value
void calculateLuminance() {
    // Attenuate RGB values by the LEDs documented luminous intensity at the specified mA value
    float highestLuminance = max(max(red.luminance, green.luminance), blue.luminance);
    tuneRatio[0] = 1.0f - ((red.luminance / highestLuminance) / red.mA);
    tuneRatio[1] = 1.0f - ((green.luminance / highestLuminance) / green.mA);
    tuneRatio[2] = 1.0f - ((blue.luminance / highestLuminance) / blue.mA);

    // Get the highest Ratio value, adjust it up to 1, and adjust the other two ratios up at the same rate
    float maxRatio = max(max(tuneRatio[0], tuneRatio[1]), tuneRatio[2]);
    float tuneratio = 1.0f / maxRatio;
    for (int i = 0; i < 3; i++) {
        tuneRatio[i] *= tuneratio;
    }
}

// MARK: ------------------------------ Gamma Correction ------------------------------
// Define gamma8 array in the implementation section
const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
    2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,   5,
    5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,   9,
    9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15,  15,
   16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23,  24,
   24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34,  35,
   35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  49,
   50, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,  66,
   67, 68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85,  86,
   87, 89, 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109, 110,
  112,114,115,117,119,120,122,124,126,127,129,131,133,135,137, 138,
  140,142,144,146,148,150,152,154,156,158,160,162,164,167,169, 171,
  173,175,177,180,182,184,186,189,191,193,196,198,200,203,205, 208,
  210,213,215,218,220,223,225,228,231,233,236,239,241,244,247, 249,
  252,255 };

// MARK: ------------------------------ Button Handling ------------------------------

// Function to check if color button has been pressed
void checkColorButton() {
    // Read the current button state
    int colorButtonState = digitalRead(colorBtn);

    // Check if the button state has changed
    if (colorButtonState != colorButtonLastState) {
        // Increment the swatch index only if the button state is LOW, wrapping around if exceeded the maximum
        if (colorButtonState == LOW) {
            // Use the number of available swatches (6) from swatchArray
            swatchIndex = (swatchIndex + 1) % 6;

            // When color button is pressed, copy the selected swatch's colors to currentSwatch
            currentSwatch.highlight = swatchArray[swatchIndex].highlight;
            currentSwatch.primary = swatchArray[swatchIndex].primary;
            currentSwatch.accent = swatchArray[swatchIndex].accent;
            currentSwatch.background = swatchArray[swatchIndex].background;
        }
        // Save the current button state for next comparison
        colorButtonLastState = colorButtonState;
    }
}

void checkAnimButton() {
    // Read the current button state
    int animButtonState = digitalRead(animBtn);

    // Check if the button state has changed
    if (animButtonState != animButtonLastState) {
        // Increment the animation index only if the button state is LOW, wrapping around if exceeded the maximum
        if (animButtonState == LOW) {
            animIndex = (animIndex + 1) % numAnimations;
        }
        // Save the current button state for next comparison
        animButtonLastState = animButtonState;
    }
}

// MARK: ------------------------------ RGB Processing ------------------------------
// Convert the rgb or rgba string to an RGB Array
void rgbStringToArray(const char* rgbString, int rgbArray[3]) {
    if (sscanf(rgbString, "rgb(%d,%d,%d)", &rgbArray[0], &rgbArray[1], &rgbArray[2]) != 3) {
        // If the provided rgb string includes an alpha value, fuck that shit off
        sscanf(rgbString, "rgba(%d,%d,%d,%*f)", &rgbArray[0], &rgbArray[1], &rgbArray[2]);
    }
}

void sendToRGB(const int segment, const int rgbValue[3]) {
    int tunedRGB[3];

    // Write the end color to the handover color matching the led segment
    for (int pin = 0; pin < 3; pin++) {
        handoverColor[segment][pin] = rgbValue[pin];
    }

    // Adjust the RGB values by the luminosity ratios, the brightness modifier, and apply gamma correction
    for (int pin = 0; pin < 3; pin++) {
        tunedRGB[pin] = gamma8[(int)(((rgbValue[pin] * tuneRatio[pin]) * maxBrightness) / 100)];
    }

    // Output the final values to the LED array
    for (int pin = 0; pin < 3; pin++) {
        for (int brightness = 0; brightness < 100; brightness++) {
            digitalWrite(ledSegment[segment][pin], brightness < tunedRGB[pin] ? LOW : HIGH);
        }
    }

    // Check buttons for updates
    checkColorButton();
    checkAnimButton();
}
