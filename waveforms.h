#ifndef ENVELOPES_H
#define ENVELOPES_H
#include <Arduino.h>

// MARK: ------------------------------ Envelope definitions ------------------------------
// Envelope structure, array of 32 brightness values, 0-255
struct waveformArray {
    uint8_t waveform[32];
};

// Master waveform array
extern waveformArray waveform[];

#endif // ENVELOPES_H
