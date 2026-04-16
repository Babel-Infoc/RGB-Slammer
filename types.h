#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>

// Maximum total LED segments across all hardware configurations (GPIO + SR)
#define MAX_LED_SEGMENTS 5

// MARK: ------------------------------ Hardware structure definitions ------------------------------
// LED segment role — determines which animation colour target a segment belongs to
#define ROLE_CORE 0   // Directly-driven GPIO LED (core)
#define ROLE_EXT  1   // Shift-register eye pod

// Struct for LED segment definition
struct ledSegment {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    bool    isSR;      // true = driven via shift register, false = direct GPIO
    uint8_t srChannel; // shift register channel index (ignored when isSR = false)
    uint8_t role;      // ROLE_CORE or ROLE_EXT
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

// Brightness and luminance globals — 8-bit fixed-point: 0-255 represents 0.0-1.0
extern uint8_t currentBrightness;
extern uint8_t pulseBrightness;
extern uint8_t coreOutputScale; // Post-gamma output scale for GPIO segments (0-255)
extern uint8_t srOutputScale;   // Post-gamma output scale for SR segments (0-255)
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
void sendToRole(uint8_t role, const uint8_t color[3]);
void eyeDoublePulse();
void swatchPreview();
void animationPreview();
void brightnessAdjustmentMode();
#endif // TYPES_H
