#ifndef PINOUTS_H
#define PINOUTS_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations

// Available configurations as enum for easier selection
enum ConfigType {
    NANOFRAME,
    AURORA_GLASYA,
    BLINDER_MINI,
    AG_ECHO_FRAME
};

struct PinConfig {
    ledSegment leds[MAX_LED_SEGMENTS]; // All segments: GPIO first, then SR
    uint8_t coreLEDs;           // Total segments (GPIO + SR)
    shiftRegPins shiftReg;     // Shift register control pins (SER, RCLK, SRCLK)
    uint8_t shiftRegChannels;  // Number of RGB LED channels on the shift register (0 = none)
    uint8_t colorButton;
    uint8_t animButton;
};

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
const PinConfig NANOFRAME_PINOUT = {
    .leds = {
        {PC1, PC2, PC3, false, 0, ROLE_CORE}, // Seg 0: Core LED (direct GPIO)
        {0,   0,   0,   true,  0, ROLE_EXT},  // Seg 1: SR ch0 — eye top-left
        {0,   0,   0,   true,  1, ROLE_EXT},  // Seg 2: SR ch1 — eye bottom-left
        {0,   0,   0,   true,  2, ROLE_EXT},  // Seg 3: SR ch2 — eye top-right
        {0,   0,   0,   true,  3, ROLE_EXT},  // Seg 4: SR ch3 — eye bottom-right
    },
    .coreLEDs = 5,
    .shiftReg    = {PD4, PD3, PD2}, // SER, RCLK, SRCLK
    .shiftRegChannels = 4,
    .colorButton = PC4,
    .animButton = PC5
};

// Configuration for Aurora Glasya
const PinConfig AURORA_GLASYA_PINOUT = {
    .leds = {
        {PC4, PC5, PC6, false, 0, ROLE_CORE}, // Seg 0: Upper LEDs
        {PD4, PD5, PD3, false, 0, ROLE_CORE}, // Seg 1: Lower LEDs
    },
    .coreLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton = PD2,
    .animButton = PC7
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI_PINOUT = {
    .leds = {
        {PC4, PC5, PC6, false, 0, ROLE_CORE}, // Seg 0: Upper LEDs
        {PD4, PD5, PD3, false, 0, ROLE_CORE}, // Seg 1: Lower LEDs
    },
    .coreLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton = PD2,
    .animButton = PC7
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .leds = {
        {PD7, PD6, PD5, false, 0, ROLE_CORE}, // Seg 0: Eyepiece
        {PD2, PD3, PD4, false, 0, ROLE_CORE}, // Seg 1: Main body
    },
    .coreLEDs = 2,
    .shiftReg    = {0, 0, 0},
    .shiftRegChannels = 0,
    .colorButton = PC3,
    .animButton = PC4
};

// Function to get the active configuration based on the selected hardware
inline const PinConfig& getActiveConfig(ConfigType configType) {
    switch(configType) {
        case NANOFRAME:
            return NANOFRAME_PINOUT;
        case AURORA_GLASYA:
            return AURORA_GLASYA_PINOUT;
        case BLINDER_MINI:
            return BLINDER_MINI_PINOUT;
        case AG_ECHO_FRAME:
        default:
            return AG_ECHO_FRAME_PINOUT;
    }
}

#endif // PINOUTS_H
