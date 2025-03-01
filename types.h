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

// Number of LED segments
extern const int numSegments;

// Forward declarations of globals
extern const float maxBrightness;
extern const luminance redLum;
extern const luminance greenLum;
extern const luminance blueLum;

// Handover color
extern int handoverColor[2][3];

#endif // TYPES_H
