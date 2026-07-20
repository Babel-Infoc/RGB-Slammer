#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>

// Sentinel value meaning "no pin assigned" — safe because CH32V003 valid pin numbers are well below 65535
#define PIN_NONE 0xFF

extern uint8_t numZones;

// MARK: Hardware structure definitions
// LED zone type — determines how to process colour output for a zone
#define GPIOLED 0    // LEDs attached directly to GPIOLED pins, driven by software PWM in sendToRGB()
#define SHIFTREG 1   // LEDs attached to shift registers, driven by hardware PWM via shift register channel color buffers and srUpdateCallback()
#define NOTUSED 2    // Null out zones not used on that platform

// LED zone type — determines what kinds of color / animation should be sent to that zone
#define CORELED 0
#define ACCTLED 1
#define EYETOPLEFT 2
#define EYEBOTTOMLEFT 3
#define EYETOPRIGHT 4
#define EYEBOTTOMRIGHT 5

struct pin {
    uint8_t red;
    uint8_t grn;
    uint8_t blu;
};

// Struct for LED zone definition
struct zoneConfig {
    uint8_t r, g, b;
    uint8_t srChannel; // shift register channel index (ignored when led[zone].type == GPIOLED)
    uint8_t type;      // GPIOLED or SHIFTREG
    uint8_t role;      // CORELED, ACCTLED, etc.
    uint16_t scale;    // Brightness scale for this zone. Scale this according to how many LEDs are physically present in the zone. 65535 = full brightness, 128 = half brightness, etc.
};

// Struct for shift register pin configuration
struct shiftRegPins {
    uint8_t ser;    // Serial data input
    uint8_t rclk;   // Register clock (latch)
    uint8_t srclk;  // Shift register clock
};

// Array of LED zones
// Pin definitions
extern uint8_t colorBtn;
extern uint8_t animBtn;

// Shift register configuration
extern shiftRegPins shiftReg;
extern uint8_t srLEDs;

// LED luminance information
struct rgbTuner {
    uint16_t mA;
    uint16_t luminance;
};

// Hardware pin and tuning configuration for one target platform
struct hardwareConfig {
    zoneConfig zone[5];         // LED zones
    shiftRegPins shiftReg;      // Shift register control pins (SER, RCLK, SRCLK)
    uint8_t colorButton;        // Swatch change button
    uint8_t animButton;         // Animation change button
    uint8_t defaultBrightness;  // Initial brightness on boot (0-65535)
    uint8_t zoneOutputScale;    // Per-zone post-gamma output scale (0-65535)
    uint8_t minBrightness;      // Lower limit for brightness adjust mode (0-65535)
    uint8_t maxBrightness;      // Upper limit for brightness adjust mode (0-65535)
    uint8_t slowDown;           // PWM loop delay in ms
    uint8_t tuning[3][2];       // values for white balance adjustment
};

// Brightness and luminance globals — 8-bit fixed-point: 0-65535 represents 0.0-1.0
extern uint8_t currentBrightness;
extern uint8_t zoneOutputScale[5]; // Per-zone post-gamma output scale (0-65535)
extern uint8_t tuning[3][2];

// MARK: Global variables
// Handover color — one entry per zone (GPIOLED + SR)
extern uint8_t handoverColor[5][3];

// CPU slowdown — initialised from active hardware config at boot
//extern uint8_t slowDown;

// MARK: Button handling
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
extern const uint16_t brightnessModeTriggerTime;

// MARK: Functions
// Shift register channel color buffer (defined in rgbProcessor.cpp)
extern uint8_t shiftRegColors[4][3];

void calculateLuminance();
void sendToRGB(const uint8_t zone, const uint8_t rgbValue[3]);
void eyeDoublePulse();
void swatchPreview();
void animationPreview();
void brightnessAdjustmentMode();
#endif // TYPES_H
