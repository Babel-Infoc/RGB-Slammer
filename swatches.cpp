#include "swatches.h"
#include <Arduino.h>

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

uint8_t swatchIndex = 0;

/*
    highlight
    primary
    ambient
    accent
    background
*/
// Master array of swatches
swatch swatchArray[] = {
    {
        rgb(255, 220, 106),
        rgb(209, 0, 52),
        rgb(75,0,130),
        rgb(0, 0, 97),
        rgb(0,0,0),
    },
    {
        rgb(0, 255, 150),
        rgb(0, 200, 200),
        rgb(126, 204, 0),
        rgb(0, 143, 168),
        rgb(29, 12, 95),
    },
    {
        rgb(255, 255, 0),
        rgb(255, 150, 50),
        rgb(255, 50, 0),
        rgb(255, 0, 0),
        rgb(100, 0, 0),
    },
    {
        rgb(0, 120, 255),
        rgb(0, 0, 255),
        rgb(0, 0, 255),
        rgb(26, 101, 187),
        rgb(50, 26, 146),
    },
    {
        rgb(255, 0, 255),
        rgb(255, 0, 120),
        rgb(173, 33, 115),
        rgb(255, 0, 0),
        rgb(120, 0, 0),
    },
    {
        rgb(214, 212, 142),
        rgb(219, 242, 38),
        rgb(158, 193, 49),
        rgb(0, 91, 82),
        rgb(4, 40, 63),
    },
    {
        rgb(255, 255, 255),
        rgb(133, 255, 245),
        rgb(0, 0, 0),
        rgb(126, 163, 157),
        rgb(0, 0, 0),
    }
};

// Calculate the number of swatches automatically based on array size
uint8_t numSwatches = sizeof(swatchArray) / sizeof(swatchArray[0]);

// Colors used for the bootup animation
const uint8_t bootswatch[5][3] = {
    rgb(255, 220, 106),
    rgb(209, 0, 52),
    rgb(75,0,130),
    rgb(0, 0, 97),
    rgb(0,0,0)
};
