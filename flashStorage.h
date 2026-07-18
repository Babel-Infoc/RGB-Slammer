#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <Arduino.h>

// Structure to hold settings that will be stored in flash
// Make sure it's aligned to 4 bytes (32-bit) for proper flash writing
typedef struct {
    uint8_t signature;      // Used to verify if settings are valid
    uint8_t swatchNumber;   // Current swatch number
    uint8_t brightness;     // Current brightness (scaled 0-255)
    uint8_t currentAnim;    // Current animation mode
    uint8_t checksum;       // Simple checksum for data verification
} __attribute__((aligned(4))) FlashSettings;

// Function prototypes (brightness is 0-255)
bool loadSettingsFromFlash(uint8_t* swatchNum, uint8_t* brightness, uint8_t* animationMode);
bool saveSettingsToFlash(uint8_t swatchNum, uint8_t brightness, uint8_t animationMode);

void testLoop();
void glitchLoop();
void cautionCitizen();
void slowFade();
void photomode1();
void photomode2();

typedef void (*AnimFunc)();

// Current animation mode
extern AnimFunc animationList[];
extern uint8_t currentAnim;
extern const uint8_t numAnimationModes;

#endif // FLASH_STORAGE_H
