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
    {   // Anodized steel
        rgb(255, 220, 106),
        rgb(209, 0, 52),
        rgb(75,0,130),
        rgb(0, 0, 97),
        rgb(0,0,0),
    },
    {   // Oceanic
        rgb(0, 120, 255),
        rgb(0, 0, 255),
        rgb(0, 0, 255),
        rgb(26, 101, 187),
        rgb(50, 26, 146),
    },
    {   // Sunset
        rgb(255, 0, 255),
        rgb(255, 0, 120),
        rgb(245, 69, 0),
        rgb(255, 0, 0),
        rgb(120, 0, 0),
    },
    {   // Chemical spill
        rgb(214, 212, 142),
        rgb(219, 242, 38),
        rgb(158, 193, 49),
        rgb(98, 31, 138),
        rgb(4, 40, 63),
    },
    {   // Chromatic abberation
        rgb(255, 183, 243),
        rgb(255, 255, 255),
        rgb(183, 253, 255),
        rgb(20, 20, 20),
        rgb(0, 0, 0),
    },
    {   // Baby Pink
        rgb(255, 221, 255),
        rgb(255, 200, 200),
        rgb(255, 150, 150),
        rgb(255, 100, 100),
        rgb(255, 50, 50),
    },
    {   // Hazard pay
        rgb(255, 255, 0),
        rgb(255, 120, 0),
        rgb(220, 200, 0),
        rgb(255, 0, 0),
        rgb(100, 0, 0),
    },
    {   // Neon
        rgb(255, 0, 255),
        rgb(0, 255, 255),
        rgb(0, 0, 255),
        rgb(255, 0, 0),
        rgb(0, 0, 0),
    },
    {   // Deep purple and grey
        rgb(174, 0, 255),
        rgb(119, 0, 255),
        rgb(70, 0, 78),
        rgb(119, 60, 124),
        rgb(53, 53, 53),
    },
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
