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
    ledSegment leds[3];
    uint8_t colorButton;
    uint8_t animButton;
    uint8_t motorPinA;  // Motor pin A (positive when forward)
    uint8_t motorPinB;  // Motor pin B (positive when reverse)
};

// Configuration for Nanoframe
const PinConfig NANOFRAME_PINOUT = {
    .leds = {
        {PC6, PC7, PC5}, // Channel 1
        {PD4, PD5, PD3}, // Channel 2
        {PC1, PC2, PC0}  // Channel 3
    },
    .colorButton = PD6,
    .animButton = PD7,
    .motorPinA = PC4,    // Motor pin A
    .motorPinB = PC3     // Motor pin B
};

// Configuration for Aurora Glasya
const PinConfig AURORA_GLASYA_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Channel 1
        {PD4, PD5, PD3}, // Channel 2
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PD2,
    .animButton = PC7,
    .motorPinA = PC1,    // Motor pin A
    .motorPinB = PC2     // Motor pin B
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Channel 1
        {PD4, PD5, PD3}, // Channel 2
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PD2,
    .animButton = PC7,
    .motorPinA = PC1,    // Motor pin A
    .motorPinB = PC2     // Motor pin B
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .leds = {
        {PD7, PD6, PD5}, // Channel 1 - Eyepiece
        {PD2, PD3, PD4}, // Channel 2 - Main body
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PC3,
    .animButton = PC4,
    .motorPinA = PC5,    // Motor pin A
    .motorPinB = PC6     // Motor pin B
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
