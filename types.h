#ifndef TYPES_H
#define TYPES_H

// MARK: ------------------------------ Hardware Definitions ------------------------------
// Array for LED segment definition
extern int ledSegment[2][3];

// Pin definitions
extern const int colorBtn;
extern const int animBtn;

// MARK: ------------------------------ External Variables ------------------------------
// Number of LED segments
extern const int numLEDs;

// LED luminance information
struct luminance {
    int mA;
    int luminance;
};

// Brightness and luminance globals
extern const int maxBrightness;
extern const luminance red;
extern const luminance green;
extern const luminance blue;

// Handover color
extern int handoverColor[2][3];

// MARK: ------------------------------ Swatch Handling ------------------------------
// Swatch struct
struct swatch {
    char* highlight;
    char* primary;
    char* accent;
    char* background;

    // Default constructor
    swatch() : highlight(nullptr), primary(nullptr), accent(nullptr), background(nullptr) {}
};

// Current color swatch index
extern int swatchIndex;
extern int swatchArraySize;

// Store the current swatch in a global variable
struct swatch* currentSwatch;

// MARK: ------------------------------ Button Handling ------------------------------
// Current animation index
extern const int numAnimations;
extern int animIndex;

// Function to check button states and update indexes
void checkColorButton();
void checkAnimButton();

// MARK: ------------------------------ Gamma Correction ------------------------------
// Declare gamma8 array (definition is in the .cpp file)
extern const uint8_t gamma8[];

// MARK: ------------------------------ RGB Processing Functions ------------------------------
void calculateLuminance();

// Convert the rgb or rgba string to an RGB Array
void rgbStringToArray(const char* rgbString, int rgbArray[3]);

// Process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const int segment, const int rgbValue[3]);

#endif // TYPES_H
