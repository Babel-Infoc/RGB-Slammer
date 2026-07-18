#ifndef TYPES_H
#define TYPES_H
#include <Arduino.h>

// MARK: Hardware structure definitions
// Struct for LED zone definition
struct ledZone {
    int red;
    int green;
    int blue;
};
extern ledZone led[];

// A gradient position + brightness pair, used to describe one zone's
// fade target (see fadeToColor in RGB-Slammer.ino). colors[0] = zone 0,
// colors[1] = zone 1, and so on.
struct ZoneColor {
    uint8_t grad;
    uint8_t alpha;
};

// MARK: Animation base types
class Animation;
typedef Position;
struct Colour{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Pin definitions
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern const uint8_t numLEDs;

// LED luminance information
struct luminance {
    uint8_t mA;
    uint8_t luminance;
};

// Current displayed RGB colors for all zones
struct rgbCache {
    uint8_t zone;
    Colour rgb;
};

// Brightness and luminance globals (brightness values are 0-255)
extern uint8_t currentBrightness;
extern uint8_t minBrightness;
extern uint8_t maxBrightness;
extern luminance red;
extern luminance green;
extern luminance blue;

// MARK: Global variables
// Handover color
extern rgbCache handoverColor;

// CPU slowdown
extern uint8_t slowDown;

// MARK: Button handling

// Current animation mode
extern uint8_t animationMode;

// Swatch preview animation flag
extern bool swatchPreviewActive;

// Animation preview flag
extern bool animationPreviewActive;

// Internal guard to allow preview functions to run without self-interrupting
extern bool previewMode;

// Brightness adjustment mode flag
extern bool brightnessAdjustMode;

// Brightness mode trigger time
extern const unsigned long brightnessModeTriggerTime;

// MARK: Functions
void calculateLuminance();
Colour calcRGB(const uint8_t zone, const uint16_t gradPoint, const uint8_t alpha, Colour inputColor);
Colour calcGrad(uint8_t gradPoint);
void swatchPreview();
void animationPreview();
void brightnessAdjustmentMode();
void writeRGB();
void checkButtons();
// Gradient calculation
void gradientPosition(uint8_t position, uint8_t output[3]);

#endif // TYPES_H
