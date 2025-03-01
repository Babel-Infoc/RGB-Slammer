#ifndef RGBPROCESSOR_H
#define RGBPROCESSOR_H
#include <algorithm>
#include "types.h"

// MARK: ------------------------------ Gamma Correction ------------------------------
// Define gamma8 array in the implementation section
const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,     0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,     1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,     2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,     5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9,    10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16,    16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24,    25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35,    36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,    50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67,    68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87,    89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,   114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,   142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,   175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,   213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,   255 };

//const uint8_t smoothing[256] PROGMEM = {
//    255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240,
//    239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 224,
//    223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208,
//    207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192,
//    191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176,
//    175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160,
//    159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,
//    143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128,
//    127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
//    111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96,
//    95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
//    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64,
//    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
//    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
//    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
//    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
//};


// MARK: ------------------------------ RGB Processing ------------------------------
// Convert the rgb string to an RGB Array
void rgbStringToArray(const char* rgbString, const int rgbArray[3]) {
    sscanf(rgbString, "rgb(%d,%d,%d)", &rgbArray[0], &rgbArray[1], &rgbArray[2]);
}

// MARK: ------------------------------ RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const ledSegment led, const int RGBValue[]) {
    float rRatio, gRatio, bRatio;
    int tunedRGB[3];

    // Write the end color to the handover color matching the led segment
    for (int i = 0; i < 3; i++) {
        handoverColor[led.ledNum][i] = RGBValue[i];
    }

    // Attenuate RGB values by the LEDs documented luminous intensity at the specified mA value
    float maxLum = max(max(redLum.lum, greenLum.lum), blueLum.lum);
    rRatio    = 1.0f - ((redLum.lum / maxLum) / redLum.mA);
    gRatio    = 1.0f - ((greenLum.lum / maxLum) / greenLum.mA);
    bRatio    = 1.0f - ((blueLum.lum / maxLum) / blueLum.mA);
    float tuneRatio[] = {rRatio, gRatio, bRatio};

    // Get the highest Ratio value, adjust it up to 1, and adjust the other two ratios up at the same rate
    float maxRatio = max(max(rRatio, gRatio), bRatio);
    float tuneratio = 1.0f / maxRatio;
    for (int i = 0; i < 3; i++) {
        tuneRatio[i] *= tuneratio;
    }
    // Adjust the RGB values by the luminosity ratios, the brightness modifier, and apply gamma correction
    for (int i = 0; i < 3; i++) {
        tunedRGB[i] = gamma8[(int)((RGBValue[i] * tuneRatio[i]) * maxBrightness)];
    }

    // Output the final values to the LED array
    for (int i = 0; i < 100; i++) {
        digitalWrite(led.redPin, i < tunedRGB[0] ?      LOW : HIGH);
        digitalWrite(led.greenPin, i < tunedRGB[1] ?    LOW : HIGH);
        digitalWrite(led.bluePin, i < tunedRGB[2] ?     LOW : HIGH);
    }
}


#endif // RGBPROCESSOR_H
