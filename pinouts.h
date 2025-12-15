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
    motor motorpins[4];
    button buttonPins[2];
    shiftReg shiftRegPins[3]
};

// Configuration for Nanoframe
const PinConfig NANOFRAME_PINOUT = {
    .leds = {
        {0, 0, 0},       // Channel 1 (not in use)
        {0, 0, 0},       // Channel 2 (not in use)
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .motorpins ={
        {PC7, PC6, PC4, PC4},    // Motor pins
    },
    buttonPins = {
        {PD6},    // Color button
        {PD7}     // Animation button
    },
    shiftRegPins = {
        {
            PD2,    // Serial
            PD3,    // RCLK
            PD4     // SRCLK
        },
    },
};

// Configuration for Aurora Glasya
const PinConfig AURORA_GLASYA_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Channel 1
        {PD4, PD5, PD3}, // Channel 2
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PD2,  // Color button
    .animButton = 0,     // Animation button (not in use)
    .motor1 = 0,         // Motor pin 1 (not in use)
    .motor2 = 0          // Motor pin 2 (not in use)
    .motor3 = 0,         // Motor pin 3 (not in use)
    .motor4 = 0,         // Motor pin 4 (not in use)
    .serPin = 0          // Data pin (not in use)
    .rclkPin = 0,        // Clock pin (not in use)
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Channel 1
        {PD4, PD5, PD3}, // Channel 2
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PD2,  // Color button
    .animButton = PC7,   // Animation button
    .motor1 = 0,         // Motor pin 1 (not in use)
    .motor2 = 0          // Motor pin 2 (not in use)
    .motor3 = 0,         // Motor pin 3 (not in use)
    .motor4 = 0,         // Motor pin 4 (not in use)
    .serPin = 0          // Data pin (not in use)
    .rclkPin = 0,        // Clock pin (not in use)
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .leds = {
        {PD7, PD6, PD5}, // Channel 1 - Eyepiece
        {PD2, PD3, PD4}, // Channel 2 - Main body
        {0, 0, 0}        // Channel 3 (not in use)
    },
    .colorButton = PC3,  // Color button
    .animButton = PC4,   // Animation button
    .motor1 = 0,         // Motor pin 1 (not in use)
    .motor2 = 0          // Motor pin 2 (not in use)
    .motor3 = 0,         // Motor pin 3 (not in use)
    .motor4 = 0,         // Motor pin 4 (not in use)
    .serPin = 0          // Data pin (not in use)
    .rclkPin = 0,        // Clock pin (not in use)
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
