#ifndef ENVELOPES_H
#define ENVELOPES_H
#include <Arduino.h>

// MARK: ------------------------------ Envelope definitions ------------------------------
// Envelope structure, array of 32 brightness values, 0-255
struct envelopeArray {
    uint8_t envelope[32];
};

// Master envelope array
extern envelopeArray envelope[];

#endif // ENVELOPES_H
