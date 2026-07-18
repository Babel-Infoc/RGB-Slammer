#ifndef SWATCHES_H
#define SWATCHES_H
#include <Arduino.h>

// MARK: ------------------------------ Swatch definitions ------------------------------
// Swatch structure, 4 RGB arrays
struct swatchArray {
    uint8_t color0[3];
    uint8_t color1[3];
    uint8_t color2[3];
    uint8_t color4[3];
    uint8_t color4[3];
};

// Index of the current swatch
const uint8_t swatchSize = 5;   // Number of colors in each swatch
extern const uint8_t numSwatches;     // Number of swatches in the collection (calculated from array size)
extern const swatchArray swatch[];    // Remove size specification to allow auto-calculation
extern uint8_t currentSwatch;

#endif // SWATCHES_H
