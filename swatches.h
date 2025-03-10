#ifndef SWATCHES_H
#define SWATCHES_H
#include <Arduino.h>

// MARK: ------------------------------ Swatch definitions ------------------------------
// Swatch structure, 4 RGB arrays
struct swatch {
    uint8_t highlight[3];
    uint8_t primary[3];
    uint8_t ambient[3];
    uint8_t accent[3];
    uint8_t background[3];
};

// Index of the current swatch
const uint8_t swatchSize = 5;   // Number of colors in each swatch
extern uint8_t numSwatches;     // Number of swatches in the collection (calculated from array size)
extern swatch swArr[];    // Remove size specification to allow auto-calculation
extern uint8_t swInx;

// Bootswatch definition
extern const uint8_t bootswatch[5][3];

#endif // SWATCHES_H
