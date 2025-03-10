#include "swatches.h"
#include <Arduino.h>

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

// Initialise the swatch array index
uint8_t swNum = 0;

/*
    highlight
    primary
    ambient
    accent
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
    {   // Yellow to dark red
        rgb(255, 255, 0),    // Vivid yellow highlight
        rgb(255, 200, 0),    // Golden yellow primary
        rgb(255, 150, 0),    // Orange ambient
        rgb(220, 60, 0),     // Bright red accent
        rgb(130, 0, 0),      // Dark red background
    },
    {   // Bright cyan to dark purple
        rgb(0, 255, 255),    // Bright cyan highlight
        rgb(0, 180, 220),    // Cyan-blue primary
        rgb(50, 120, 180),   // Mid-tone blue ambient
        rgb(100, 70, 180),   // Blue-purple accent
        rgb(60, 0, 100),     // Dark purple background
    },
    {   // Chemical spill
        rgb(175, 255, 0),    // Acid green highlight
        rgb(132, 220, 20),   // Bright lime primary
        rgb(88, 158, 95),    // Mid-tone ambient
        rgb(95, 55, 140),    // Purple accent
        rgb(40, 0, 80),      // Dark purple background
    },
    {   // Neon
        rgb(255, 0, 255),    // Bright magenta highlight
        rgb(255, 0, 200),    // Vivid pink primary
        rgb(255, 0, 150),    // Bright pink ambient
        rgb(200, 0, 100),    // Deep pink accent
        rgb(100, 0, 50),     // Dark pink background
    },
    {   // Cyberpunk
        rgb(255, 0, 255),    // Bright magenta highlight
        rgb(0, 255, 255),    // Bright cyan primary
        rgb(0, 0, 255),      // Bright blue ambient
        rgb(0, 0, 128),      // Deep blue accent
        rgb(0, 0, 0),        // Black background
    },
    {   // Cyberdoll
        rgb(180, 120, 220),    // Light purple highlight
        rgb(120, 80, 180),     // Medium purple primary
        rgb(90, 60, 140),      // Deep purple ambient
        rgb(70, 70, 90),       // Purple-grey accent
        rgb(40, 40, 50),       // Dark grey background
    },
    {   // Baby Pink
        rgb(255, 221, 255),    // Light pink highlight
        rgb(255, 200, 200),    // Light red primary
        rgb(255, 150, 150),    // Light red ambient
        rgb(255, 170, 251),    // Light purple accent
        rgb(244, 121, 255),    // Light purple background
    },
    {   // Blue to dark red
        rgb(38, 0, 255),       // Bright blue highlight
        rgb(55, 0, 207),       // Medium blue primary
        rgb(90, 0, 173),       // Blue-purple ambient
        rgb(115, 0, 126),      // Purple-red accent
        rgb(75, 0, 19),        // Dark red background
    },
    {   // Chemical spill
        rgb(225, 255, 0),       // Bright yellow highlight
        rgb(103, 196, 17),      // Bright green primary
        rgb(54, 172, 24),       // Green ambient
        rgb(66, 28, 156),       // Purple accent
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
        rgb(255, 255, 154),
        rgb(255, 230, 0),
        rgb(216, 163, 17),
        rgb(204, 50, 23),
        rgb(100, 0, 0),
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
