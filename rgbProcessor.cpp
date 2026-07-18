#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "flashStorage.h"

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;

uint8_t debounceStart = 0;

// Initialise preview states
bool swatchPreviewActive = false;
bool animationPreviewActive = false;
bool previewMode = false;

// Brightness adjustment mode variables
unsigned long buttonPressStartTime = 0;
bool buttonLongHold = false;

// Replace std::array with plain C arrays to save significant flash memory
// tuneRatio stores color tuning ratios as 0-255 (where 255 = 1.0)
uint8_t tuneRatio[3] = {0, 0, 0};
// Last gradient position and brightness sent to each zone
uint8_t handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}};

// Reference to the led array and buttons defined in the main sketch
extern ledZone led[2];
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern uint8_t currentBrightness;

// Global cache for the last RGB values sent to each zone
RgbCache globalCache;

// MARK: Gamma Correction
const uint8_t PROGMEM gamma8[] = {
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

// MARK: Luminance Calculation
// Function to calculate luminance ratios for consistent color output
void calculateLuminance() {
    // Calculate the effective luminance per mA for each LED (scaled to avoid overflow)
    uint16_t rEfficiency = (uint16_t)red.luminance * 100 / red.mA;
    uint16_t gEfficiency = (uint16_t)green.luminance * 100 / green.mA;
    uint16_t bEfficiency = (uint16_t)blue.luminance * 100 / blue.mA;

    // Find the most efficient LED (highest luminance per mA)
    uint16_t maxEfficiency = max(rEfficiency, max(gEfficiency, bEfficiency));

    // Scale all LEDs DOWN from the most efficient one, stored as 0-255 (where 255 = 1.0)
    // This ensures no LED is driven above its baseline
    tuneRatio[0] = (uint8_t)((uint16_t)rEfficiency * 255 / maxEfficiency); // Red ratio (≤ 255)
    tuneRatio[1] = (uint8_t)((uint16_t)gEfficiency * 255 / maxEfficiency); // Green ratio (≤ 255)
    tuneRatio[2] = (uint8_t)((uint16_t)bEfficiency * 255 / maxEfficiency); // Blue ratio (≤ 255)
}

// MARK: Button Handling
void checkButtons() {
    if (debounceStart > 0) {
        debounceStart--;
    } else {
        // Check color button
        uint8_t colorButtonState = digitalRead(colorBtn);

        if (colorButtonState != colorButtonLastState) {
            if (colorButtonState == LOW) {
                // Button just pressed - start timing
                buttonPressStartTime = millis();
                buttonLongHold = false;
            } else {
                // Button just released
                if (!buttonLongHold) {
                    // Short press - change swatch
                    currentSwatch = (currentSwatch + 1) % numSwatches;
                    saveSettingsToFlash(currentSwatch, currentBrightness, currentAnim);
                    swatchPreviewActive = true;
                } else {
                    // Long press was released - exit brightness mode and save brightness
                    brightnessAdjustMode = false;
                    // Save both swatch and current brightness to flash
                    saveSettingsToFlash(currentSwatch, currentBrightness, currentAnim);
                }
                buttonLongHold = false;
            }
            colorButtonLastState = colorButtonState;
        } else if (colorButtonState == LOW && !buttonLongHold && !brightnessAdjustMode) {
            // Button is being held - check if trigger time has passed
            if (millis() - buttonPressStartTime >= brightnessModeTriggerTime) {
                buttonLongHold = true;
                brightnessAdjustMode = true;
            }
        }

        // Check animation button
        uint8_t animButtonState = digitalRead(animBtn);

        if (animButtonState != animButtonLastState) {
            if (animButtonState == LOW) {
                // Button just pressed - cycle to next animation mode (flash save deferred to end of animationPreview)
                currentAnim = (currentAnim + 1) % numAnimationModes;
                animationPreviewActive = true;
            }
            animButtonLastState = animButtonState;
        }

        // Restart the debounce timer
        debounceStart = 10; // Reduced for more responsive long press detection
    }
}

// MARK: Color Pipeline

// MARK: calcGrad
Colour calcGrad(const uint8_t zone, uint8_t gradPoint, const uint8_t alpha) {
    Colour inputColor;
    Colour outputColor;
    uint8_t zone = gradPoint >> 6;      // Divide into 4 zones of 64 steps each
    uint8_t step = gradPoint & 0x3F;    // Get remainder (0-63)

    // Pointers to the 5 swatch colors
    const uint8_t* swcol1 = &swatch[currentSwatch].color0[0];
    const uint8_t* swcol2 = &swatch[currentSwatch].color1[0];
    const uint8_t* swcol3 = &swatch[currentSwatch].color2[0];
    const uint8_t* swcol4 = &swatch[currentSwatch].color4[0];
    const uint8_t* swcol5 = &swatch[currentSwatch].color4[0];

    const uint8_t* startColor;
    const uint8_t* endColor;

    switch (zone) {
        case 0:
            startColor = swcol1;
            endColor = swcol2;
            break;
        case 1:
            startColor = swcol2;
            endColor = swcol3;
            break;
        case 2:
            startColor = swcol3;
            endColor = swcol4;
            break;
        default:
            startColor = swcol4;
            endColor = swcol5;
            break;
    }

    uint8_t alpha = (uint16_t)step * 255 / 63; // Scale 0-63 to 0-255

    inputColor.r = ((uint16_t)endColor[1] * alpha + (uint16_t)startColor[1] * (255 - alpha)) >> 8;
    inputColor.g = ((uint16_t)endColor[2] * alpha + (uint16_t)startColor[2] * (255 - alpha)) >> 8;
    inputColor.b = ((uint16_t)endColor[3] * alpha + (uint16_t)startColor[3] * (255 - alpha)) >> 8;

    calcRGB(zone, inputColor, alpha);
    return outputColor;
}

// MARK: calcRGB
Colour calcRGB(const uint8_t zone, Colour inputColor, const uint16_t alpha) {
    Colour outputColor;
    // Calculate the actual output brightness modifier from 0 to currentBrightness, using alpha as the pointer
    uint16_t actualBrightness = (uint16_t)(((uint16_t)alpha * currentBrightness) / 65535);

    // Apply color tuning ratio to color-correct against the LEDs' relative efficiency
    inputColor.r = (uint16_t)(((uint16_t)inputColor.r * tuneRatio[0]) / 255);
    inputColor.g = (uint16_t)(((uint16_t)inputColor.g * tuneRatio[1]) / 255);
    inputColor.b = (uint16_t)(((uint16_t)inputColor.b * tuneRatio[2]) / 255);

    // Convert to final brightness according to user-selected brightness level
    inputColor.r = (uint16_t)(((uint16_t)inputColor.r * actualBrightness) / 255);
    inputColor.g = (uint16_t)(((uint16_t)inputColor.g * actualBrightness) / 255);
    inputColor.b = (uint16_t)(((uint16_t)inputColor.b * actualBrightness) / 255);

    // Apply gamma correction for perceptually even brightness steps
    outputColor.r = pgm_read_byte(&gamma8[inputColor.r]);
    outputColor.g = pgm_read_byte(&gamma8[inputColor.g]);
    outputColor.b = pgm_read_byte(&gamma8[inputColor.b]);

    return outputColor;
}

/*
// Send RGB values as PWM to GPIO
void writeRGB() {
    for (uint8_t zone = 0; zone < numLEDs; zone++) {
        // 4. CORRECT WAY TO RECALL: Read from the global instance array
        for (int step = 0; step < 256; step++) {
            digitalWrite(led[zone].red,   step < globalCache.zones[zone].rgb[0] ? LOW : HIGH);
            digitalWrite(led[zone].green, step < globalCache.zones[zone].rgb[1] ? LOW : HIGH);
            digitalWrite(led[zone].blue,  step < globalCache.zones[zone].rgb[2] ? LOW : HIGH);
        }
    }
}
*/
