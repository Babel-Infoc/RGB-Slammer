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
        rgb(255, 196, 0),
        rgb(255, 0, 64),
        rgb(105, 0, 180),
        rgb(0, 0, 138),
        rgb(0,0,0),
    },
    {   // Bright cyan to dark purple
        rgb(0, 255, 255),    // Bright cyan primary
        rgb(0, 180, 220),    // Cyan-blue accent
        rgb(17, 103, 179),   // Mid-tone blue midtone
        rgb(68, 23, 190),   // Blue-purple contrast
        rgb(85, 0, 141),     // Dark purple background
    },
    {   // Yellow to dark red
        rgb(255, 255, 0),    // Vivid yellow primary
        rgb(255, 136, 0),    // Golden yellow accent
        rgb(255, 150, 0),    // Orange midtone
        rgb(220, 60, 0),     // Bright red contrast
        rgb(184, 0, 0),      // Dark red background
    },
    {   // Chemical spill
        rgb(166, 255, 0),    // Acid green primary
        rgb(145, 255, 0),   // Bright lime accent
        rgb(0, 181, 226),    // Mid-tone midtone
        rgb(90, 0, 194),    // Purple contrast
        rgb(65, 0, 131),      // Dark purple background
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
        rgb(162, 62, 230),    // Light purple primary
        rgb(99, 45, 180),     // Medium purple accent
        rgb(71, 18, 158),      // Deep purple midtone
        rgb(86, 86, 129),       // Purple-grey contrast
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
        rgb(255, 51, 0),
        rgb(238, 255, 0),
        rgb(155, 132, 0),
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

// MARK: gradientPosition ------------------------------------------------------------------------------------------------
// Calculates a gradient rgb color between all 5 colors in the current swatch
// position: 255 = background color, 0 = primary color
void gradientPosition(uint8_t position, uint8_t output[3]) {
    // Create an rgb_t object to return
    uint8_t segment = position >> 6;    // Divide by 64 (4 segments of 64 steps each) (inv_position >> 6)
    uint8_t step = position & 0x3F;     // Get remainder (0-63) (inv_position & 0x3F)
    // Clamp segment to valid range (segment > 3)
    if (segment > 3) segment = 3;
    // Calculate pointers to start and end colors (startColor, endColor)
    const uint8_t* startColor = &swatch[swNum].primary[0] + (4 - segment) * 3;
    const uint8_t* endColor = &swatch[swNum].primary[0] + (3 - segment) * 3;

    // Calculate interpolation factor (0-63 to 0-255)
    uint8_t alpha = step << 2;  // Multiply by 4 to scale 0-63 to 0-252

    // Linear interpolation between colors
    output[0] = ((uint16_t)endColor[0] * alpha + (uint16_t)startColor[0] * (255 - alpha)) >> 8;
    output[1] = ((uint16_t)endColor[1] * alpha + (uint16_t)startColor[1] * (255 - alpha)) >> 8;
    output[2] = ((uint16_t)endColor[2] * alpha + (uint16_t)startColor[2] * (255 - alpha)) >> 8;
}
