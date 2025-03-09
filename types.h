#ifndef TYPES_H
#define TYPES_H

#include <array>

// MARK: ------------------------------ Hardware Definitions ------------------------------
// Struct for LED segment definition
typedef struct {
    int red;             // Red pin
    int green;           // Green pin
    int blue;            // Blue pin
} ledSegment;

// Array of LED segments
extern ledSegment leds[];

// Pin definitions
extern const int colorBtn;
extern const int animBtn;

// MARK: ------------------------------ External Variables ------------------------------
// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}

extern const int numLEDs;  // Removed initialization here to avoid multiple definitions

// LED luminance information
struct luminance {
    int mA;
    int luminance;
};

// Brightness and luminance globals
extern const float maxBrightness;
extern const luminance red;
extern const luminance green;
extern const luminance blue;

// Handover color
extern int handoverColor[2][3];

// MARK: ------------------------------ Button Handling ------------------------------
// Current color swatch
extern int colorIndex;

// Current animation index
extern const int numAnimations;
extern int animIndex;

// MARK: ------------------------------ Swatch Handling ------------------------------
// Swatch struct
struct swatch {
    int highlight[3];
    int primary[3];
    int accent[3];
    int background[3];

    // Default constructor
    swatch() : highlight{0, 0, 0}, primary{0, 0, 0}, accent{0, 0, 0}, background{0, 0, 0} {}

    // Constructor with parameters
    swatch(int h[3], int p[3], int a[3], int b[3]) {
        for (int i = 0; i < 3; i++) {
            highlight[i] = h[i];
            primary[i] = p[i];
            accent[i] = a[i];
            background[i] = b[i];
        }
    }
};

// Current color swatch index
extern int swatchIndex;
extern int swatchArraySize;

// Store the current swatch in a global variable
extern swatch currentSwatch;

// MARK: ------------------------------ Functions ------------------------------
void calculateLuminance();
//void rgbStringToArray(const char* rgbString, int* rgbArray);
void sendToRGB(const int segment, const int* rgbValue);
void checkButtons();
#endif // TYPES_H
