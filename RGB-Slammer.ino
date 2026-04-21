// RGB Slammer
// Written by Tully Jagoe 2026

// This script is best edited in VSCode for color token selection in swatches.cpp

#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "waveforms.h"
#include "flashStorage.h"
// Select from the active hardware configuration options in hardware.h
// That file stores all GPIO pin assignments, brightness tuning, and LED calibration values.
#define ACTIVE_CONFIG CONFIG_BREACH_KEY
#include "hardware.h"

// Define the LED array and button pins according to the active configuration
ledSegment led[MAX_LED_SEGMENTS];
uint8_t colorBtn;
uint8_t animBtn;

// Shift register pin configuration
shiftRegPins shiftReg;
// Initialise LED channels (will be updated during boot based on active config)
uint8_t gpioLEDs = 0;
uint8_t srLEDs = 0;

// Brightness 0-255 (initialised from active hardware config at boot; overridden by flash if saved)
uint8_t currentBrightness;
// Temporary brightness value used when previewing a new swatch
uint8_t pulseBrightness;

// Per-segment output scale (0-255, post-gamma). Initialised from active hardware config at boot.
uint8_t segOutputScale[MAX_LED_SEGMENTS];

// Brightness adjustment mode limits
uint8_t minBrightness;
uint8_t maxBrightness;
const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// PWM loop delay in milliseconds
uint8_t slowDown;

// LED luminance calibration
luminance red;
luminance green;
luminance blue;

extern void (*srUpdateCallback)(); // defined in rgbProcessor.cpp

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig();

    // Copy pin configuration from the selected hardware profile
    for (uint8_t i = 0; i < config.gpioLEDs && i < MAX_LED_SEGMENTS; i++) {
        led[i] = config.leds[i];
    }
    gpioLEDs  = config.gpioLEDs;
    colorBtn = config.colorButton;
    animBtn  = config.animButton;

    // Copy shift register configuration
    shiftReg = config.shiftReg;
    srLEDs  = config.shiftRegChannels;

    // Copy hardware-specific brightness and LED tuning from active config
    currentBrightness = config.defaultBrightness;
    pulseBrightness   = config.pulseBrightness;
    for (uint8_t i = 0; i < MAX_LED_SEGMENTS; i++) segOutputScale[i] = config.segOutputScale[i];
    minBrightness = config.minBrightness;
    maxBrightness = config.maxBrightness;
    slowDown      = config.slowDown;
    red           = config.redLed;
    green         = config.greenLed;
    blue          = config.blueLed;

    // Set up all LED segments (GPIO only — SR segments have no pins to configure)
    for (uint8_t segment = 0; segment < gpioLEDs; segment++) {
        if (!led[segment].isSR) {
            pinMode(led[segment].red, OUTPUT);
            pinMode(led[segment].green, OUTPUT);
            pinMode(led[segment].blue, OUTPUT);
        }
    }

    // Set up shift register pins if configured
    if (srLEDs > 0) {
        pinMode(shiftReg.ser,   OUTPUT);
        pinMode(shiftReg.rclk,  OUTPUT);
        pinMode(shiftReg.srclk, OUTPUT);
        // Clear all shift register outputs on startup — 0xFF = all outputs HIGH = all LEDs off (active-LOW wiring)
        digitalWrite(shiftReg.rclk, LOW);
        shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, 0xFF);
        shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, 0xFF);
        digitalWrite(shiftReg.rclk, HIGH);
    }
    // Set up the buttons
    pinMode(colorBtn, INPUT_PULLUP);
    if (animBtn != PIN_NONE) pinMode(animBtn, INPUT_PULLUP);

    // Automatic white balance
    calculateLuminance();

    // Try to load saved settings from flash; currentBrightness falls back to hardware default if none saved
    if (!loadSettingsFromFlash(&swNum, &currentBrightness, &animationMode)) {
        swNum             = 0;
        currentBrightness = config.defaultBrightness;
        animationMode     = 0;
    }

    // Show the bootup animation
    bounceBoot(40);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/
// Dispatches animation modes; glitchLoop drives Core and SR channels in the same frame.

void loop() {
    // Check if brightness adjustment mode should be active
    if (brightnessAdjustMode) {
        brightnessAdjustmentMode();
    }
    // Check if swatch preview animation should play
    else if (swatchPreviewActive) {
        swatchPreview();
    }
    // Check if animation preview should play
    else if (animationPreviewActive) {
        animationPreview();
    } else {
        // Run the selected animation mode
        switch (animationMode) {
            case 0:
            default:
                glitchLoop(10, 10);
                //cautionCitizen();
                break;
            case 1:
                cautionCitizen();
                break;
            case 2:
                glitchLoop(20, 10);
                break;
            case 3:
                glitchLoop(20, 10);
                break;
            case 4:
                glitchLoop(20, 10);
                break;
        }
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// -------------------------------------------------------------------------------------
// MARK: bounceBoot
void bounceBoot(int speed){
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = speed * 3;
        fadeToColor(swatch[swNum].primary,      255,    swatch[swNum].background,   100,    speed);
        fadeToColor(swatch[swNum].accent,       220,    swatch[swNum].primary,      255,    speed);
        fadeToColor(swatch[swNum].midtone,      180,    swatch[swNum].accent,       220,    speed);
        fadeToColor(swatch[swNum].contrast,     150,    swatch[swNum].midtone,      180,    speed);
        fadeToColor(swatch[swNum].background,   100,    swatch[swNum].contrast,     150,    speed);
        fadeToColor(swatch[swNum].background,   100,    swatch[swNum].background,   100,    speed);
    }
    showColor(swatch[0].background, 0, swatch[0].background, 0, speed*5);
}

// -------------------------------------------------------------------------------------
// MARK: glitchLoop
// Per-frame dual-channel loop: runs until interrupted by a swatch or animation preview.
// SR channel:   probability roll → eyeScatter or eyeDoublePulse, writes shiftRegColors[].
// Core channel: probability roll → blocking glitch effect or default flicker (flush point).
void glitchLoop(const uint8_t coreEffectChance, const uint8_t srEffectChance) {
    const uint8_t flickerChance = 70;     // Chance (0-100) that a given frame will have a Core glitch effect instead of default flicker
    while (true) {
        // Exit immediately on any preview interrupt
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) {
            return;
        }

        // SR channel: register the SR function for this iteration.
        // sendToRGB will call it automatically before each Core flush (including inside glitch effects).
        srUpdateCallback = (random(0, 100) < srEffectChance) ? eyeScatter : eyeDoublePulse;

        // Core channel: probability roll selects a blocking glitch effect or the default flicker.
        if (random(0, 100) < coreEffectChance) {
            uint8_t flickerSegment = 0;
            for (uint8_t s = 0; s < gpioLEDs; s++) { if (led[s].role == ROLE_GPIO) { flickerSegment = s; break; } }
            switch (random(0, 6)) {
                case 0:
                    coreGlitch1(flickerSegment, 700);
                    break;
                case 1:
                    coreGlitch2(swatch[swNum].midtone ,swatch[swNum].contrast, 700);
                    break;
                case 2:
                    coreGlitch3(flickerSegment, swatch[swNum].primary, 20, 3);
                    break;
                case 3:
                    coreGlitch4(6, 700);
                    break;
                case 4:
                    coreGlitch5();
                    break;
                case 5:
                    fakeMorse(65, 210, 400);
                    break;
            }
        } else {
            // Default: flicker for a duration before the next probability roll.
            // Without this loop, each flicker() call takes ~1 ms and the glitch dice
            // would re-roll thousands of times per second, making effects fire constantly.
            unsigned long flickerEnd = millis() + 200;
            while (millis() < flickerEnd) {
                if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;
                // Non-SR ROLE_SR segments flicker independently.
                for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
                    if (led[seg].role == ROLE_SR && !led[seg].isSR) {
                        flicker(seg, flickerChance, 180, 200, 5);
                    }
                }
                for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
                    if (led[seg].role == ROLE_GPIO) {
                        // Seg 0 → primary (0), Seg 1 → background (4)
                        flicker(seg, flickerChance, 180, 200, (seg == 1 ? 1 : 4));
                    }
                }
            }
        }
    }
}

// No-arg wrapper: ripple between contrast and background — used as srUpdateCallback idle in cautionCitizen.
static void cautionRipple() { eyeRipple(0, 50, 100); }

// -------------------------------------------------------------------------------------
// MARK: cautionCitizen
// Idle: eyeRipple on SR eye pods (via callback), flicker on core LED.
// Every 1.5 s: burst of 3 rapid strobe flashes — SR flashes primary/contrast,
// core flashes accent/contrast — then returns to idle.
void cautionCitizen() {
    const unsigned long STROBE_INTERVAL_MS = 1500UL;
    const uint8_t       STROBE_REPS        = 3;
    const uint8_t       FLASH_ON_MS        = 20;
    const uint8_t       FLASH_OFF_MS       = 20;

    static unsigned long lastStrobeTime = 0;
    if (lastStrobeTime == 0) lastStrobeTime = millis();

    uint8_t coreSeg = 0;
    for (uint8_t s = 0; s < gpioLEDs; s++) { if (led[s].role == ROLE_GPIO) { coreSeg = s; break; } }

    srUpdateCallback = cautionRipple;

    while (true) {
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        if (millis() - lastStrobeTime >= STROBE_INTERVAL_MS) {
            // Strobe burst: null callback so showColor drives SR directly
            srUpdateCallback = nullptr;
            for (uint8_t i = 0; i < STROBE_REPS; i++) {
                if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) break;
                showColor(swatch[swNum].accent,   255,    swatch[swNum].primary,   255,    FLASH_ON_MS);
                showColor(swatch[swNum].contrast, 255,    swatch[swNum].contrast,  255,    FLASH_OFF_MS);
            }
            lastStrobeTime = millis();
            srUpdateCallback = cautionRipple;
        } else {
            // Idle: flicker drives the core flush; cautionRipple callback animates SR
            flicker(coreSeg, 70, 100, 150, 5);
        }
    }
}


// -------------------------------------------------------------------------------------
// MARK: fadeToColor
// color1 → ROLE_GPIO segments, color2 → ROLE_SR segments.
// If srUpdateCallback is set, it runs during the ROLE_GPIO flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive ROLE_SR directly with color2.
void fadeToColor(const uint8_t color1[3], uint8_t brightness1, const uint8_t color2[3], uint8_t brightness2, const int fadeTime){
    uint8_t startColor[MAX_LED_SEGMENTS][3];
    uint8_t output[MAX_LED_SEGMENTS][3];

    // Copy handoverColor to startColor for all segments
    for (uint8_t segment = 0; segment < gpioLEDs; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            startColor[segment][pin] = handoverColor[segment][pin];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        uint8_t fadeRatio = (uint8_t)(((unsigned long)(millis() - startTime) * 255UL) / (unsigned long)fadeTime);
        for (int pin = 0; pin < 3; pin++) {
            for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
                bool isCore = (led[seg].role == ROLE_GPIO);
                const uint8_t* target = isCore ? color1 : color2;
                uint8_t bri = isCore ? brightness1 : brightness2;
                int16_t delta = (int16_t)target[pin] - (int16_t)startColor[seg][pin];
                uint8_t blended = (uint8_t)((int16_t)startColor[seg][pin] + (int16_t)(((int32_t)delta * fadeRatio) >> 8));
                output[seg][pin] = (uint8_t)(((uint16_t)blended * bri) >> 8);
            }
        }
        // SR-first: buffer ROLE_SR color2, then ROLE_GPIO flush sends both.
        // If srUpdateCallback is set it fires during the ROLE_GPIO flush and overrides the buffer.
        for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
            if (led[seg].role == ROLE_SR) sendToRGB(seg, output[seg]);
        }
        for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
            if (led[seg].role == ROLE_GPIO) sendToRGB(seg, output[seg]);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
// color1 → ROLE_GPIO segments, color2 → ROLE_SR segments.
// If srUpdateCallback is set, it runs during the ROLE_GPIO flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive ROLE_SR directly with color2.
void showColor(uint8_t color1[3], uint8_t brightness1, uint8_t color2[3], uint8_t brightness2, int duration){
    uint8_t out1[3], out2[3];
    for (uint8_t c = 0; c < 3; c++) {
        out1[c] = (uint8_t)(((uint16_t)color1[c] * brightness1) >> 8);
        out2[c] = (uint8_t)(((uint16_t)color2[c] * brightness2) >> 8);
    }
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }
        // SR-first: buffer ROLE_SR color2, then ROLE_GPIO flush sends both.
        // If srUpdateCallback is set it fires during the ROLE_GPIO flush and overrides the buffer.
        for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
            if (led[seg].role == ROLE_SR) sendToRGB(seg, out2);
        }
        for (uint8_t seg = 0; seg < gpioLEDs; seg++) {
            if (led[seg].role == ROLE_GPIO) sendToRGB(seg, out1);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
// Flickers a segment by scaling a swatch color by a random brightness in [min, max].
// swatchIndex selects which swatch color to use:
//   0=primary  1=accent  2=midtone  3=contrast  4=background  5=random
// If pin is an SR segment, suppresses srUpdateCallback and triggers a ROLE_GPIO flush
// so the buffered SR color is output immediately.
void flicker(const uint8_t pin, const uint8_t chance, const uint8_t min, const uint8_t max, const uint8_t swatchIndex){
    uint8_t outputColor[3];
    uint8_t brightness = random(min, max);
    const uint8_t* colorChoices[5] = {
        swatch[swNum].primary,
        swatch[swNum].accent,
        swatch[swNum].midtone,
        swatch[swNum].contrast,
        swatch[swNum].background
    };
    uint8_t idx = (swatchIndex >= 5) ? (uint8_t)random(0, 5) : swatchIndex;
    const uint8_t* baseColor = colorChoices[idx];
    for (uint8_t c = 0; c < 3; c++)
        outputColor[c] = (uint8_t)(((uint16_t)baseColor[c] * brightness) >> 8);
    sendToRGB(pin, outputColor);
    // SR segments buffer only — need a GPIO flush to output. Suppress callback so
    // hand-written SR color is not overwritten, then flush via the ROLE_GPIO segment.
    if (pin < gpioLEDs && led[pin].isSR) {
        void (*saved)() = srUpdateCallback;
        srUpdateCallback = nullptr;
        for (uint8_t s = 0; s < gpioLEDs; s++) {
            if (!led[s].isSR) { sendToRGB(s, handoverColor[s]); break; }
        }
        srUpdateCallback = saved;
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch1
void coreGlitch1(const uint8_t segment, int duration){
    uint8_t otherSegment = 0;
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        // Animate the active segment; the opposite role always holds contrast
        if (led[segment].role == ROLE_GPIO) {
            showColor(swatch[swNum].accent,      255,    swatch[swNum].contrast, 100,    50);
            showColor(swatch[swNum].background,   70,    swatch[swNum].contrast, 100,    50);
        } else {
            showColor(swatch[swNum].contrast, 100,    swatch[swNum].accent,      255,    50);
            showColor(swatch[swNum].contrast, 100,    swatch[swNum].background,   70,    50);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch2
void coreGlitch2(uint8_t color1[3], uint8_t color2[3], int duration) {
    // Part 1: Rapidly flash between black and background for 1 second
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }
        rapidPulse(color1, color2, 255, 50);
    }

    // Part 2: Rapidly fade core through all swatch colors; eyes hold contrast throughout
    uint8_t fadeTime = 50;
    fadeToColor(swatch[swNum].background,   50,     swatch[swNum].contrast,     100,    fadeTime);
    fadeToColor(swatch[swNum].contrast,     100,    swatch[swNum].contrast,     100,    fadeTime);
    fadeToColor(swatch[swNum].midtone,      150,    swatch[swNum].contrast,     100,    fadeTime);
    fadeToColor(swatch[swNum].accent,       200,    swatch[swNum].contrast,     100,    fadeTime);
    fadeToColor(swatch[swNum].primary,      255,    swatch[swNum].contrast,     100,    fadeTime);
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch3
void coreGlitch3(uint8_t segment, uint8_t color2[3], int duration,  uint8_t reps) {
    uint8_t startColor[3] = {handoverColor[segment][0], handoverColor[segment][1], handoverColor[segment][2]};
    // Find a representative segment of the opposite role for handover reference
    uint8_t otherSegment = 0;
    if (led[segment].role == ROLE_GPIO) {
        for (uint8_t s = 0; s < gpioLEDs; s++) { if (led[s].role == ROLE_SR)  { otherSegment = s; break; } }
    } else {
        for (uint8_t s = 0; s < gpioLEDs; s++) { if (led[s].role == ROLE_GPIO) { otherSegment = s; break; } }
    }
    // Hold otherSegment at its handoverColor, and flash segment between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        if (led[segment].role == ROLE_GPIO) {
            showColor(startColor, 255, handoverColor[otherSegment], 255, duration);
            showColor(color2, 255, handoverColor[otherSegment], 255, duration);
        } else {
            showColor(handoverColor[otherSegment], 255, startColor, 255, duration);
            showColor(handoverColor[otherSegment], 255, color2, 255, duration);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch4
void coreGlitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();
    while (millis() - start < duration) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        for (uint8_t segment = 0; segment < gpioLEDs; segment++) {
            if (led[segment].role == ROLE_SR) continue;
            gradientPosition(random(1, 255), color);
            for (uint8_t i = 0; i < reps; i++) {
                sendToRGB(segment, color);
                sendToRGB(segment, swatch[swNum].contrast);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch5
void coreGlitch5(){
    // Use one of the waveform arrays from waveforms.cpp
    uint8_t waveformIndex = random(0, 2); // Choose between the two available waveforms
    uint8_t outputColor[3];

    // First play through the waveform once
    for (uint8_t i = 0; i < 32; i++) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        // Get color at this position in the gradient
        gradientPosition(waveform[waveformIndex].waveform[i], outputColor);

        // Show this color on the core LED; eyes hold contrast
        showColor(outputColor, 255, swatch[swNum].contrast, 100, 50);

        // Brief black flash on core every few steps for a glitchy effect
        if (i % 4 == 0) {
            uint8_t blackColor[3] = {0, 0, 0};
            showColor(blackColor, 255, swatch[swNum].contrast, 100, 10);
        }
    }

    // Then do some rapid random jumps between waveform positions
    for (uint8_t i = 0; i < 8; i++) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        uint8_t randomPos = random(0, 32);
        gradientPosition(waveform[waveformIndex].waveform[randomPos], outputColor);
        showColor(outputColor, 255, swatch[swNum].contrast, 100, 30);

        // Brief flashes to black between jumps
        uint8_t blackColor[3] = {0, 0, 0};
        showColor(blackColor, 255, swatch[swNum].contrast, 100, 15);
    }

    // End with a final dramatic fade to black on core; eyes hold contrast
    fadeToColor(swatch[swNum].background,   100,   swatch[swNum].midtone,   200,     300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t color1[3], uint8_t color2[3], uint8_t brightness, int speed){
    showColor(color1, brightness, color2, brightness, speed); // core = color1, eye = color2
    showColor(color2, brightness, color2, brightness, speed); // core = color2, eye = color2
}

// -------------------------------------------------------------------------------------
// MARK: fakeMorse
void fakeMorse(uint8_t color1, uint8_t color2, int duration) {
    uint8_t output1[3];
    uint8_t output2[3];
    gradientPosition(color1, output1);
    gradientPosition(color2, output2);
    int interval = 50; // Change every 100ms

    unsigned long start = millis();
    while (millis() - start < duration) {
        // Check if swatch preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || brightnessAdjustMode) {
            return;
        }

        int selection = random(0, 3);
        // Core flickers between gradient positions; eyes hold contrast throughout
        if (selection == 0) {
            showColor(output1, 60, swatch[swNum].contrast, 150, interval);
        } else if (selection == 1) {
            showColor(output2, 60, swatch[swNum].contrast, 150, interval);
        } else {
            showColor(output1, 60, swatch[swNum].contrast, 150, interval);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: swatchPreview
void swatchPreview() {
    const uint8_t fadeUpDuration = 50; // 0.2 seconds fade up
    const uint16_t fadeDownDuration = 400; // 0.6 seconds fade down
    const uint8_t fadeUpSteps = 20; // Steps for fade up
    const uint8_t fadeDownSteps = 60; // Steps for fade down

    // Store original brightness and temporarily increase it
    uint8_t originalBrightness = currentBrightness;
    currentBrightness = pulseBrightness;

    // Phase 1: Fade UP quickly from dark to bright over 0.2 seconds
    for (uint8_t i = 0; i < fadeUpSteps; i++) {
        uint8_t gradientPos = (i * 255) / (fadeUpSteps - 1);
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t seg = 1; seg < gpioLEDs; seg++) { sendToRGB(seg, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (uint8_t i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t seg = 1; seg < gpioLEDs; seg++) { sendToRGB(seg, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeDownDuration / fadeDownSteps);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Save deferred from button handler — flash erase here is safe outside the PWM loop
    saveSettingsToFlash(swNum, currentBrightness, animationMode);

    // Reset the flag
    swatchPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: animationPreview
void animationPreview() {
    const uint8_t flashDuration = 20; // Quick flash duration in ms
    const uint8_t numFlashes = 3; // Number of flashes to indicate mode change

    // Store original brightness and temporarily increase it
    uint8_t originalBrightness = currentBrightness;
    currentBrightness = pulseBrightness;

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 200); // Blank all LEDs briefly to better mark the swatch pulse start

    // Flash the primary color quickly to indicate animation mode change
    for (uint8_t i = 0; i < numFlashes; i++) {
        // Bright flash
        for (uint8_t seg = 1; seg < gpioLEDs; seg++) { sendToRGB(seg, swatch[swNum].primary); }
        sendToRGB(0, swatch[swNum].primary);
        delay(flashDuration);

        // Dark flash
        for (uint8_t seg = 1; seg < gpioLEDs; seg++) { sendToRGB(seg, swatch[swNum].background); }
        sendToRGB(0, swatch[swNum].background);
        delay(flashDuration);
    }

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 200); // Same again here

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Save deferred from button handler — flash erase here is safe outside the PWM loop
    saveSettingsToFlash(swNum, currentBrightness, animationMode);

    // Reset the flag
    animationPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: fadeInLevel
// Returns 0–255 for a linear fade-in: 0 at t=0, 255 at t=duration.
static inline uint8_t fadeInLevel(unsigned long t, unsigned long duration) {
    return (t >= duration) ? 255 : (uint8_t)((t * 255UL) / duration);
}

// -------------------------------------------------------------------------------------
// MARK: fadeOutLevel
// Returns 0–255 for a linear fade-out: 255 at t=0, 0 at t=duration.
static inline uint8_t fadeOutLevel(unsigned long t, unsigned long duration) {
    return (t >= duration) ? 0 : (uint8_t)(((duration - t) * 255UL) / duration);
}

// -------------------------------------------------------------------------------------
// MARK: setSRChannel
// Blends color against base at the given level (0=base, 255=full color) into shiftRegColors[ch].
static void setSRChannel(uint8_t ch, const uint8_t* color, const uint8_t* base, uint8_t level) {
    for (uint8_t c = 0; c < 3; c++)
        shiftRegColors[ch][c] = base[c] + (uint8_t)(((int16_t)(color[c] - base[c]) * level) / 255);
}

// -------------------------------------------------------------------------------------
// MARK: calcPulseLevel
// Double-pulse brightness envelope: P1 rise 50 ms → P1 fall 70 ms → P2 rise 50 ms → P2 slow fall 400 ms.
// pt = milliseconds into the 570 ms pulse window. Returns 0–255.
static uint8_t calcPulseLevel(unsigned long pt) {
    if (pt < 50UL)  return fadeInLevel(pt, 50UL);            pt -= 50UL;  // P1 rise
    if (pt < 70UL)  return fadeOutLevel(pt, 70UL);           pt -= 70UL;  // P1 fall
    if (pt < 50UL)  return fadeInLevel(pt, 50UL);            pt -= 50UL;  // P2 rise
    if (pt < 400UL) return fadeOutLevel(pt, 400UL);                        // P2 slow fall
    return 0;
}

// -------------------------------------------------------------------------------------
// MARK: eyeDoublePulse
// Non-blocking, millis()-based pulse animation for the 4 SR eye channels.
// Reads animationMode to select sub-mode:
//   mode 1 — Pair:   ch0/ch2 (SR1/SR3) fire first, then ch1/ch3 (SR2/SR4).
//   mode 2 — Mirror: left eye (ch0/ch1) vs right eye (ch2/ch3), cycling between
//             in-phase and anti-phase every 3 s.
// Writes to shiftRegColors[] via setSRChannel; call this then sendToRGB(0, ...) to flush.
void eyeDoublePulse() {
    const bool mirrorMode = (animationMode == 2);

    const uint8_t flickerMin = 140;  // Minimum background flicker brightness (0-255)
    const uint8_t flickerMax = 160;  // Maximum background flicker brightness (0-255)

    static bool          mirrorInPhase = true;
    static unsigned long modeStartTime = 0;

    unsigned long now = millis();

    // Mirror phase alternates every 3 s
    if (modeStartTime == 0) modeStartTime = now;
    if (now - modeStartTime >= 3000UL) { mirrorInPhase = !mirrorInPhase; modeStartTime = now; }

    if (mirrorMode) {
        // Mode 2 — left eye (ch0/ch1) vs right eye (ch2/ch3), in-phase or anti-phase
        const unsigned long PULSE_MS = 570UL;
        const unsigned long CYCLE_MS = PULSE_MS + 220UL; // 790 ms total
        unsigned long tLeft  = now % CYCLE_MS;
        unsigned long tRight = mirrorInPhase ? tLeft : (tLeft + CYCLE_MS / 2) % CYCLE_MS;
        uint8_t flickerBri = (uint8_t)random(flickerMin, flickerMax + 1);
        for (uint8_t ch = 0; ch < 4; ch++) {
            unsigned long tCh = (ch < 2) ? tLeft : tRight;
            bool isPulse = (tCh < PULSE_MS);
            if (isPulse) {
                uint8_t lvl = calcPulseLevel(tCh);
                uint8_t gradColor[3];
                gradientPosition(lvl, gradColor);
                uint8_t bri = flickerMax + (uint8_t)(((uint16_t)(255 - flickerMax) * lvl) >> 8);
                for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * bri) >> 8);
            } else {
                for (uint8_t c = 0; c < 3; c++)
                    shiftRegColors[ch][c] = (uint8_t)(((uint16_t)swatch[swNum].background[c] * flickerBri) >> 8);
            }
        }
    } else {
        // Mode 1 — pair A (ch0/ch2) pulses first, then pair B (ch1/ch3)
        const unsigned long PULSE_MS = 570UL;
        const unsigned long CYCLE_MS = (PULSE_MS + 220UL) * 2; // 1580 ms
        unsigned long t = now % CYCLE_MS;
        int8_t  activePair  = (t < PULSE_MS)            ?  0
                            : (t < PULSE_MS + 220UL)     ? -1
                            : (t < PULSE_MS * 2 + 220UL) ?  1 : -1;
        unsigned long tOffset = (activePair == 1) ? t - (PULSE_MS + 220UL) : t;
        uint8_t flickerBri = (uint8_t)random(flickerMin, flickerMax + 1);
        for (uint8_t ch = 0; ch < 4; ch++) {
            bool isPairA = (ch == 0 || ch == 2); // ch0=SR1, ch2=SR3
            bool active  = (activePair == (isPairA ? 0 : 1));
            if (active) {
                uint8_t lvl = calcPulseLevel(tOffset);
                uint8_t gradColor[3];
                gradientPosition(lvl, gradColor);
                uint8_t bri = flickerMax + (uint8_t)(((uint16_t)(255 - flickerMax) * lvl) >> 8);
                for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * bri) >> 8);
            } else {
                for (uint8_t c = 0; c < 3; c++)
                    shiftRegColors[ch][c] = (uint8_t)(((uint16_t)swatch[swNum].background[c] * flickerBri) >> 8);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: eyeRipple
// Non-blocking traveling-wave animation for the 4 SR eye channels.
// Uses the sine waveform table (waveform[1]) with per-channel quarter-wave phase offsets
// so the wave appears to travel ch0 → ch1 → ch2 → ch3.
// RIPPLE_MS controls the speed of one full cycle.
void eyeRipple(uint8_t minGrad, uint8_t maxGrad, uint8_t brightness) {
    const unsigned long RIPPLE_MS        = 1200UL; // ms per full sine cycle
    const uint8_t       WAVE_STEPS       = 32;
    const uint8_t       phaseOffset[4]   = {0, 8, 16, 24}; // quarter-wave spacing per channel

    unsigned long now    = millis();
    uint8_t       baseIdx = (uint8_t)((now % RIPPLE_MS) * WAVE_STEPS / RIPPLE_MS);

    uint8_t numCh = (srLEDs < 4) ? srLEDs : 4;
    for (uint8_t ch = 0; ch < numCh; ch++) {
        uint8_t tableIdx = (baseIdx + phaseOffset[ch]) % WAVE_STEPS;
        uint8_t sineVal  = waveform[1].waveform[tableIdx]; // sine 0–255
        uint8_t lvl      = minGrad + (uint8_t)(((uint16_t)(maxGrad - minGrad) * sineVal) >> 8);
        uint8_t gradColor[3];
        gradientPosition(lvl, gradColor);
        for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * brightness) >> 8);
    }
}

// -------------------------------------------------------------------------------------
// MARK: eyeScatter
// Non-blocking independent glitch animation for SR eye pod channels.
// Each of the 4 SR channels has its own per-channel timer and independently picks random
// swatch colours and brightness levels.
void eyeScatter() {
    // Per-slot brightness cap (0-255): applied to the color before blending to background
    const uint8_t brightPrimary    = 255;
    const uint8_t brightAccent     = 100;
    const uint8_t brightMidtone    = 50;
    const uint8_t brightContrast   = 50;
    const uint8_t brightBackground = 50;

    static uint8_t       colorSlot[4]  = {3, 0, 1, 2};
    static uint8_t       level[4]      = {80, 60, 120, 40};
    static unsigned long nextChange[4] = {0, 0, 0, 0};

    unsigned long now  = millis();
    uint8_t       numCh = (srLEDs < 4) ? srLEDs : 4;

    for (uint8_t ch = 0; ch < numCh; ch++) {
        if (now >= nextChange[ch]) {
            uint8_t roll = (uint8_t)random(0, 10);
            if (roll < 5) {
                // Flicker: random swatch colour, mid-range brightness
                colorSlot[ch] = (uint8_t)random(0, 5);
                level[ch]     = (uint8_t)random(60, 200);
                nextChange[ch] = now + (unsigned long)random(40, 250);
            } else if (roll < 8) {
                // Brief bright flash: primary, accent or midtone at high brightness
                colorSlot[ch] = (uint8_t)random(0, 3);
                level[ch]     = (uint8_t)random(200, 255);
                nextChange[ch] = now + (unsigned long)random(20, 80);
            } else {
                // Dark pause: background-level colour, very low brightness
                colorSlot[ch] = 4;
                level[ch]     = (uint8_t)random(0, 30);
                nextChange[ch] = now + (unsigned long)random(50, 300);
            }
        }
        const uint8_t* color;
        uint8_t bright;
        switch (colorSlot[ch]) {
            case 0:  color = swatch[swNum].primary;    bright = brightPrimary;    break;
            case 1:  color = swatch[swNum].accent;     bright = brightAccent;     break;
            case 2:  color = swatch[swNum].midtone;    bright = brightMidtone;    break;
            case 3:  color = swatch[swNum].contrast;   bright = brightContrast;   break;
            default: color = swatch[swNum].background; bright = brightBackground; break;
        }
        uint8_t scaledColor[3];
        for (uint8_t c = 0; c < 3; c++)
            scaledColor[c] = (uint8_t)(((uint16_t)color[c] * bright) >> 8);
        setSRChannel(ch, scaledColor, swatch[swNum].background, level[ch]);
    }
}

// -------------------------------------------------------------------------------------
// MARK: brightnessAdjustmentMode
void brightnessAdjustmentMode() {
    const int cycleDuration = 4000; // 4 seconds total
    const int stepDuration = 100; // Update every 100ms

    unsigned long modeStartTime = millis();

    // Use white color for brightness display
    uint8_t whiteColor[3] = {255, 255, 255};

    while (brightnessAdjustMode && digitalRead(colorBtn) == LOW) {
        unsigned long elapsedTime = millis() - modeStartTime;

        // Simple triangle wave for brightness cycling (integer fixed-point)
        uint32_t phase = elapsedTime % (uint32_t)cycleDuration;
        uint8_t  ratio = (phase < (uint32_t)(cycleDuration / 2))
                         ? (uint8_t)(phase * 255UL / (uint32_t)(cycleDuration / 2))
                         : (uint8_t)(((uint32_t)cycleDuration - phase) * 255UL / (uint32_t)(cycleDuration / 2));
        // Map to brightness range
        currentBrightness = minBrightness + (uint8_t)(((uint16_t)(maxBrightness - minBrightness) * ratio) >> 8);

        // Display white at current brightness on all segments
        // SR segments are suppressed automatically by the brightnessAdjustMode check in sendToRGB
        for (int i = 0; i < 10; i++) { // Display multiple times per step for stability
            for (uint8_t seg = 1; seg < gpioLEDs; seg++) { sendToRGB(seg, whiteColor); }
            sendToRGB(0, whiteColor);
            delay(stepDuration / 10);
        }
    }
    // Save deferred from button handler — flash erase here is safe outside the PWM loop
    saveSettingsToFlash(swNum, currentBrightness, animationMode);

    // Reset mode flag
    brightnessAdjustMode = false;
}
