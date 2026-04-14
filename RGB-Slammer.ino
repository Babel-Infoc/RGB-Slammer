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

// Forward declarations
void animationPreview();

// Number of directly-driven LED segments (set from active hardware config at boot)
uint8_t numLEDs = 1;

// Select which hardware configuration to use
ConfigType activeConfig = NANOFRAME;

// Define the LED array and button pins according to the active configuration
ledSegment led[MAX_LED_SEGMENTS];
uint8_t colorBtn;
uint8_t animBtn;

// Shift register pin configuration
shiftRegPins shiftReg;
// Initialize with 0 channels by default; will be set from active configuration at boot
uint8_t numShiftRegChannels = 0;

// Set the default brightness modifier, 0.0 to 0.65 max
float currentBrightness = 0.4;
// Temporary brightness value to be used when previewing the new swatch after changing it
float pulseBrightness = 0.65;

// Per-type output scale — applied after currentBrightness, luminance and gamma correction.
// Use these to balance perceived brightness between Core GPIO LEDs and SR pod LEDs
// independently of the global brightness control. Range 0.0 – 1.0.
float coreOutputScale = 0.7;  // Scale factor for directly-driven GPIO LED segments
float srOutputScale   = 1.0;  // Scale factor for shift-register LED segments

// Brightness adjustment settings
const float minBrightness = 0.3;
const float maxBrightness = 0.6;
const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// Slow down all animations by this amou nt (in milliseconds)
const uint8_t slowDown = 1;

// LED Color tuning
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values for standard forward current
// {mA, luminosity}
const luminance red       = {5, 180};
const luminance green     = {5, 450};
const luminance blue      = {5, 71};

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig(activeConfig);

    // Copy pin configuration from the selected hardware profile
    for (uint8_t i = 0; i < config.numLEDs && i < MAX_LED_SEGMENTS; i++) {
        led[i] = config.leds[i];
    }
    numLEDs  = config.numLEDs;
    colorBtn = config.colorButton;
    animBtn  = config.animButton;

    // Copy shift register configuration
    shiftReg            = config.shiftReg;
    numShiftRegChannels = config.shiftRegChannels;

    // Set up all LED segments (GPIO only — SR segments have no pins to configure)
    for (uint8_t segment = 0; segment < numLEDs; segment++) {
        if (!led[segment].isSR) {
            pinMode(led[segment].red, OUTPUT);
            pinMode(led[segment].green, OUTPUT);
            pinMode(led[segment].blue, OUTPUT);
        }
    }

    // Set up shift register pins if configured
    if (numShiftRegChannels > 0) {
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

    // Set up random seeds
    float randSeed1(analogRead(0));
    float randSeed2(analogRead(1));


    // Try to load saved settings from flash
    if (!loadSettingsFromFlash(&swNum, &currentBrightness, &animationMode)) {
        // If no valid settings found, use defaults (which are already set in declarations)
        swNum = 22;
        currentBrightness = 0.4; // Default brightness
        animationMode = 0; // Default to glitchLoop
    }

    // Show the bootup animation
    bounceBoot(40);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/
// Only runs the glitchLoop animation

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
                glitchLoop(70, 20, 1000);
                break;
            case 1:
                // Eye animation: SR pods run their pulse cycle while the core LED holds swatch background.
                // updateEyeAnimation() writes directly to shiftRegColors[]; the sendToRGB(0,...) call
                // flushes those values to the shift registers via the GPIO PWM frame.
                updateEyeAnimation();
                sendToRGB(0, swatch[swNum].background);
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
// Advanced neon flicker with 3 different animation patterns selected randomly
void glitchLoop(const uint8_t flickerChance, const uint8_t effectChance, const int duration) {
    // For <duration> milliseconds, both LED segments will either play a special animation or the normal flicker
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    bool effectTrigger = random(0, 100) < effectChance;
    while (currentTime - startTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        if (effectTrigger) {
            // Apply a special effect
            uint8_t flickerSegment = random(0, numLEDs);
            // Pick a random glitch effect
            switch (random(0, 6)) {
                case 0:
                    glitch1(flickerSegment, 700);
                    break;
                case 1:
                    glitch2(swatch[swNum].midtone ,swatch[swNum].contrast, 700);
                    break;
                case 2:
                    glitch3(flickerSegment, swatch[swNum].primary, 20, 3);
                    break;
                case 3:
                    glitch4(6, 700);
                    break;
                case 4:
                    glitch5();
                    break;
                case 5:
                    fakeMorse(65, 210, 400);
                    break;
            }
            currentTime = millis();
        } else {
            // Normal flicker — drive all direct segments
            for (uint8_t seg = 0; seg < numLEDs; seg++) {
                flicker(seg, flickerChance, seg == 0 ? 200 : 0, seg == 0 ? 255 : 200);
            }
            currentTime = millis();
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: fadeToColor
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const int fadeTime){
    uint8_t startColor[MAX_LED_SEGMENTS][3];
    uint8_t output[MAX_LED_SEGMENTS][3];

    // Copy handoverColor to startColor for all segments
    for (uint8_t segment = 0; segment < numLEDs; segment++) {
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

        float fadeRatio = (float)(millis() - startTime) / fadeTime;
        for (int pin = 0; pin < 3; pin++) {
            // Segment 0 fades toward color1; all others fade toward color2
            output[0][pin] = startColor[0][pin] + (color1[pin] - startColor[0][pin]) * fadeRatio;
            for (uint8_t seg = 1; seg < numLEDs; seg++) {
                output[seg][pin] = startColor[seg][pin] + (color2[pin] - startColor[seg][pin]) * fadeRatio;
            }
        }
        // Update SR segment buffers first, then trigger GPIO PWM + SR clock-out
        for (uint8_t seg = 1; seg < numLEDs; seg++) {
            sendToRGB(seg, output[seg]);
        }
        sendToRGB(0, output[0]);
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
void showColor(uint8_t color1[3], uint8_t color2[3], int duration){
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }
        // Update SR segment buffers first, then trigger GPIO PWM + SR clock-out
        for (uint8_t seg = 1; seg < numLEDs; seg++) {
            sendToRGB(seg, color2);
        }
        sendToRGB(0, color1);
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
void flicker(const uint8_t pin, const uint8_t chance, const uint8_t min, const uint8_t max){
    uint8_t outputColor[3];
    uint8_t range = random(min, max);
    gradientPosition(range, outputColor);
    sendToRGB(pin, outputColor);
}

// -------------------------------------------------------------------------------------
// MARK: glitch1
void glitch1(const uint8_t segment, int duration){
    uint8_t otherSegment = 0;
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        if (segment == 0) {
            showColor(swatch[swNum].contrast, swatch[swNum].accent,50);
            showColor(swatch[swNum].contrast, swatch[swNum].background,50);
        } else {
            showColor(swatch[swNum].accent, swatch[swNum].contrast,50);
            showColor(swatch[swNum].background, swatch[swNum].contrast,50);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch2
void glitch2(uint8_t color1[3], uint8_t color2[3], int duration) {
    // Part 1: Rapidly flash between black and background for 1 second
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }
        rapidPulse(color1, color2, 50);
    }

    // Part 2: Rapidly fade through all swatch colors from background to primary
    uint8_t fadeTime = 50; // Quick fade time between colors
    // Fade through the colors in sequence: background → contrast → midtone → accent → primary
    fadeToColor(swatch[swNum].background,   swatch[swNum].background,   fadeTime);
    fadeToColor(swatch[swNum].contrast,     swatch[swNum].contrast,     fadeTime);
    fadeToColor(swatch[swNum].midtone,      swatch[swNum].midtone,      fadeTime);
    fadeToColor(swatch[swNum].accent,       swatch[swNum].accent,       fadeTime);
    fadeToColor(swatch[swNum].primary,      swatch[swNum].primary,      fadeTime);
}

// -------------------------------------------------------------------------------------
// MARK: glitch3
void glitch3(uint8_t segment, uint8_t color2[3], int duration,  uint8_t reps) {
    uint8_t startColor[3] = {handoverColor[segment][0], handoverColor[segment][1], handoverColor[segment][2]};
    uint8_t otherSegment = 0;
    if (segment == 0) {otherSegment = 1;}
    // Hold otherSegment at its handoverColor, and flash segment between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        if (segment == 0) {
            showColor(startColor, handoverColor[1], duration);
            showColor(color2, handoverColor[1], duration);
        } else {
            showColor(handoverColor[0], startColor, duration);
            showColor(handoverColor[0], color2, duration);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch4
void glitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();
    while (millis() - start < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            gradientPosition(random(1, 255), color);
            for (uint8_t i = 0; i < reps; i++) {
                sendToRGB(segment, color);
                sendToRGB(segment, swatch[swNum].contrast);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch5
void glitch5(){
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

        // Show this color on both LEDs briefly
        showColor(outputColor, outputColor, 50);

        // Brief black flash every few steps for a glitchy effect
        if (i % 4 == 0) {
            uint8_t blackColor[3] = {0, 0, 0};
            showColor(blackColor, blackColor, 10);
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
        showColor(outputColor, outputColor, 30);

        // Brief flashes to black between jumps
        uint8_t blackColor[3] = {0, 0, 0};
        showColor(blackColor, blackColor, 15);
    }

    // End with a final dramatic fade to black
    fadeToColor(swatch[swNum].contrast, swatch[swNum].background, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t color1[3], uint8_t color2[3], int speed){
    showColor(color1, color1, speed);
    showColor(color2, color2, speed);
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
        // Set LED colors based on selection
        if (selection == 0) {
            showColor(output1, output2, interval);
        } else if (selection == 1) {
            // Second segment gets color2, first gets color1
            showColor(output2, output1, interval);
        } else {
            // Neither selected, both get color1
            showColor(output1, output1, interval);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: swatchPreview
void swatchPreview() {
    const int fadeUpDuration = 50; // 0.2 seconds fade up
    const int fadeDownDuration = 400; // 0.6 seconds fade down
    const int fadeUpSteps = 20; // Steps for fade up
    const int fadeDownSteps = 60; // Steps for fade down

    // Store original brightness and temporarily increase it
    float originalBrightness = currentBrightness;
    currentBrightness = pulseBrightness;

    // Phase 1: Fade UP quickly from dark to bright over 0.2 seconds
    for (int i = 0; i < fadeUpSteps; i++) {
        uint8_t gradientPos = (i * 255) / (fadeUpSteps - 1); // Start at 0 (bright), go to 255 (dark)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t seg = 1; seg < numLEDs; seg++) { sendToRGB(seg, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (int i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t seg = 1; seg < numLEDs; seg++) { sendToRGB(seg, outputColor); }
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
    const int flashDuration = 100; // Quick flash duration in ms
    const int numFlashes = 3; // Number of flashes to indicate mode change

    // Store original brightness and temporarily increase it
    float originalBrightness = currentBrightness;
    currentBrightness = pulseBrightness;

    // Flash the primary color quickly to indicate animation mode change
    for (int i = 0; i < numFlashes; i++) {
        // Bright flash
        for (uint8_t seg = 1; seg < numLEDs; seg++) { sendToRGB(seg, swatch[swNum].primary); }
        sendToRGB(0, swatch[swNum].primary);
        delay(flashDuration);

        // Dark flash
        for (uint8_t seg = 1; seg < numLEDs; seg++) { sendToRGB(seg, swatch[swNum].background); }
        sendToRGB(0, swatch[swNum].background);
        delay(flashDuration);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Reset the flag
    animationPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: updateEyeAnimation
// Non-blocking, millis()-based animation for the 4 SR eye channels.
// Writes directly to shiftRegColors[]; call this then sendToRGB(0, ...) to flush
// the updated values to the shift registers via the GPIO PWM frame.
//
// Used by animation mode 1 in loop(). For unified animations that drive all
// segments equally, use the standard animation functions instead (glitchLoop, etc.).
//
// Physical layout on the glasses:
//   Ch0 (shiftRegColors[0])  Ch2 (shiftRegColors[2])  <- top    (Pair A — primary)
//   Ch1 (shiftRegColors[1])  Ch3 (shiftRegColors[3])  <- bottom (Pair B — accent)
//
// Normal cycle: alternating double-pulse per pair with contrast shimmer on idle channels.
// Random glitch: every 10–20 s, a 2-second blink event fires on a random channel.
void updateEyeAnimation() {

    // --- Periodic random eye glitch ---
    // Every 10–20 s, one channel blinks rapidly for 2 s while others shimmer at contrast.
    static unsigned long nextGlitchTime  = 0;   // 0 = not yet scheduled
    static unsigned long glitchStartTime = 0;
    static uint8_t       glitchChannel   = 255; // 255 = no active glitch

    unsigned long now = millis();

    // Schedule first / next glitch when idle
    if (glitchChannel == 255 && nextGlitchTime == 0) {
        nextGlitchTime = now + 10000UL + (unsigned long)random(0, 10001);
    }

    // Fire when the timer expires
    if (glitchChannel == 255 && now >= nextGlitchTime) {
        glitchChannel   = (uint8_t)random(0, 4);
        glitchStartTime = now;
        nextGlitchTime  = 0; // Reschedule after glitch ends
    }

    // Render the 2 s glitch event.
    // The selected channel blinks: 10 ms on (primary) / 40 ms off (contrast) per 50 ms cycle.
    // A new random channel is chosen on each rising edge.
    // All other channels shimmer at contrast throughout.
    if (glitchChannel != 255) {
        if (now - glitchStartTime < 2000UL) {
            // Throttled shimmer noise — update at most once per 50 ms
            static uint8_t       glitchNoise     = 0;
            static unsigned long lastGlitchNoise = 0;
            if (now - lastGlitchNoise >= 20UL) {
                glitchNoise     = (uint8_t)random(0, 22);
                lastGlitchNoise = now;
            }

            // 10 ms flash in every 50 ms cycle; pick a new random channel on each rising edge
            bool flashOn = ((now - glitchStartTime) % 20UL) < 10UL;
            static bool prevFlashOn = false;
            if (flashOn && !prevFlashOn) {
                glitchChannel = (uint8_t)random(0, 4);
            }
            prevFlashOn = flashOn;

            for (uint8_t ch = 0; ch < 4; ch++) {
                const uint8_t* base = swatch[swNum].contrast;
                if (ch == glitchChannel) {
                    for (uint8_t c = 0; c < 3; c++)
                        shiftRegColors[ch][c] = flashOn ? swatch[swNum].primary[c] : base[c];
                } else {
                    const uint8_t* color = (ch == 0 || ch == 2) ? swatch[swNum].primary
                                                                : swatch[swNum].accent;
                    for (uint8_t c = 0; c < 3; c++)
                        shiftRegColors[ch][c] = base[c] + (uint8_t)(((int16_t)(color[c] - base[c]) * glitchNoise) / 255);
                }
            }
            return; // Skip normal animation during glitch
        }
        glitchChannel = 255; // Glitch complete — reschedule on next call
    }

    // --- Normal double-pulse cycle ---
    // Timing constants (milliseconds)
    const unsigned long P1_UP   =  50UL; // pulse 1 rise  (2× speed)
    const unsigned long P1_DOWN =  50UL; // pulse 1 fall  (2× speed)
    const unsigned long IGAP    =  40UL; // rest between pulses (2× speed)
    const unsigned long P2_UP   =  50UL; // pulse 2 rise
    const unsigned long P2_DOWN = 450UL; // pulse 2 slow fade out
    const unsigned long PGAP    = 220UL; // gap between pairs (both at contrast)

    const unsigned long PAIR_MS  = P1_UP + P1_DOWN + IGAP + P2_UP + P2_DOWN;
    const unsigned long CYCLE_MS = (PAIR_MS + PGAP) * 2;

    unsigned long t = now % CYCLE_MS;

    uint8_t brightness = 0;
    int8_t  activePair = -1; // -1 = both at contrast, 0 = top row, 1 = bottom row

    // Returns brightness (0-255) for a position within a pair's PAIR_MS slot
    auto pairBrightness = [&](unsigned long pt) -> uint8_t {
        if (pt < P1_UP)   return (uint8_t)((pt * 255UL) / P1_UP);
        pt -= P1_UP;
        if (pt < P1_DOWN) return (uint8_t)(((P1_DOWN - pt) * 255UL) / P1_DOWN);
        pt -= P1_DOWN;
        if (pt < IGAP)    return 0;
        pt -= IGAP;
        if (pt < P2_UP)   return (uint8_t)((pt * 255UL) / P2_UP);
        pt -= P2_UP;
        return (uint8_t)(((P2_DOWN - pt) * 255UL) / P2_DOWN); // slow fade-out
    };

    if (t < PAIR_MS) {
        activePair = 0;
        brightness = pairBrightness(t);
    } else if (t < PAIR_MS + PGAP) {
        activePair = -1;
    } else if (t < PAIR_MS * 2 + PGAP) {
        activePair = 1;
        brightness = pairBrightness(t - (PAIR_MS + PGAP));
    }
    // else: second pair gap — activePair stays -1

    // Flicker sample for inactive channels — updated at most once per 50 ms
    // so the shimmer is visible rather than per-frame noise.
    static uint8_t       baseFlicker       = 0;
    static unsigned long lastFlickerSample = 0;
    if (now - lastFlickerSample >= 50UL) {
        baseFlicker       = (uint8_t)random(0, 22);
        lastFlickerSample = now;
    }

    // ch 0,2 = top row (Pair A) — swatch primary
    // ch 1,3 = bottom row (Pair B) — swatch accent
    // Active pair pulses smoothly; idle channels shimmer at contrast floor.
    for (uint8_t ch = 0; ch < 4; ch++) {
        bool isPairA         = (ch == 0 || ch == 2);
        int8_t pairIndex     = isPairA ? 0 : 1;
        const uint8_t* color = isPairA ? swatch[swNum].primary : swatch[swNum].accent;
        const uint8_t* base  = swatch[swNum].contrast;

        uint8_t lvl = (activePair == pairIndex) ? brightness : baseFlicker;

        for (uint8_t c = 0; c < 3; c++) {
            shiftRegColors[ch][c] = base[c] + (uint8_t)(((int16_t)(color[c] - base[c]) * lvl) / 255);
        }
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

        // Simple triangle wave for brightness cycling
        float cyclePos = (float)(elapsedTime % cycleDuration) / cycleDuration;
        float brightnessRatio = (cyclePos < 0.5) ? cyclePos * 2 : 2 - (cyclePos * 2);

        // Map to brightness range
        currentBrightness = minBrightness + (maxBrightness - minBrightness) * brightnessRatio;

        // Display white at current brightness on all segments
        // SR segments are suppressed automatically by the brightnessAdjustMode check in sendToRGB
        for (int i = 0; i < 10; i++) { // Display multiple times per step for stability
            for (uint8_t seg = 1; seg < numLEDs; seg++) { sendToRGB(seg, whiteColor); }
            sendToRGB(0, whiteColor);
            delay(stepDuration / 10);
        }
    }

    // Reset mode flag
    brightnessAdjustMode = false;
}
