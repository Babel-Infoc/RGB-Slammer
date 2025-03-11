#ifndef SWATCHES_H
#define SWATCHES_H
#include <Arduino.h>

// MARK: ------------------------------ Swatch definitions ------------------------------
// Swatch structure, 4 RGB arrays
struct swatchArray {
    uint8_t primary[3];
    uint8_t accent[3];
    uint8_t midtone[3];
    uint8_t contrast[3];
    uint8_t background[3];
};

// Structure to represent an RGB color
struct rgb_t{
    uint8_t r;  // Red component (0-255)
    uint8_t g;  // Green component (0-255)
    uint8_t b;  // Blue component (0-255)
};

// Index of the current swatch
const uint8_t swatchSize = 5;   // Number of colors in each swatch
extern uint8_t numSwatches;     // Number of swatches in the collection (calculated from array size)
extern swatchArray swatch[];    // Remove size specification to allow auto-calculation
extern uint8_t swNum;

// Bootswatch definition
extern const uint8_t bootswatch[5][3];

// Gradient calculation
rgb_t gradientPosition(uint8_t position);

#endif // SWATCHES_H
