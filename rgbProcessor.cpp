#include <Arduino.h>
#include "types.h"
#include "swatches.h"

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;
uint8_t animIndex = 0;
uint8_t colorIndex = 0;

// Replace std::array with plain C arrays to save significant flash memory
float tuneRatio[3] = {1.0, 1.0, 1.0};
uint8_t handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}};

// Reference to the leds array defined in the main sketch
ledSegment led[3];

// MARK: Gamma Correction ------------------------------
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

// MARK: Luminance Calculation ------------------------------
// Function to calculate luminance ratios for consistent color output
void calculateLuminance() {
    // Convert to fixed-point to save flash by eliminating floating point operations
    // Scale values by 256 for 8-bit fixed-point math
    uint8_t rLum = (red.luminance << 8) / red.mA;
    uint8_t gLum = (green.luminance << 8) / green.mA;
    uint8_t bLum = (blue.luminance << 8) / blue.mA;

    // Find the maximum value
    uint8_t maxLum = rLum;
    if (gLum > maxLum) maxLum = gLum;
    if (bLum > maxLum) maxLum = bLum;

    // Calculate and store the tuning ratios
    tuneRatio[0] = (float)maxLum / rLum;
    tuneRatio[1] = (float)maxLum / gLum;
    tuneRatio[2] = (float)maxLum / bLum;
}

// MARK: Button Handling ------------------------------
void checkButtons() {
    // Check color button
    uint8_t colorButtonState = digitalRead(colorBtn);
    if (colorButtonState != colorButtonLastState) {
        if (colorButtonState == LOW) {
            swatchIndex = (swatchIndex + 1) % numSwatches;
        }
        colorButtonLastState = colorButtonState;
    }

    // Check animation button
    uint8_t animButtonState = digitalRead(animBtn);
    if (animButtonState != animButtonLastState) {
        if (animButtonState == LOW) {
            animIndex = (animIndex + 1) % numAnimations;
        }
        animButtonLastState = animButtonState;
    }
}

// MARK: RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]) {
    uint8_t tunedRGB[3];

    // Write the end color to the handover color matching the led segment
    for (uint8_t pin = 0; pin < 3; pin++) {
        handoverColor[segment][pin] = rgbValue[pin];
    }

    // Adjust the RGB values by the luminosity ratios, the brightness modifier, and apply gamma correction
    for (uint8_t pin = 0; pin < 3; pin++) {
        tunedRGB[pin] = gamma8[(uint8_t)(((rgbValue[pin] * tuneRatio[pin]) * maxBrightness) / 100)];
    }

    // Output the final values to the LED array
    for (uint8_t brightness = 0; brightness < 100; brightness++) {
        digitalWrite(led[segment].red,     brightness < tunedRGB[0] ? LOW : HIGH);
        digitalWrite(led[segment].green,   brightness < tunedRGB[1] ? LOW : HIGH);
        digitalWrite(led[segment].blue,    brightness < tunedRGB[2] ? LOW : HIGH);
    }

    // Check buttons once per frame
    checkButtons();
}
