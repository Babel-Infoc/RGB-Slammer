#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h> 

// MARK: ------------------------------ Hardware structure definitions ------------------------------
// Struct for LED segment definition
struct ledSegment {
    int red;
    int green;
    int blue;
};

// Array of LED segments
extern ledSegment led[];

// Pin definitions
extern const uint8_t numLEDs;
extern const uint8_t colorBtn;
extern const uint8_t animBtn;
extern const uint8_t lightSensor;
extern float brightnessMod;
extern const bool sensorEnabled;

// LED luminance information
struct luminance {
    uint8_t mA;
    uint8_t luminance;
};

// Brightness and luminance globals
extern const float maxBrightness;
extern const luminance red;
extern const luminance green;
extern const luminance blue;

// MARK: ------------------------------ Global variables ------------------------------
// Handover color
extern uint8_t handoverColor[2][3];

// MARK: ------------------------------ Button handling ------------------------------
// Current color swatch
extern uint8_t colorIndex;

// Current animation index
extern const uint8_t numAnimations;
extern uint8_t animIndex;

// MARK: ------------------------------ Functions ------------------------------
void calculateLuminance();
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]);
#endif // TYPES_H
