#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>

// Maximum total LED segments across all hardware configurations (GPIO + SR)
#define MAX_LED_SEGMENTS 5

// MARK: ------------------------------ Hardware structure definitions ------------------------------
// Struct for LED segment definition
struct ledSegment {
    int red;
    int green;
    int blue;
    bool    isSR;      // true = driven via shift register, false = direct GPIO
    uint8_t srChannel; // shift register channel index (ignored when isSR = false)
};

// Struct for shift register pin configuration
struct shiftRegPins {
    uint8_t ser;    // Serial data input
    uint8_t rclk;   // Register clock (latch)
    uint8_t srclk;  // Shift register clock
};

// Array of LED segments
extern ledSegment led[];

// Pin definitions
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern uint8_t numLEDs;

// Shift register configuration
extern shiftRegPins shiftReg;
extern uint8_t numShiftRegChannels;

// LED luminance information
struct luminance {
    uint8_t mA;
    uint8_t luminance;
};

// Brightness and luminance globals
extern float currentBrightness;
extern float pulseBrightness;
extern float coreOutputScale; // Post-gamma output scale for GPIO segments
extern float srOutputScale;   // Post-gamma output scale for SR segments
extern const luminance red;
extern const luminance green;
extern const luminance blue;

// MARK: ------------------------------ Global variables ------------------------------
// Handover color — one entry per segment (GPIO + SR)
extern uint8_t handoverColor[MAX_LED_SEGMENTS][3];

// CPU slowdown
extern const uint8_t slowDown;

// MARK: ------------------------------ Button handling ------------------------------
// Current color swatch
extern uint8_t colorIndex;

// Current animation mode
extern uint8_t animationMode;

// Swatch preview animation flag
extern bool swatchPreviewActive;

// Animation preview flag
extern bool animationPreviewActive;

// Brightness adjustment mode flag
extern bool brightnessAdjustMode;

// Brightness mode trigger time
extern const unsigned long brightnessModeTriggerTime;

// MARK: ------------------------------ Functions ------------------------------
// Shift register channel color buffer (defined in rgbProcessor.cpp)
extern uint8_t shiftRegColors[4][3];

void calculateLuminance();
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]);
void updateEyeAnimation();
void swatchPreview();
void animationPreview();
void brightnessAdjustmentMode();
#endif // TYPES_H
