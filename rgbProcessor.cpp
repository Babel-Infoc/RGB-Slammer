#include <Arduino.h>
#include <algorithm>
#include <array>
#include "swatches.h"
#include "types.h"

// Initialize the last state of the color button
int colorButtonLastState = HIGH;
int animButtonLastState = HIGH;

// Initialize the tuned RGB array and tune ratio array
std::array<float, 3> tuneRatio = {1.0, 1.0, 1.0};  // Initialize with default values

// Initialize the handover color array
std::array<std::array<int, 3>, 2> handoverColor = {{{0, 0, 0}, {0, 0, 0}}}; // Definition for extern variable from types.h

// Reference to the leds array defined in the main sketch
extern ledSegment leds[];

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
void checkColorButton() {
    // Read the current button state
    int colorButtonState = digitalRead(colorBtn);

    // Check if the button state has changed
    if (colorButtonState != colorButtonLastState) {
        // Increment the swatch index only if the button state is LOW, wrapping around if exceeded the maximum
        if (colorButtonState == LOW) {
            // Use numSwatches from swatches.h instead of hard-coded value
            swatchIndex = (swatchIndex + 1) % numSwatches;

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
void rgbStringToArray(const char* rgbString, std::array<int, 3>& rgbArray) {
    if (sscanf(rgbString, "rgb(%d,%d,%d)", &rgbArray[0], &rgbArray[1], &rgbArray[2]) != 3) {
        // If the provided rgb string includes an alpha value, fuck that shit off
        sscanf(rgbString, "rgba(%d,%d,%d,%*f)", &rgbArray[0], &rgbArray[1], &rgbArray[2]);
    }
}

// MARK: ------------------------------ RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const int segment, const std::array<int, 3>& inRgbValue) {
    float rRatio, gRatio, bRatio;
    std::array<int, 3> tunedRGB;

    // Assign the input RGB values to the handover color array
    handoverColor[segment] = inRgbValue;

    // Attenuate RGB values by the LEDs documented luminous intensity at the specified mA value
    float maxLum = max(max(red.luminance, green.luminance), blue.luminance);
    rRatio    = 1.0f - ((red.luminance / maxLum) / red.mA);
    gRatio    = 1.0f - ((green.luminance / maxLum) / green.mA);
    bRatio    = 1.0f - ((blue.luminance / maxLum) / blue.mA);
    tuneRatio = {rRatio, gRatio, bRatio};

    // Get the highest Ratio value, adjust it up to 1, and adjust the other two ratios up at the same rate
    float maxRatio = max(max(rRatio, gRatio), bRatio);
    float tuneratio = 1.0f / maxRatio;
    for (int pin = 0; pin < 3; pin++) {
        tuneRatio[pin] *= tuneratio;
    }

    // Adjust the RGB values by the luminosity ratios, the brightness modifier, and apply gamma correction
    for (int pin = 0; pin < 3; pin++) {
        tunedRGB[pin] = gamma8[(int)((inRgbValue[pin] * tuneRatio[pin]) * maxBrightness / 100)]; // Divide by 100 to scale properly
    }

    // Output the final values to the LED array
    for (int brightness = 0; brightness < 100; brightness++) {
        digitalWrite(leds[segment].red,     brightness < tunedRGB[0] ? LOW : HIGH);
        digitalWrite(leds[segment].green,   brightness < tunedRGB[1] ? LOW : HIGH);
        digitalWrite(leds[segment].blue,    brightness < tunedRGB[2] ? LOW : HIGH);
    }

    // Check buttons for updates
    checkColorButton();
    checkAnimButton();
}
