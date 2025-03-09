#ifndef SWATCHES_H
#define SWATCHES_H
#include <array>
#include "types.h"

// Array of swatches
constexpr int swatchSize = 4; // Number of colors in each swatch
constexpr int numSwatches = 6; // Number of swatches in the collection

// Using std::array for a fixed-size container with better type safety
const std::array<swatch, numSwatches> swatchArray = {{
    {
        "rgb(255, 220, 106)",
        "rgb(209, 0, 52)",
        "rgb(75,0,130)",
        "rgb(0, 0, 97)"
    },
    {
        "rgb(174, 255, 130)",
        "rgb(66, 214, 164)",
        "rgb(0, 79, 138)",
        "rgb(25, 34, 37)"
    },
    {
        "rgb(255, 180, 134)",
        "rgb(255, 111, 89)",
        "rgb(190, 46, 221)",
        "rgb(21, 25, 40)"
    },
    {
        "rgb(174, 255, 130)",
        "rgb(66, 214, 164)",
        "rgb(0, 79, 138)",
        "rgb(25, 34, 37)"
    },
    {
        "rgb(255, 183, 28)",
        "rgb(255, 72, 0)",
        "rgb(255, 0, 0)",
        "rgb(95, 0, 0)"
    },
    {
        "rgb(168, 255, 209)",
        "rgb(40, 178, 164)",
        "rgb(74, 69, 143)",
        "rgb(18, 31, 48)"
    }
}};

// Initialize the currentSwatch
extern swatch currentSwatch;

// Using std::array for bootwatch colors as well
const std::array<const char*, 5> bootswatch = {{
    "rgb(0,0,0)",
    "rgb(255, 220, 106)",
    "rgb(209, 0, 52)",
    "rgb(75,0,130)",
    "rgb(0, 0, 97)"
}};

#endif // SWATCHES_H
