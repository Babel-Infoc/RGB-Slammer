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
#include "hardware.h"

// Forward declarations
void animationPreview();
void fadeToColor(const ZoneColor (&colors)[2], const int fadeTime);
void showColor(const ZoneColor (&colors)[2], int duration);

// Number of LED zones
const uint8_t numLEDs = 2;

// Select which hardware configuration to use
ConfigType activeConfig = NANOSHARD;

// Define the LED array and button pins according to the active configuration
ledZone led[2];
uint8_t colorBtn;
uint8_t animBtn;

// Values below are loaded from the active hardware configuration in setup()
// Brightness adjustment range
uint8_t minBrightness;
uint8_t maxBrightness;
uint8_t currentBrightness;
uint8_t currentAnim;
// Slow down all animations by this amount (in milliseconds)
uint8_t slowDown;
// LED color tuning: {mA, luminosity} per channel, from the LED datasheet
luminance red;
luminance green;
luminance blue;

const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// Positions of the swatch colors along the gradient (0 = color4, 255 = color0)
const uint8_t posPri = 255;
const uint8_t posAcc = 204;
const uint8_t posMid = 153;
const uint8_t posCon = 102;
const uint8_t posBgd = 51;

const uint8_t white[3] = {255, 255, 255};
const uint8_t black[3] = {0, 0, 0};

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig(activeConfig);

    // Copy all pin from the selected hardware profile
    for (uint8_t zone = 0; zone < numLEDs; zone++) {
        led[zone] = config.leds[zone];
    }
    colorBtn = config.colorButton;
    animBtn = config.animButton;

    // Copy tuning values from the selected hardware profile
    minBrightness = config.minBrightness;
    maxBrightness = config.maxBrightness;
    red = config.red;
    green = config.green;
    blue = config.blue;
    slowDown = config.slowDown;

    // Set up all LED zones
    for (uint8_t zone = 0; zone < 2; zone++) {
        pinMode(led[zone].red, OUTPUT);
        pinMode(led[zone].green, OUTPUT);
        pinMode(led[zone].blue, OUTPUT);
        // Force all channels off (black) immediately - pinMode() alone does not
        // guarantee an initial LOW/HIGH state, and since LOW = LED on for this
        // hardware, an undriven pin can read as fully lit (a bright white flash)
        // until the first real color is written.
        digitalWrite(led[zone].red, HIGH);
        digitalWrite(led[zone].green, HIGH);
        digitalWrite(led[zone].blue, HIGH);
    }
    // Set up the buttons
    pinMode(colorBtn, INPUT_PULLUP);
    pinMode(animBtn, INPUT_PULLUP);

    // Calculate the luminosity modifiers
    calculateLuminance();

    // Try to load saved settings from flash
    if (!loadSettingsFromFlash(&currentSwatch, &currentBrightness, &currentAnim)) {
        // If no valid settings found, set defaults
        currentSwatch = 0;                             // Default swatch
        currentBrightness = maxBrightness;      // Default brightness
        currentAnim = 0;                      // Default animation mode
    }

    // Make sure loaded brightness is still usable on this hardware.
    if (currentBrightness < minBrightness) {
        currentBrightness = minBrightness;
    }
    if (currentBrightness > maxBrightness) {
        currentBrightness = maxBrightness;
    }

    // Guard against an out-of-range swatch index (e.g. stale flash settings
    // from a build that had more swatches). Reading past the swatch array
    // returns garbage colors that render as a white flash.
    if (currentSwatch >= numSwatches) {
        currentSwatch = 0;
    }

    // Show the bootup animation
    bounceBoot();
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
        switch (currentAnim) {
            case 0:
            default:
                glitchLoop();
                break;
            case 1:
                cautionCitizen();
                break;
            case 2:
                slowFade();
                break;
            case 3:
                photomode1();
                break;
            case 4:
                photomode2();
                break;
        }
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// -------------------------------------------------------------------------------------
// MARK: bounceBoot
Animation bounceBoot(){
    uint16_t speed = 40;
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = speed * 3;

          showColor({{posPri, posPri}, {posPri, posPri}}, 10);
        fadeToColor({{posAcc, posAcc}, {posAcc, posAcc}}, speed);
        fadeToColor({{posMid, posMid}, {posMid, posMid}}, speed);
        fadeToColor({{posCon, posCon}, {posCon, posCon}}, speed);
        fadeToColor({{posBgd, posBgd}, {posBgd, posBgd}}, speed);
    }
    showColor({{posBgd, 10}, {posBgd, 10}}, speed);
}

// MARK: testLoop
void testLoop() {
      showColor({{posPri,255},  {posPri,255}}, 10);
    fadeToColor({{posAcc, 255}, {posAcc, 255}}, 1000);
    fadeToColor({{posMid, 255}, {posMid, 255}}, 1000);
    fadeToColor({{posCon, 255}, {posCon, 255}}, 1000);
    fadeToColor({{posBgd, 255}, {posBgd, 255}}, 1000);
    fadeToColor({{posPri, 150}, {posBgd, 150}}, 1000);
    fadeToColor({{posAcc, 150}, {posCon, 150}}, 1000);
    fadeToColor({{posMid, 150}, {posMid, 150}}, 1000);
    fadeToColor({{posCon, 150}, {posAcc, 150}}, 1000);
    fadeToColor({{posBgd, 150}, {posPri, 150}}, 1000);
    showColor({{posPri,255}, {posBgd,255}}, 1000);
    showColor({{posAcc,255}, {posCon,255}}, 1000);
    showColor({{posMid,255}, {posMid,255}}, 1000);
    showColor({{posCon,255}, {posAcc,255}}, 1000);
    showColor({{posBgd,255}, {posPri,255}}, 1000);

    fadeToColor({{posPri,posPri}, {posPri,posPri}}, 1000);
    fadeToColor({{posAcc,posAcc}, {posAcc,posAcc}}, 1000);
    fadeToColor({{posMid,posMid}, {posMid,posMid}}, 1000);
    fadeToColor({{posCon,posCon}, {posCon,posCon}}, 1000);
    fadeToColor({{posBgd,posBgd}, {posBgd,posBgd}}, 1000);

    showColor({{posPri,posPri}, {posPri,posPri}}, 1000);
    showColor({{posAcc,posAcc}, {posAcc,posAcc}}, 1000);
    showColor({{posMid,posMid}, {posMid,posMid}}, 1000);
    showColor({{posCon,posCon}, {posCon,posCon}}, 1000);
    showColor({{posBgd,posBgd}, {posBgd,posBgd}}, 1000);

    delay (200);
    glitch1((random(0, numLEDs)), 700);
    delay (200);
    glitch2(posMid, posCon, 700);
    delay (200);
    glitch3((random(0, numLEDs)), posPri, 20, 3);
    delay (200);
    glitch4(6, 700);
    delay (200);
    glitch5();
    delay (200);
    fakeMorse(65, 210, 400);
    delay (200);
    glitchLoop();
    delay (200);
}

// -------------------------------------------------------------------------------------
// MARK: glitchLoop
// Advanced neon flicker with 3 different animation patterns selected randomly
void glitchLoop() {
    // For <duration> milliseconds, both LED zones will either play a special animation or the normal flicker
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    bool effectTrigger = random(0, 100) < 10;
    checkButtons();
    // Check if swatch preview should interrupt this animation
    if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

    if (effectTrigger) {
        // Apply a special effect
        uint8_t flickerZone = random(0, numLEDs);
        // Pick a random glitch effect
        switch (random(0, 6)) {
            case 0:
                glitch1(flickerZone, 700);
                break;
            case 1:
                glitch2(posMid, posCon, 700);
                break;
            case 2:
                glitch3(flickerZone, posPri, 20, 3);
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
        // Normal flicker on both zones
        flicker(0, 10, posPri, posAcc);
        flicker(1, 10, posCon, posBgd);
        currentTime = millis();
    }
}

// -------------------------------------------------------------------------------------
// MARK: cautionCitizen
void cautionCitizen() {
    const unsigned long STROBE_INTERVAL_MS = 1500UL;
    const uint8_t       STROBE_REPS        = 3;
    const uint8_t       FLASH_ON_MS        = 20;
    const uint8_t       FLASH_OFF_MS       = 20;

    static unsigned long lastStrobeTime = 0;
    if (lastStrobeTime == 0) lastStrobeTime = millis();

    while (true) {
        checkButtons();
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        if (millis() - lastStrobeTime >= STROBE_INTERVAL_MS) {
            // Strobe burst: showColor drives SR directly
            for (uint8_t i = 0; i < STROBE_REPS; i++) {
                checkButtons();
                if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) break;
                showColor({{posAcc,255}, {posPri,255}}, FLASH_ON_MS);
                showColor({{posCon,100}, {posCon,100}}, FLASH_OFF_MS);
            }
            lastStrobeTime = millis();
        } else {
            // Idle: flicker all zones independently
            flicker(0, 20, 170, 180); // Core zone flicker separately from the others
            for (uint8_t seg = 1; seg < numLEDs; seg++) {
                flicker(seg, 20, 140, 150);
            }
        }
    }
}

// MARK: slowFade
// Breathing effect: fades through swatch colors (color4→color0) over 1s, then reverses over 3s.
void slowFade() {
    const uint16_t FADE_UP_TIME   = 150;  // 1000ms / 4 transitions = 250ms each
    const uint16_t FADE_DOWN_TIME = 750;  // 3000ms / 4 transitions = 750ms each

    while (true) {
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        // Fade up: color4 → color4 → color2 → color1 → color0 (1 second total)
        fadeToColor({{posCon, posCon}, {posCon, posCon}}, FADE_UP_TIME);
        fadeToColor({{posMid, posMid}, {posMid, posMid}}, FADE_UP_TIME);
        fadeToColor({{posAcc, posAcc}, {posAcc, posAcc}}, FADE_UP_TIME);
        fadeToColor({{posPri, posPri}, {posPri, posPri}}, FADE_UP_TIME);

        // Fade down: color0 → color1 → color2 → color4 → color4
        fadeToColor({{posAcc, posAcc}, {posAcc, posAcc}}, FADE_DOWN_TIME);
        fadeToColor({{posMid, posMid}, {posMid, posMid}}, FADE_DOWN_TIME);
        fadeToColor({{posCon, posCon}, {posCon, posCon}}, FADE_DOWN_TIME);
        fadeToColor({{posBgd, posBgd}, {posBgd, posBgd}}, FADE_DOWN_TIME);
    }
}

// MARK: photomode1
void photomode1() {
    while (true) {
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;
        // showColor handles all zones internally: color0→ROLE_GPIO, color1→ROLE_SR
        showColor({{posPri, 255}, {posPri, 255}}, 100);
    }
}

// MARK: photomode2
void photomode2() {
    while (true) {
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;
        // showColor handles all zones internally: color0→ROLE_GPIO, color1→ROLE_SR
        showColor({{posPri, 255}, {posBgd, 255}}, 100);
    }
}

// -------------------------------------------------------------------------------------
// MARK: fadeToColor
void fadeToColor(const uint8_t zone, Colour targetColor, const uint8_t alpha){
    Colour targetColor;
    Colour startColor;
    Colour output;

    // Copy handoverColor to startColor
    startColor.r = handoverColor

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        float fadeRatio = (float)(millis() - startTime) / fadeTime;
            targetColor.r = startColor.r + (color1[pin] - startColor.r) * fadeRatio;
            targetColor.g = startColor.g + (color1[pin] - startColor.g) * fadeRatio;
            targetColor.b = startColor.b + (color1[pin] - startColor.b) * fadeRatio;
        calcRGB(zone, output, );
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
// Holds a single zone at a gradient position with the given brightness modifier
void showColor(const ZoneColor (&colors)[2], int duration){
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        checkButtons();
        // Check if swatch preview should interrupt this animation.
        // Preview functions set previewMode=true to avoid self-abort.
        if (!previewMode && (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode)) return;

        for (uint8_t zone = 0; zone < numLEDs; zone++) {
            calcRGB(zone, colors[zone].grad, colors[zone].alpha);
        }
        writeRGB();
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
void flicker(const uint8_t zone, const uint8_t chance, const uint8_t min, const uint8_t max){
    uint8_t range = random(min, max);
    calcRGB(zone, range, 255);
}

// -------------------------------------------------------------------------------------
// MARK: glitch1
void glitch1(const uint8_t zone, int duration){
    uint8_t otherZone = 0;
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        checkButtons();
        // Check if preview or brightness adjust mode should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) {
            return; // Immediately exit to allow the preview/mode to play
        }

        if (zone == 0) {
            showColor({{posCon,50}, {posAcc,50}}, 50);
            showColor({{posCon,50}, {posBgd,50}}, 50);
        } else {
            showColor({{posAcc,50}, {posCon,50}}, 50);
            showColor({{posBgd,50}, {posCon,50}}, 50);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch2
void glitch2(uint8_t pos1, uint8_t pos2, int duration) {
    // Part 1: Rapidly flash between the two gradient positions
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        rapidPulse(pos1, pos2, 50);
    }

    // Part 2: Rapidly fade through all swatch colors from color4 to color0
    uint8_t fadeTime = 50; // Quick fade time between colors
    // Fade through the colors in sequence: color4 → color4 → color2 → color1 → color0
    fadeToColor({{posBgd,posBgd}, {posBgd,posBgd}}, fadeTime);
    fadeToColor({{posCon,posCon}, {posCon,posCon}}, fadeTime);
    fadeToColor({{posMid,posMid}, {posMid,posMid}}, fadeTime);
    fadeToColor({{posAcc,posAcc}, {posAcc,posAcc}}, fadeTime);
    fadeToColor({{posPri,posPri}, {posPri,posPri}}, fadeTime);
}

// -------------------------------------------------------------------------------------
// MARK: glitch3
// Freeze all other zones at their handoverPos and brightness, and flash
// only the selected zone between startPos and pos2 for `reps` repetitions.
void glitch3(uint8_t zone, uint8_t pos2, int duration, uint8_t reps) {
    uint8_t startPos = handoverPos[zone];
    uint8_t startBright = handoverBright[zone];

    for (uint8_t rep = 0; rep < reps; rep++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        // Snapshot every zone frozen at its current handover state; only
        // the selected zone's position gets overridden below.
        ZoneColor colors[2];
        for (uint8_t s = 0; s < numLEDs; s++) {
            colors[s].pos = handoverPos[s];
            colors[s].brightness = handoverBright[s];
        }

        colors[zone].grad = startPos;
        colors[zone].alpha = startBright;
        showColor(colors, duration);

        colors[zone].grad = pos2;
        showColor(colors, duration);
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch4
void glitch4(uint8_t reps, int duration) {
    unsigned long start = millis();
    while (millis() - start < duration) {
        checkButtons();
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        for (uint8_t zone = 0; zone < numLEDs; zone++) {
            uint8_t pos = random(1, 255);
            for (uint8_t i = 0; i < reps; i++) {
                calcGrad(zone, posPri, 255);
                calcGrad(zone, posCon, 255);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch5
void glitch5(){
    // Use one of the waveform arrays from waveforms.cpp
    uint8_t waveformIndex = random(0, 2); // Choose between the two available waveforms

    // First play through the waveform once
    for (uint8_t i = 0; i < 32; i++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        // Show the color at this position in the gradient on both LEDs briefly
        showColor({{waveform[waveformIndex].waveform[i], 255}, {waveform[waveformIndex].waveform[i], 255}}, 50);

        // Brief black flash every few steps for a glitchy effect
        if (i % 4 == 0) {
            showColor({{posBgd, 0}, {posBgd, 0}}, 10);
        }
    }

    // Then do some rapid random jumps between waveform positions
    for (uint8_t i = 0; i < 8; i++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        uint8_t randomPos = random(0, 32);
        showColor({{waveform[waveformIndex].waveform[randomPos], 255}, {waveform[waveformIndex].waveform[randomPos], 255}}, 30);

        // Brief flashes to black between jumps
        showColor({{posBgd, 10}, {posBgd, 10}}, 15);
    }

    // End with a final dramatic fade up
    fadeToColor({{posPri, posPri}, {posBgd, posBgd}}, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t pos1, uint8_t pos2, int speed){
    showColor({{pos1, 255}, {pos1, 255}}, speed);
    showColor({{pos2, 255}, {pos2, 255}}, speed);
}

// -------------------------------------------------------------------------------------
// MARK: fakeMorse
void fakeMorse(uint8_t pos1, uint8_t pos2, int duration) {
    int interval = 50; // Change every 100ms

    unsigned long start = millis();
    while (millis() - start < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) return;

        int selection = random(0, 3);
        // Set LED colors based on selection
        if (selection == 0) {
            showColor({{pos1, 255}, {pos2, 255}}, interval);
        } else if (selection == 1) {
            // Second zone gets pos2, first gets pos1
            showColor({{pos2, 255}, {pos1, 255}}, interval);
        } else {
            // Neither selected, both get pos1
            showColor({{pos1, 255}, {pos1, 255}}, interval);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: swatchPreview
void swatchPreview() {
    const int fadeUpDuration = 50; // 0.2 seconds fade up
    const int fadeDownDuration = 400; // 0.6 seconds fade down

    bool previousPreviewMode = previewMode;
    previewMode = true;

    // Store original brightness and temporarily increase it
    uint8_t originalBrightness = currentBrightness;
    currentBrightness = maxBrightness;

    // Phase 1: Fade up quickly
    showColor({{posBgd, posBgd}, {posBgd, posBgd}}, 10);
    fadeToColor({{posPri, posPri}, {posPri, posPri}}, fadeUpDuration);

    // Phase 2: Fade down slowly
    fadeToColor({{posBgd, 255}, {posBgd, 255}}, fadeDownDuration);
    fadeToColor({{posBgd, posBgd}, {posBgd, posBgd}}, 50);

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Reset the flag
    swatchPreviewActive = false;
    previewMode = previousPreviewMode;
}

// -------------------------------------------------------------------------------------
// MARK: animationPreview
void animationPreview() {
    const int flashDuration = 30; // Quick flash duration in ms
    const int numFlashes = 3; // Number of flashes to indicate mode change

    bool previousPreviewMode = previewMode;
    previewMode = true;

    // Store original brightness and temporarily increase it
    uint8_t originalBrightness = currentBrightness;
    currentBrightness = maxBrightness;

    // Flash the color0 color quickly to indicate animation mode change
    for (int i = 0; i < numFlashes; i++) {
        showColor({{posPri, posPri}, {posPri, posPri}}, flashDuration);
        showColor({{posBgd, posBgd}, {posBgd, posBgd}}, flashDuration);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Save the selected animation mode now that the preview has completed
    saveSettingsToFlash(currentSwatch, currentBrightness, currentAnim);

    // Reset the flag
    animationPreviewActive = false;
    previewMode = previousPreviewMode;
}

// -------------------------------------------------------------------------------------
// MARK: brightnessAdjustmentMode
void brightnessAdjustmentMode() {
    const int cycleDuration = 4000; // 4 seconds total
    const int stepDuration = 100; // Update every 100ms

    unsigned long modeStartTime = millis();

    while (brightnessAdjustMode && digitalRead(colorBtn) == LOW) {
        unsigned long elapsedTime = millis() - modeStartTime;

        // Simple triangle wave for brightness cycling (using fixed-point math)
        // cyclePos is 0-256 where 256 = full cycle
        uint16_t cyclePos = ((uint32_t)(elapsedTime % cycleDuration) * 256) / cycleDuration;
        // brightnessRatio is 0-256 where 256 = full brightness
        uint16_t brightnessRatio = (cyclePos < 128) ? (cyclePos * 2) : (512 - cyclePos * 2);

        // Map to brightness range (0-255)
        currentBrightness = minBrightness + (uint8_t)(((uint16_t)(maxBrightness - minBrightness) * brightnessRatio) >> 8);

        // Display white at current brightness on both zones
        // White is not on the swatch gradient, so use the raw RGB path
        for (int i = 0; i < 10; i++) { // Display multiple times per step for stability
            calcRawRGB(0, white, currentBrightness);
            calcRawRGB(1, white, currentBrightness);
            delay(stepDuration / 10);
        }
    }

    // Reset mode flag
    brightnessAdjustMode = false;
}
