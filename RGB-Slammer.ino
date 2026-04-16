/*
RGB Slammer
Written by Tully Jagoe 2025
MIT License

This script is best edited in VSCode for color token selection in swatches.cpp
*/


#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "waveforms.h"
#include "flashStorage.h"
#include "pinouts.h"

// Number of directly-driven LED segments (set from active hardware config at boot)

// Select which hardware configuration to use
ConfigType activeConfig = NANOFRAME;

// Define the LED array and button pins according to the active configuration
ledSegment led[MAX_LED_SEGMENTS];
uint8_t colorBtn;
uint8_t animBtn;

// Shift register pin configuration
shiftRegPins shiftReg;
// Initialise LED channels (will be updated during boot based on active config)
uint8_t coreLEDs = 0;
uint8_t extLEDs = 0;

// Brightness stored as 8-bit fixed-point: 0-255 = 0.0-1.0.
// Default 0.40 → 102; pulse preview 0.65 → 166.
uint8_t currentBrightness = 120;
// Temporary brightness value used when previewing a new swatch
uint8_t pulseBrightness = 150;

// Individual section brightness adjustment
uint8_t coreOutputScale = 100;
uint8_t srOutputScale   = 255;

// Brightness adjustment mode limits (0-255 fixed-point: 0.30→77, 0.60→153)
const uint8_t minBrightness = 75;
const uint8_t maxBrightness = 150;
const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// Slow down all animations by this amou nt (in milliseconds)
const uint8_t slowDown = 1;

// LED Color tuning
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values for standard forward current
// {mA, luminosity}
const luminance red       = {5, 100};
const luminance green     = {5, 100};
const luminance blue      = {5, 100};


extern void (*srUpdateCallback)(); // defined in rgbProcessor.cpp

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig(activeConfig);

    // Copy pin configuration from the selected hardware profile
    for (uint8_t i = 0; i < config.coreLEDs && i < MAX_LED_SEGMENTS; i++) {
        led[i] = config.leds[i];
    }
    coreLEDs  = config.coreLEDs;
    colorBtn = config.colorButton;
    animBtn  = config.animButton;

    // Copy shift register configuration
    shiftReg            = config.shiftReg;
    extLEDs = config.shiftRegChannels;

    // Set up all LED segments (GPIO only — SR segments have no pins to configure)
    for (uint8_t segment = 0; segment < coreLEDs; segment++) {
        if (!led[segment].isSR) {
            pinMode(led[segment].red, OUTPUT);
            pinMode(led[segment].green, OUTPUT);
            pinMode(led[segment].blue, OUTPUT);
        }
    }

    // Set up shift register pins if configured
    if (extLEDs > 0) {
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
    pinMode(animBtn, INPUT_PULLUP);

    // Calculate the luminosity modifiers
    calculateLuminance();

    // Try to load saved settings from flash
    if (!loadSettingsFromFlash(&swNum, &currentBrightness, &animationMode)) {
        // If no valid settings found, use defaults (which are already set in declarations)
        swNum = 14;
        animationMode = 0;
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
                //glitchLoop(20, 10);
                cautionCitizen();
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
        fadeToColor(swatch[swNum].primary,      swatch[swNum].background,   speed);
        fadeToColor(swatch[swNum].accent,       swatch[swNum].primary,      speed);
        fadeToColor(swatch[swNum].midtone,      swatch[swNum].accent,       speed);
        fadeToColor(swatch[swNum].contrast,     swatch[swNum].midtone,      speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].contrast,     speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].background,   speed);
    }
    showColor(swatch[0].background, swatch[0].background, speed*5);
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
        if (swatchPreviewActive || animationPreviewActive) {
            return;
        }

        // SR channel: register the SR function for this iteration.
        // sendToRGB will call it automatically before each Core flush (including inside glitch effects).
        srUpdateCallback = (random(0, 100) < srEffectChance) ? eyeScatter : eyeDoublePulse;

        // Core channel: probability roll selects a blocking glitch effect or the default flicker.
        if (random(0, 100) < coreEffectChance) {
            uint8_t flickerSegment = 0;
            for (uint8_t s = 0; s < coreLEDs; s++) { if (led[s].role == ROLE_CORE) { flickerSegment = s; break; } }
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
            // Default: flicker()'s sendToRGB flushes shiftRegColors[] and Core simultaneously.
            for (uint8_t seg = 0; seg < coreLEDs; seg++) {
                if (led[seg].role == ROLE_CORE) {
                    flicker(seg, flickerChance, 200, 255);
                }
            }
        }
    }
}

// No-arg wrapper: ripple between contrast and background — used as srUpdateCallback idle in cautionCitizen.
static void cautionRipple() { eyeRipple(192, 255); }

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
    for (uint8_t s = 0; s < coreLEDs; s++) { if (led[s].role == ROLE_CORE) { coreSeg = s; break; } }

    srUpdateCallback = cautionRipple;

    while (true) {
        if (swatchPreviewActive || animationPreviewActive) return;

        if (millis() - lastStrobeTime >= STROBE_INTERVAL_MS) {
            // Strobe burst: null callback so showColor drives SR directly
            srUpdateCallback = nullptr;
            for (uint8_t i = 0; i < STROBE_REPS; i++) {
                if (swatchPreviewActive || animationPreviewActive) break;
                showColor(swatch[swNum].accent,   swatch[swNum].primary,   FLASH_ON_MS);
                showColor(swatch[swNum].contrast, swatch[swNum].contrast,  FLASH_OFF_MS);
            }
            lastStrobeTime = millis();
            srUpdateCallback = cautionRipple;
        } else {
            // Idle: flicker drives the core flush; cautionRipple callback animates SR
            flicker(coreSeg, 70, 200, 255);
        }
    }
}


// -------------------------------------------------------------------------------------
// MARK: fadeToColor
// color1 → ROLE_CORE segments, color2 → ROLE_EXT segments.
// If srUpdateCallback is set, it runs during the ROLE_CORE flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive ROLE_EXT directly with color2.
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const int fadeTime){
    uint8_t startColor[MAX_LED_SEGMENTS][3];
    uint8_t output[MAX_LED_SEGMENTS][3];

    // Copy handoverColor to startColor for all segments
    for (uint8_t segment = 0; segment < coreLEDs; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            startColor[segment][pin] = handoverColor[segment][pin];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        uint8_t fadeRatio = (uint8_t)(((unsigned long)(millis() - startTime) * 255UL) / (unsigned long)fadeTime);
        for (int pin = 0; pin < 3; pin++) {
            for (uint8_t seg = 0; seg < coreLEDs; seg++) {
                const uint8_t* target = (led[seg].role == ROLE_CORE) ? color1 : color2;
                int16_t delta = (int16_t)target[pin] - (int16_t)startColor[seg][pin];
                output[seg][pin] = (uint8_t)((int16_t)startColor[seg][pin] + (int16_t)(((int32_t)delta * fadeRatio) >> 8));
            }
        }
        // SR-first: buffer ROLE_EXT color2, then ROLE_CORE flush sends both.
        // If srUpdateCallback is set it fires during the ROLE_CORE flush and overrides the buffer.
        for (uint8_t seg = 0; seg < coreLEDs; seg++) {
            if (led[seg].role == ROLE_EXT) sendToRGB(seg, output[seg]);
        }
        for (uint8_t seg = 0; seg < coreLEDs; seg++) {
            if (led[seg].role == ROLE_CORE) sendToRGB(seg, output[seg]);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
// color1 → ROLE_CORE segments, color2 → ROLE_EXT segments.
// If srUpdateCallback is set, it runs during the ROLE_CORE flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive ROLE_EXT directly with color2.
void showColor(uint8_t color1[3], uint8_t color2[3], int duration){
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }
        // SR-first: buffer ROLE_EXT color2, then ROLE_CORE flush sends both.
        // If srUpdateCallback is set it fires during the ROLE_CORE flush and overrides the buffer.
        for (uint8_t seg = 0; seg < coreLEDs; seg++) {
            if (led[seg].role == ROLE_EXT) sendToRGB(seg, color2);
        }
        for (uint8_t seg = 0; seg < coreLEDs; seg++) {
            if (led[seg].role == ROLE_CORE) sendToRGB(seg, color1);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
// Works for any segment role. If pin is an SR segment, suppresses srUpdateCallback
// and triggers a ROLE_CORE flush so the buffered SR color is output immediately.
void flicker(const uint8_t pin, const uint8_t chance, const uint8_t min, const uint8_t max){
    uint8_t outputColor[3];
    uint8_t range = random(min, max);
    gradientPosition(range, outputColor);
    sendToRGB(pin, outputColor);
    // SR segments buffer only — need a GPIO flush to output. Suppress callback so
    // hand-written SR color is not overwritten, then flush via the ROLE_CORE segment.
    if (pin < coreLEDs && led[pin].isSR) {
        void (*saved)() = srUpdateCallback;
        srUpdateCallback = nullptr;
        for (uint8_t s = 0; s < coreLEDs; s++) {
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
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        // Animate the active segment; the opposite role always holds contrast
        if (led[segment].role == ROLE_CORE) {
            showColor(swatch[swNum].accent,      swatch[swNum].contrast, 50);
            showColor(swatch[swNum].background,  swatch[swNum].contrast, 50);
        } else {
            showColor(swatch[swNum].contrast, swatch[swNum].accent,      50);
            showColor(swatch[swNum].contrast, swatch[swNum].background,  50);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch2
void coreGlitch2(uint8_t color1[3], uint8_t color2[3], int duration) {
    // Part 1: Rapidly flash between black and background for 1 second
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }
        rapidPulse(color1, color2, 50);
    }

    // Part 2: Rapidly fade core through all swatch colors; eyes hold contrast throughout
    uint8_t fadeTime = 50;
    fadeToColor(swatch[swNum].background,   swatch[swNum].contrast, fadeTime);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast, fadeTime);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].contrast, fadeTime);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].contrast, fadeTime);
    fadeToColor(swatch[swNum].primary,      swatch[swNum].contrast, fadeTime);
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch3
void coreGlitch3(uint8_t segment, uint8_t color2[3], int duration,  uint8_t reps) {
    uint8_t startColor[3] = {handoverColor[segment][0], handoverColor[segment][1], handoverColor[segment][2]};
    // Find a representative segment of the opposite role for handover reference
    uint8_t otherSegment = 0;
    if (led[segment].role == ROLE_CORE) {
        for (uint8_t s = 0; s < coreLEDs; s++) { if (led[s].role == ROLE_EXT)  { otherSegment = s; break; } }
    } else {
        for (uint8_t s = 0; s < coreLEDs; s++) { if (led[s].role == ROLE_CORE) { otherSegment = s; break; } }
    }
    // Hold otherSegment at its handoverColor, and flash segment between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        if (led[segment].role == ROLE_CORE) {
            showColor(startColor, handoverColor[otherSegment], duration);
            showColor(color2, handoverColor[otherSegment], duration);
        } else {
            showColor(handoverColor[otherSegment], startColor, duration);
            showColor(handoverColor[otherSegment], color2, duration);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: coreGlitch4
void coreGlitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();
    while (millis() - start < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        for (uint8_t segment = 0; segment < coreLEDs; segment++) {
            if (led[segment].role == ROLE_EXT) continue;
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
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        // Get color at this position in the gradient
        gradientPosition(waveform[waveformIndex].waveform[i], outputColor);

        // Show this color on the core LED; eyes hold contrast
        showColor(outputColor, swatch[swNum].contrast, 50);

        // Brief black flash on core every few steps for a glitchy effect
        if (i % 4 == 0) {
            uint8_t blackColor[3] = {0, 0, 0};
            showColor(blackColor, swatch[swNum].contrast, 10);
        }
    }

    // Then do some rapid random jumps between waveform positions
    for (uint8_t i = 0; i < 8; i++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        uint8_t randomPos = random(0, 32);
        gradientPosition(waveform[waveformIndex].waveform[randomPos], outputColor);
        showColor(outputColor, swatch[swNum].contrast, 30);

        // Brief flashes to black between jumps
        uint8_t blackColor[3] = {0, 0, 0};
        showColor(blackColor, swatch[swNum].contrast, 15);
    }

    // End with a final dramatic fade to black on core; eyes hold contrast
    fadeToColor(swatch[swNum].background, swatch[swNum].contrast, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t color1[3], uint8_t color2[3], int speed){
    showColor(color1, color2, speed); // core = color1, eye = color2
    showColor(color2, color2, speed); // core = color2, eye = color2
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
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        int selection = random(0, 3);
        // Core flickers between gradient positions; eyes hold contrast throughout
        if (selection == 0) {
            showColor(output1, swatch[swNum].contrast, interval);
        } else if (selection == 1) {
            showColor(output2, swatch[swNum].contrast, interval);
        } else {
            showColor(output1, swatch[swNum].contrast, interval);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: swatchPreview
void swatchPreview() {
    const uint8_t fadeUpDuration = 50; // 0.2 seconds fade up
    const uint8_t fadeDownDuration = 400; // 0.6 seconds fade down
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

        for (uint8_t seg = 1; seg < coreLEDs; seg++) { sendToRGB(seg, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (uint8_t i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t seg = 1; seg < coreLEDs; seg++) { sendToRGB(seg, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeDownDuration / fadeDownSteps);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Reset the flag
    swatchPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: animationPreview
void animationPreview() {
    const uint8_t flashDuration = 100; // Quick flash duration in ms
    const uint8_t numFlashes = 3; // Number of flashes to indicate mode change

    // Store original brightness and temporarily increase it
    uint8_t originalBrightness = currentBrightness;
    currentBrightness = pulseBrightness;

    // Flash the primary color quickly to indicate animation mode change
    for (uint8_t i = 0; i < numFlashes; i++) {
        // Bright flash
        for (uint8_t seg = 1; seg < coreLEDs; seg++) { sendToRGB(seg, swatch[swNum].primary); }
        sendToRGB(0, swatch[swNum].primary);
        delay(flashDuration);

        // Dark flash
        for (uint8_t seg = 1; seg < coreLEDs; seg++) { sendToRGB(seg, swatch[swNum].background); }
        sendToRGB(0, swatch[swNum].background);
        delay(flashDuration);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

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

    static bool          mirrorInPhase = true;
    static unsigned long modeStartTime = 0;

    unsigned long now = millis();

    // Ripple background for inactive channels (mirrors eyeRipple logic)
    const uint8_t rPhaseOffset[4] = {0, 8, 16, 24};
    uint8_t       rBaseIdx = (uint8_t)((now % 1200UL) * 32UL / 1200UL);

    // Mirror phase alternates every 3 s
    if (modeStartTime == 0) modeStartTime = now;
    if (now - modeStartTime >= 3000UL) { mirrorInPhase = !mirrorInPhase; modeStartTime = now; }

    if (mirrorMode) {
        // Mode 2 — left eye (ch0/ch1) vs right eye (ch2/ch3), in-phase or anti-phase
        const unsigned long PULSE_MS = 570UL;
        const unsigned long CYCLE_MS = PULSE_MS + 220UL; // 790 ms total
        unsigned long tLeft  = now % CYCLE_MS;
        unsigned long tRight = mirrorInPhase ? tLeft : (tLeft + CYCLE_MS / 2) % CYCLE_MS;
        for (uint8_t ch = 0; ch < 4; ch++) {
            unsigned long tCh = (ch < 2) ? tLeft : tRight;
            uint8_t lvl = (tCh < PULSE_MS) ? calcPulseLevel(tCh) : waveform[1].waveform[(rBaseIdx + rPhaseOffset[ch]) % 32];
            uint8_t gradColor[3];
            gradientPosition(lvl, gradColor);
            for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = gradColor[c];
        }
    } else {
        // Mode 1 — pair A (ch0/ch2) pulses first, then pair B (ch1/ch3)
        const unsigned long PULSE_MS = 570UL;
        const unsigned long CYCLE_MS = (PULSE_MS + 220UL) * 2; // 1580 ms
        unsigned long t = now % CYCLE_MS;
        int8_t  activePair = (t < PULSE_MS)            ?  0
                           : (t < PULSE_MS + 220UL)     ? -1
                           : (t < PULSE_MS * 2 + 220UL) ?  1 : -1;
        unsigned long tOffset = (activePair == 1) ? t - (PULSE_MS + 220UL) : t;
        for (uint8_t ch = 0; ch < 4; ch++) {
            bool    isPairA = (ch == 0 || ch == 2); // ch0=SR1, ch2=SR3
            bool    active  = (activePair == (isPairA ? 0 : 1));
            uint8_t lvl     = active ? calcPulseLevel(tOffset) : waveform[1].waveform[(rBaseIdx + rPhaseOffset[ch]) % 32];
            uint8_t gradColor[3];
            gradientPosition(lvl, gradColor);
            for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = gradColor[c];
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: eyeRipple
// Non-blocking traveling-wave animation for the 4 SR eye channels.
// Uses the sine waveform table (waveform[1]) with per-channel quarter-wave phase offsets
// so the wave appears to travel ch0 → ch1 → ch2 → ch3.
// RIPPLE_MS controls the speed of one full cycle.
void eyeRipple(uint8_t minGrad, uint8_t maxGrad) {
    const unsigned long RIPPLE_MS        = 1200UL; // ms per full sine cycle
    const uint8_t       WAVE_STEPS       = 32;
    const uint8_t       phaseOffset[4]   = {0, 8, 16, 24}; // quarter-wave spacing per channel

    unsigned long now    = millis();
    uint8_t       baseIdx = (uint8_t)((now % RIPPLE_MS) * WAVE_STEPS / RIPPLE_MS);

    uint8_t numCh = (extLEDs < 4) ? extLEDs : 4;
    for (uint8_t ch = 0; ch < numCh; ch++) {
        uint8_t tableIdx = (baseIdx + phaseOffset[ch]) % WAVE_STEPS;
        uint8_t sineVal  = waveform[1].waveform[tableIdx]; // sine 0–255
        uint8_t lvl      = minGrad + (uint8_t)(((uint16_t)(maxGrad - minGrad) * sineVal) >> 8);
        uint8_t gradColor[3];
        gradientPosition(lvl, gradColor);
        for (uint8_t c = 0; c < 3; c++) shiftRegColors[ch][c] = gradColor[c];
    }
}

// -------------------------------------------------------------------------------------
// MARK: eyeScatter
// Non-blocking independent glitch animation for SR eye pod channels.
// Each of the 4 SR channels has its own per-channel timer and independently picks random
// swatch colours and brightness levels.
void eyeScatter() {
    static uint8_t       colorSlot[4]  = {3, 0, 1, 2};
    static uint8_t       level[4]      = {80, 60, 120, 40};
    static unsigned long nextChange[4] = {0, 0, 0, 0};

    unsigned long now  = millis();
    uint8_t       numCh = (extLEDs < 4) ? extLEDs : 4;

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
        switch (colorSlot[ch]) {
            case 0:  color = swatch[swNum].primary;    break;
            case 1:  color = swatch[swNum].accent;     break;
            case 2:  color = swatch[swNum].midtone;    break;
            case 3:  color = swatch[swNum].contrast;   break;
            default: color = swatch[swNum].background; break;
        }
        setSRChannel(ch, color, swatch[swNum].background, level[ch]);
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
            for (uint8_t seg = 1; seg < coreLEDs; seg++) { sendToRGB(seg, whiteColor); }
            sendToRGB(0, whiteColor);
            delay(stepDuration / 10);
        }
    }
    // Reset mode flag
    brightnessAdjustMode = false;
}
