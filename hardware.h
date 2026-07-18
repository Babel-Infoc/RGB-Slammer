#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations

// Available configurations as enum for easier selection
enum ConfigType {
    NANOFRAME,
    AURORA_GLASYA,
    BLINDER_MINI,
    NANOSHARD,
    AG_ECHO_FRAME
};

struct PinConfig {
    uint8_t zones;
    ledZone leds[2];
    uint8_t colorButton;
    uint8_t animButton;
    // Brightness tuning (0-255)
    uint8_t minBrightness;     // Lower bound for brightness adjustment mode
    uint8_t maxBrightness;     // Upper bound for brightness adjustment mode
    // LED color tuning: {mA, luminosity} per channel, from the LED datasheet
    luminance red;
    luminance green;
    luminance blue;
    // Slow down all animations by this amount (in milliseconds)
    uint8_t slowDown;
};

// Configuration for Aurora Glasya
const PinConfig NANOFRAME_PINOUT = {
    .zones = 5,
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD6,
    .animButton = PD7,
    .minBrightness = 77,
    .maxBrightness = 153,
    .red   = {5, 45},
    .green = {5, 45},
    .blue  = {5, 55},
    .slowDown = 1
};

// Configuration for Aurora Glasya
const PinConfig AURORA_GLASYA_PINOUT = {
    .zones = 2,
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PC7,
    .minBrightness = 77,
    .maxBrightness = 153,
    .red   = {5, 45},
    .green = {5, 45},
    .blue  = {5, 55},
    .slowDown = 1
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI_PINOUT = {
    .zones = 2,
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PC7,
    .minBrightness = 77,
    .maxBrightness = 153,
    .red   = {5, 45},
    .green = {5, 45},
    .blue  = {5, 55},
    .slowDown = 1
};

// Configuration for Nano shard
const PinConfig NANOSHARD_PINOUT = {
    .zones = 2,
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD3, PD4, PD5}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PD6,
    .minBrightness = 50,
    .maxBrightness = 100,
    .red   = {5, 90},
    .green = {5, 100},
    .blue  = {5, 100},
    .slowDown = 1
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .zones = 2,
    .leds = {
        {PD7, PD6, PD5}, // Eyepiece
        {PD2, PD3, PD4}  // Main body
    },
    .colorButton = PC3,
    .animButton = PC0,
    .minBrightness = 20,
    .maxBrightness = 80,
    .red   = {5, 45},
    .green = {5, 45},
    .blue  = {5, 55},
    .slowDown = 1
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
            return AG_ECHO_FRAME_PINOUT;
        case NANOSHARD:
            return NANOSHARD_PINOUT;
        default:
            return AG_ECHO_FRAME_PINOUT;
    }
}

#endif // HARDWARE_H
