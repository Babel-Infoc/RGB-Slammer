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
        rgb(255,    196,    0),
        rgb(255,    0,      64),
        rgb(105,    0,      180),
        rgb(0,      0,      138),
        rgb(0,      0,      0),
    },
    {   // Wintermute
        rgb(0,      255,    255),
        rgb(0,      180,    220),
        rgb(17,     103,    179),
        rgb(68,     23,     190),
        rgb(85,     0,      141),
    },
    {   // Hazard pay
        rgb(255,    255,    0),
        rgb(255,    136,    0),
        rgb(255,    150,    0),
        rgb(220,    60,     0),
        rgb(184,    0,      0),
    },
    {   // Chemical spill
        rgb(166,    255,    0),
        rgb(145,    255,    0),
        rgb(0,      181,    226),
        rgb(90,     0,      194),
        rgb(65,     0,      131),
    },
    {   // Brandi
        rgb(255,    0,      255),
        rgb(255,    0,      200),
        rgb(255,    0,      150),
        rgb(200,    0,      100),
        rgb(100,    0,      50),
    },
    {   // Classic
        rgb(255,    0,      255),
        rgb(0,      255,    255),
        rgb(0,      0,      255),
        rgb(0,      0,      128),
        rgb(0,      0,      0),
    },
    {   // Straylight
        rgb(98,     0,      255),
        rgb(96,     26,     201),
        rgb(57,     18,     167),
        rgb(57,     57,     151),
        rgb(40,     40,     50),
    },
    {   // Babygirl
        rgb(255,    221,    255),
        rgb(204,    153,    204),
        rgb(180,    97,     180),
        rgb(185,    41,     185),
        rgb(122,    10,     122),
    },
    {   // Redshift
        rgb(38,     0,      255),
        rgb(55,     0,      207),
        rgb(138,    0,      173),
        rgb(126,    0,      73),
        rgb(102,    0,      26),
    },
    {   // Chromatic abberation
        rgb(255,    183,    243),
        rgb(255,    255,    255),
        rgb(183,    253,    255),
        rgb(20,     20,     20),
        rgb(20,     20,     20),
    },
    {   // Arasaka
        rgb(255,    0,      0),
        rgb(216,    0,      0),
        rgb(156,    0,      0),
        rgb(133,    0,      0),
        rgb(0,      0,      0),
    },
};

// Calculate the number of swatches automatically based on array size
uint8_t numSwatches = sizeof(swatch) / sizeof(swatch[0]);

// Colors used for the bootup animation
const uint8_t bootswatch[5][3] = {
    rgb(255, 220, 106),
    rgb(209, 0, 52),
    rgb(75, 0, 130),
    rgb(0, 0, 97),
    rgb(0, 0, 0)
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
