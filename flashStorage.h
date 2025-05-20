#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <Arduino.h>

// Structure to hold settings that will be stored in flash
// Make sure it's aligned to 4 bytes (32-bit) for proper flash writing
typedef struct {
    uint8_t signature;      // Used to verify if settings are valid
    uint8_t swatchNumber;   // Current swatch number
    uint8_t animationIndex; // Current animation index
    uint8_t checksum;       // Simple checksum for data verification
} __attribute__((aligned(4))) FlashSettings;

// Function prototypes
bool loadSettingsFromFlash(uint8_t* swatchNum, uint8_t* animIndex);
bool saveSettingsToFlash(uint8_t swatchNum, uint8_t animIndex);

#endif // FLASH_STORAGE_H
