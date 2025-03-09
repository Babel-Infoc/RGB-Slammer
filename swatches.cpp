#include <stdint.h>
#include "swatches.h"

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

uint8_t swatchIndex = 0;

// Master array of swatches
swatch swatchArray[numSwatches] = {
    {
        rgb(255, 220, 106),
        rgb(209, 0, 52),
        rgb(75, 0, 130),
        rgb(0, 0, 97)
    },
    {
        rgb(174, 255, 130),
        rgb(66, 214, 164),
        rgb(0, 79, 138),
        rgb(25, 34, 37)
    },
    {
        rgb(255, 180, 134),
        rgb(255, 111, 89),
        rgb(190, 46, 221),
        rgb(21, 25, 40)
    },
    {
        rgb(255, 183, 28),
        rgb(255, 72, 0),
        rgb(255, 0, 0),
        rgb(95, 0, 0)
    }
};

// Colors used for the bootup animation
const uint8_t bootswatch[5][3] = {
    rgb(255, 220, 106),
    rgb(209, 0, 52),
    rgb(75,0,130),
    rgb(0, 0, 97),
    rgb(0,0,0)
};
