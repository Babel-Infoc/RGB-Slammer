#include "swatches.h"
#include <Arduino.h>

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

// Initialise the swatch array index
uint8_t swNum = 0;

/*
    primary
    accent
    midtone
    contrast
    background
*/
// Master array of swatches
swatchArray swatch[] = {
    {   // Anodized steel
        rgb(255, 220, 106),
        rgb(209, 0, 52),
        rgb(75,0,130),
        rgb(0, 0, 97),
        rgb(0,0,0),
    },
    {   // Bright cyan to dark purple
        rgb(0, 255, 255),    // Bright cyan primary
        rgb(0, 180, 220),    // Cyan-blue accent
        rgb(50, 120, 180),   // Mid-tone blue midtone
        rgb(100, 70, 180),   // Blue-purple contrast
        rgb(60, 0, 100),     // Dark purple background
    },
    {   // Yellow to dark red
        rgb(255, 255, 0),    // Vivid yellow primary
        rgb(255, 136, 0),    // Golden yellow accent
        rgb(255, 150, 0),    // Orange midtone
        rgb(220, 60, 0),     // Bright red contrast
        rgb(130, 0, 0),      // Dark red background
    },
    {   // Chemical spill
        rgb(159, 255, 167),    // Acid green primary
        rgb(145, 255, 0),   // Bright lime accent
        rgb(117, 0, 226),    // Mid-tone midtone
        rgb(93, 17, 180),    // Purple contrast
        rgb(40, 0, 80),      // Dark purple background
    },
    {   // Neon
        rgb(255, 0, 255),    // Bright magenta primary
        rgb(255, 0, 200),    // Vivid pink accent
        rgb(255, 0, 150),    // Bright pink midtone
        rgb(200, 0, 100),    // Deep pink contrast
        rgb(100, 0, 50),     // Dark pink background
    },
    {   // Cyberpunk
        rgb(255, 0, 255),    // Bright magenta primary
        rgb(0, 255, 255),    // Bright cyan accent
        rgb(0, 0, 255),      // Bright blue midtone
        rgb(0, 0, 128),      // Deep blue contrast
        rgb(0, 0, 0),        // Black background
    },
    {   // Cyberdoll
        rgb(180, 120, 220),    // Light purple primary
        rgb(120, 80, 180),     // Medium purple accent
        rgb(90, 60, 140),      // Deep purple midtone
        rgb(70, 70, 90),       // Purple-grey contrast
        rgb(40, 40, 50),       // Dark grey background
    },
    {   // Baby Pink
        rgb(255, 221, 255),    // Light pink primary
        rgb(204, 153, 204),    // Light red accent
        rgb(180, 97, 180),     // Light red midtone
        rgb(185, 41, 185),     // Light purple contrast
        rgb(122, 10, 122),     // Light purple background
    },
    {   // Blue to dark red
        rgb(38, 0, 255),       // Bright blue primary
        rgb(55, 0, 207),       // Medium blue accent
        rgb(138, 0, 173),       // Blue-purple midtone
        rgb(126, 0, 73),      // Purple-red contrast
        rgb(102, 0, 26),        // Dark red background
    },
    {   // Chemical spill
        rgb(225, 255, 0),       // Bright yellow primary
        rgb(103, 196, 17),      // Bright green accent
        rgb(54, 172, 24),       // Green midtone
        rgb(66, 28, 156),       // Purple contrast
        rgb(79, 27, 109),       // Dark purple background
    },
    {   // Chromatic abberation
        rgb(255, 183, 243),
        rgb(255, 255, 255),
        rgb(183, 253, 255),
        rgb(20, 20, 20),
        rgb(20, 20, 20),
    },
    {   // Hazard pay
        rgb(255, 255, 0),
        rgb(255, 136, 0),
        rgb(255, 38, 0),
        rgb(155, 23, 0),
        rgb(133, 0, 0),
    }
};

// Calculate the number of swatches automatically based on array size
uint8_t numSwatches = sizeof(swatch) / sizeof(swatch[0]);

// Colors used for the bootup animation
const uint8_t bootswatch[5][3] = {
    rgb(255, 220, 106),
    rgb(209, 0, 52),
    rgb(75,0,130),
    rgb(0, 0, 97),
    rgb(0,0,0)
};
