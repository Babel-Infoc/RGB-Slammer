#ifndef TYPES_H
#define TYPES_H

#include <array>

// MARK: ------------------------------ Hardware Definitions ------------------------------
// Struct for LED segment definition
typedef struct {
    int red;             // Red pin
    int green;           // Green pin
    int blue;            // Blue pin
} ledSegment;

// Array of LED segments
extern ledSegment leds[];  // Changed from led[] to leds[] to match the definition in RGB-Slammer.ino

// Pin definitions
extern const int colorBtn;
extern const int animBtn;

// MARK: ------------------------------ External Variables ------------------------------
// Number of LED segments - changed from definition to declaration
extern const int numLEDs;  // Removed initialization here to avoid multiple definitions

// LED luminance information
struct luminance {
    int mA;
    int luminance;
};

// Brightness and luminance globals
extern const float maxBrightness;
extern const luminance red;
extern const luminance green;
extern const luminance blue;

// Handover color
extern std::array<std::array<int, 3>, 2> handoverColor;

// MARK: ------------------------------ Button Handling ------------------------------
// Current color swatch
extern int colorIndex;

// Current animation index
extern const int numAnimations;
extern int animIndex;

// MARK: ------------------------------ Swatch Handling ------------------------------
// Swatch struct
struct swatch {
    const char* highlight;
    const char* primary;
    const char* accent;
    const char* background;

    // Default constructor
    swatch() : highlight(nullptr), primary(nullptr), accent(nullptr), background(nullptr) {}

    // Constructor with parameters
    swatch(const char* h, const char* p, const char* a, const char* b)
        : highlight(h), primary(p), accent(a), background(b) {}
};

// Current color swatch index
extern int swatchIndex;
extern int swatchArraySize;

// Store the current swatch in a global variable
extern swatch currentSwatch;

// MARK: ------------------------------ Functions ------------------------------
void calculateLuminance();
void rgbStringToArray(const char* rgbString, std::array<int, 3>& rgbArray);
void sendToRGB(const int segment, const std::array<int, 3>& rgbValue);
void checkColorButton();
void checkAnimButton();
#endif // TYPES_H
