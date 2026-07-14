#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations

// Available configurations — use one of these as the ACTIVE_CONFIG define in RGB-Slammer.ino
#define CONFIG_NANOFRAME      0
#define CONFIG_BREACH_KEY     1
#define CONFIG_AURORA_GLASYA  2
#define CONFIG_BLINDER_MINI   3
#define CONFIG_AG_ECHO_FRAME  4

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

#if ACTIVE_CONFIG == CONFIG_NANOFRAME
const PinConfig NANOFRAME_PINOUT = {
    .leds = {
        {PC1, PC2, PC3, false, 0, ROLE_GPIO}, // Seg 0: Core LED (direct GPIO)
        {0,   0,   0,   true,  0, ROLE_SR},  // Seg 1: SR ch0 — eye top-left
        {0,   0,   0,   true,  1, ROLE_SR},  // Seg 2: SR ch1 — eye bottom-left
        {0,   0,   0,   true,  2, ROLE_SR},  // Seg 3: SR ch2 — eye top-right
        {0,   0,   0,   true,  3, ROLE_SR},  // Seg 4: SR ch3 — eye bottom-right
    },
    .gpioLEDs = 5,
    .shiftReg    = {PD4, PD3, PD2}, // SER, RCLK, SRCLK
    .shiftRegChannels = 4,
    .colorButton        = PC5,
    .animButton         = PC4,
    .defaultBrightness  = 50,
    .pulseBrightness    = 255,
    .segOutputScale     = {110, 255, 255, 255, 255},
    .minBrightness      = 50,
    .maxBrightness      = 180,
    .slowDown           = 1,
    .redLed             = {5, 110},
    .greenLed           = {5, 100},
    .blueLed            = {5, 100}
};
#endif // CONFIG_NANOFRAME

#if ACTIVE_CONFIG == CONFIG_BREACH_KEY
const PinConfig BREACH_KEY_PINOUT = {
    .leds = {
        {PC4, PC5, PC6, false, 0, ROLE_GPIO}, // Seg 0: Upper LEDs (primary color)
        {PD4, PD5, PD3, false, 0, ROLE_SR},  // Seg 1: Lower LEDs (secondary color)
    },
    .gpioLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton        = PD2,
    .animButton         = PIN_NONE, // No animation button on this platform
    .defaultBrightness  = 120,
    .pulseBrightness    = 255,
    .segOutputScale     = {255, 255, 255, 255, 255},
    .minBrightness      = 80,
    .maxBrightness      = 180,
    .slowDown           = 1, // Slowing down the chip to improve stability and smooth color transitions
    .redLed             = {5, 110},
    .greenLed           = {5, 100},
    .blueLed            = {5, 100}
};
#endif // CONFIG_BREACH_KEY

#if ACTIVE_CONFIG == CONFIG_AURORA_GLASYA
const PinConfig AURORA_GLASYA_PINOUT = {
    .leds = {
        {PC4, PC5, PC6, false, 0, ROLE_GPIO}, // Seg 0: Upper LEDs (primary color)
        {PD4, PD5, PD3, false, 0, ROLE_SR},  // Seg 1: Lower LEDs (secondary color)
    },
    .gpioLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton        = PD2,
    .animButton         = PIN_NONE, // No animation button on this platform
    .defaultBrightness  = 120,
    .pulseBrightness    = 255,
    .segOutputScale     = {100, 255, 255, 255, 255},
    .minBrightness      = 80,
    .maxBrightness      = 180,
    .slowDown           = 1,
    .redLed             = {5, 110},
    .greenLed           = {5, 100},
    .blueLed            = {5, 100}
};
#endif // CONFIG_AURORA_GLASYA

#if ACTIVE_CONFIG == CONFIG_BLINDER_MINI
const PinConfig BLINDER_MINI_PINOUT = {
    .leds = {
        {PC4, PC5, PC6, false, 0, ROLE_GPIO}, // Seg 0: Upper LEDs
        {PD4, PD5, PD3, false, 0, ROLE_GPIO}, // Seg 1: Lower LEDs
    },
    .gpioLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton        = PD2,
    .animButton         = PC7,
    .defaultBrightness  = 180,
    .pulseBrightness    = 255,
    .segOutputScale     = {100, 100, 255, 255, 255},
    .minBrightness      = 80,
    .maxBrightness      = 200,
    .slowDown           = 1,
    .redLed             = {5, 110},
    .greenLed           = {5, 100},
    .blueLed            = {5, 100}
};
#endif // CONFIG_BLINDER_MINI

#if ACTIVE_CONFIG == CONFIG_AG_ECHO_FRAME
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .leds = {
        {PD7, PD6, PD5, false, 0, ROLE_GPIO}, // Seg 0: Eyepiece
        {PD2, PD3, PD4, false, 0, ROLE_GPIO}, // Seg 1: Main body
    },
    .gpioLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton        = PC3,
    .animButton         = PC0,
    .defaultBrightness  = 180,
    .pulseBrightness    = 255,
    .segOutputScale     = {100, 100, 255, 255, 255},
    .minBrightness      = 80,
    .maxBrightness      = 200,
    .slowDown           = 1,
    .redLed             = {5, 110},
    .greenLed           = {5, 100},
    .blueLed            = {5, 100}
};
#endif // CONFIG_AG_ECHO_FRAME

// Returns the single compiled-in hardware configuration
inline const PinConfig& getActiveConfig() {
#if ACTIVE_CONFIG == CONFIG_NANOFRAME
    return NANOFRAME_PINOUT;
#elif ACTIVE_CONFIG == CONFIG_BREACH_KEY
    return BREACH_KEY_PINOUT;
#elif ACTIVE_CONFIG == CONFIG_AURORA_GLASYA
    return AURORA_GLASYA_PINOUT;
#elif ACTIVE_CONFIG == CONFIG_BLINDER_MINI
    return BLINDER_MINI_PINOUT;
#else
    return AG_ECHO_FRAME_PINOUT;
#endif
}

#endif // HARDWARE_H
