#ifndef TYPES_H
#define TYPES_H

// Struct for LED segment definition
typedef struct {
    int redPin;             // Red pin
    int greenPin;           // Green pin
    int bluePin;            // Blue pin
    // Potential future fields:
    // uint8_t brightness;  // Per-segment brightness
    // bool enabled;        // Enable/disable state
} led_t;

// Array of LED segments
extern led_t leds[];

// LED luminance information
struct luminance {
    int mA;
    int lum;
};

// Number of LED segments
extern const int numLEDs;

// Forward declarations of globals
extern const float maxBrightness;
extern const luminance redLum;
extern const luminance greenLum;
extern const luminance blueLum;

// Handover color
extern int handoverColor[2][3];

#endif // TYPES_H
