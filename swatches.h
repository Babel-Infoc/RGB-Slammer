#ifndef SWATCHES_H
#define SWATCHES_H
#include <Arduino.h> 

// MARK: ------------------------------ Swatch definitions ------------------------------
// Swatch structure, 4 RGB arrays
struct swatch {
    uint8_t highlight[3];
    uint8_t primary[3];
    uint8_t accent[3];
    uint8_t background[3];
};

// Index of the current swatch
const uint8_t swatchSize = 4;   // Number of colors in each swatch
const uint8_t numSwatches = 4;  // Number of swatches in the collection
extern swatch swatchArray[numSwatches];
extern uint8_t swatchIndex;

// Bootswatch definition
extern const uint8_t bootswatch[5][3];

#endif // SWATCHES_H
