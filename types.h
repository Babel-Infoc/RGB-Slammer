#ifndef TYPES_H
#define TYPES_H

// LED segment structure
struct ledSegment {
    int ledNum;
    int redPin;
    int greenPin;
    int bluePin;
};

// LED luminance information
struct luminance {
    int mA;
    int lum;
};

// Forward declarations of globals (these will be defined in the main sketch)
extern int** handoverColor;
extern const float maxBrightness;
extern const luminance redLum;
extern const luminance greenLum;
extern const luminance blueLum;

// Struct to hold color information with hue and brightness
struct ColorInfo {
    const int* color;
    float hue;
    float brightness;
};

// Store the light sensor value (declaration only, definition is in the main sketch)
extern float ambientBrightness;

#endif // TYPES_H
