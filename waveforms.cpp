#include "waveforms.h"
#include <Arduino.h>

// MARK: Master waveform array ------------------------------
// Envelope structure, array of 32 brightness values, 0-255, for adjusting brightness or gradient position
waveformArray waveform[] = {
    {   // Unstable
        0, 12, 5, 30, 18, 45, 27, 70, 50, 85, 60, 110, 90, 125, 105, 150,
        130, 165, 145, 180, 160, 195, 170, 210, 185, 225, 200, 235, 215, 245, 230, 255
    },
    {   // Sine wave
        128, 153, 177, 199, 219, 234, 246, 254, 255, 254, 246, 234, 219, 199, 177, 153,
        128, 103, 79, 57, 37, 22, 10, 2, 0, 2, 10, 22, 37, 57, 79, 103
    }
};
