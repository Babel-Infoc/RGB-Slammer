#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>

// Maximum total LED segments across all hardware configurations (GPIO + SR)
#define MAX_LED_SEGMENTS 5

// Sentinel value meaning "no pin assigned" — safe because CH32V003 valid pin numbers are well below 255
#define PIN_NONE 0xFF

// MARK: ------------------------------ Hardware structure definitions ------------------------------
// LED segment role — determines which animation colour target a segment belongs to
#define ROLE_GPIO 0   // LEDs attached directly to GPIO pins, driven by software PWM in sendToRGB()
#define ROLE_SR   1   // LEDs attached to shift registers, driven by hardware PWM via shift register channel color buffers and srUpdateCallback()

// Struct for LED segment definition
struct ledSegment {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    bool    isSR;      // true = driven via shift register, false = direct GPIO
    uint8_t srChannel; // shift register channel index (ignored when isSR = false)
    uint8_t role;      // ROLE_GPIO or ROLE_SR
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
extern uint8_t gpioLEDs;

// Shift register configuration
extern shiftRegPins shiftReg;
extern uint8_t srLEDs;

// LED luminance information
struct luminance {
    uint8_t mA;
    uint8_t luminance;
};

// Hardware pin and tuning configuration for one target platform
struct PinConfig {
    ledSegment leds[MAX_LED_SEGMENTS]; // GPIO segments
    uint8_t gpioLEDs;                  // Number of GPIO LED segments
    shiftRegPins shiftReg;             // Shift register control pins (SER, RCLK, SRCLK)
    uint8_t shiftRegChannels;          // Number of RGB LED channels on the shift register (0 = none)
    uint8_t colorButton;
    uint8_t animButton;
    // Hardware-specific brightness and LED tuning
    uint8_t defaultBrightness;                // Initial brightness on boot (0-255)
    uint8_t pulseBrightness;                  // Brightness used during preview animations (0-255)
    uint8_t segOutputScale[MAX_LED_SEGMENTS]; // Per-segment post-gamma output scale (0-255)
    uint8_t minBrightness;                    // Lower limit for brightness adjust mode (0-255)
    uint8_t maxBrightness;                    // Upper limit for brightness adjust mode (0-255)
    uint8_t slowDown;                         // PWM loop delay in ms
    luminance redLed;                         // Red LED luminance calibration {mA, luminosity}
    luminance greenLed;                       // Green LED luminance calibration {mA, luminosity}
    luminance blueLed;                        // Blue LED luminance calibration {mA, luminosity}
};

// Brightness and luminance globals — 8-bit fixed-point: 0-255 represents 0.0-1.0
extern uint8_t currentBrightness;
extern uint8_t pulseBrightness;
extern uint8_t segOutputScale[MAX_LED_SEGMENTS]; // Per-segment post-gamma output scale (0-255)
extern luminance red;   // Initialised from active hardware config at boot
extern luminance green;
extern luminance blue;

// MARK: ------------------------------ Global variables ------------------------------
// Handover color — one entry per segment (GPIO + SR)
extern uint8_t handoverColor[MAX_LED_SEGMENTS][3];

// CPU slowdown — initialised from active hardware config at boot
extern uint8_t slowDown;

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
