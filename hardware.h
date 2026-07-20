#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations.
// The active configuration is selected by hardwarePlatform in the main sketch.

// Configuration for Nanoframe
// Two SN74HC595 shift registers (U19, U20) driven from PD4/PD3/PD2.
// MSBFIRST: bit0→QA, bit7→QH on each register.
//
// U19 (SR1, second shiftOut — stays in SR1):
//   QA=RED1  QB=GREEN1  QC=BLUE1   ← Channel 2
//   QD=RED2  QE=GREEN2  QF=BLUE2   ← Channel 3
//   QG=BTN1  QH=BTN2               ← Button signal outputs (kept LOW by sendToShiftReg)
//
// U20 (SR2, first shiftOut — pushed through SR1 into SR2):
//   QA=RED3  QB=GREEN3  QC=BLUE3   ← Channel 4
//   QD=RED4  QE=GREEN4  QF=BLUE4   ← Channel 5
//   QG=unused  QH=unused

const hardwareConfig NANOFRAME = {
    .zone = {
        {PC1, PC2, PC3, 0, GPIOLED,  CORELED,       110}, // Seg 0: Core LED (direct GPIOLED)
        {0,   0,   0,   0, SHIFTREG, EYETOPLEFT,    255},  // Seg 1: SR ch0 — eye top-left
        {0,   0,   0,   1, SHIFTREG, EYEBOTTOMLEFT, 255},  // Seg 2: SR ch1 — eye bottom-left
        {0,   0,   0,   2, SHIFTREG, EYETOPRIGHT,   255},  // Seg 3: SR ch2 — eye top-right
        {0,   0,   0,   3, SHIFTREG, EYEBOTTOMRIGHT,255},  // Seg 4: SR ch3 — eye bottom-right
    },
    .shiftReg    = {PD4, PD3, PD2}, // SER, RCLK, SRCLK
    .colorButton        = PC5,
    .animButton         = PC4,
    .defaultBrightness  = 50,
    .minBrightness      = 50,
    .maxBrightness      = 180,
    .slowDown           = 1,
    .tuning             = {
        {5, 110},
        {5, 100},
        {5, 100}
    }
};

const hardwareConfig BREACH_KEY = {
    .zone = {
        {PC4, PC5, PC6, 0, GPIOLED, CORELED, 255}, // Seg 0: Upper LEDs (primary color)
        {PD4, PD5, PD3, 0, GPIOLED, ACCTLED, 255}, // Seg 1: Lower LEDs (secondary color)
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
    },
    .shiftReg    = {0, 0, 0},
    .colorButton        = PD2,
    .animButton         = PIN_NONE, // No animation button on this platform
    .defaultBrightness  = 120,
    .minBrightness      = 80,
    .maxBrightness      = 180,
    .slowDown           = 1, // Slowing down the chip to improve stability and smooth color transitions
    .tuning             = {
        {5, 110},
        {5, 100},
        {5, 100}
    }
};

const hardwareConfig AURORA_GLASYA = {
    .zone = {
        {PC4, PC5, PC6, 0, GPIOLED, CORELED, 100}, // Seg 0: Upper LEDs (primary color)
        {PD4, PD5, PD3, 0, GPIOLED, ACCTLED, 255}, // Seg 1: Lower LEDs (secondary color)
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
    },
    .shiftReg    = {0, 0, 0},
    .colorButton        = PD2,
    .animButton         = PIN_NONE, // No animation button on this platform
    .defaultBrightness  = 120,
    .minBrightness      = 80,
    .maxBrightness      = 180,
    .slowDown           = 1,
    .tuning             = {
        {5, 110},
        {5, 100},
        {5, 100}
    }
};

const hardwareConfig NANOSHARD = {
    .zone = {
        {PC6, PC5, PC4, 0, GPIOLED, CORELED, 255}, // Seg 0: Upper LEDs
        {PD5, PD4, PD3, 0, GPIOLED, ACCTLED, 255}, // Seg 1: Lower LEDs
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
    },
    .shiftReg    = {0, 0, 0},
    .colorButton        = PD6,
    .animButton         = PD2,
    .defaultBrightness  = 100,
    .minBrightness      = 40,
    .maxBrightness      = 100,
    .slowDown           = 1,
    .tuning             = {
        {5, 110},
        {5, 100},
        {5, 100}
    }
};

const hardwareConfig AG_ECHO_FRAME = {
    .zone = {
        {PD7, PD6, PD5, 0, GPIOLED, CORELED, 255}, // Seg 0: Eyepiece
        {PD2, PD3, PD4, 0, GPIOLED, ACCTLED, 255}, // Seg 1: Main body
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
        {0,   0,   0,   0, NOTUSED, NOTUSED, 255}, // Not used
    },
    .shiftReg    = {0, 0, 0},
    .colorButton        = PC3,
    .animButton         = PC0,
    .defaultBrightness  = 160,
    .minBrightness      = 80,
    .maxBrightness      = 180,
    .slowDown           = 1,
    .tuning             = {
        {5, 110},
        {5, 100},
        {5, 100}
    }
};

#endif // HARDWARE_H
