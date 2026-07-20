// RGB Slammer
// Written by Tully Jagoe 2026

// This script is best edited in VSCode for color token selection in swatches.cpp

#include <Arduino.h>
#include "swatches.h"
#include "waveforms.h"
#include "flashStorage.h"
#include "hardware.h"
#include "types.h"

// Select from the active hardware configuration options in hardware.h
// That file stores all platform specifc configuration options

// Define the LED array and button pins according to the active configuration
uint8_t colorBtn;
uint8_t animBtn;

// Shared LED zone state used by the sketch and rgbProcessor.cpp
zoneConfig led[5];

// Shift register pin configuration
shiftRegPins shiftReg;
uint8_t srLEDs = 0;

// Brightness 0-255 (initialised from active hardware config at boot; overridden by flash if saved)
uint8_t currentBrightness;

// Brightness adjustment mode limits
uint8_t minBrightness;
uint8_t maxBrightness;
const uint16_t brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

uint8_t level0 = 255;
uint8_t level1 = 204;
uint8_t level2 = 153;
uint8_t level3 = 102;
uint8_t level4 = 51;

uint8_t numZones = 0;
extern zoneConfig led[5];

extern void (*srUpdateCallback)(); // defined in rgbProcessor.cpp

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    /*
        NANOFRAME
        BREACH_KEY
        AURORA_GLASYA
        NANOSHARD
        AG_ECHO_FRAME
    */
    hardwareConfig config = NANOSHARD;

    // Initialise the configured zones
    for (uint8_t zone = 0; zone < 5; zone++) {
        if (config.zone[zone].type != NOTUSED) {
            led[zone].r        = config.zone[zone].r;
            led[zone].g        = config.zone[zone].g;
            led[zone].b        = config.zone[zone].b;
            led[zone].type     = config.zone[zone].type;
            led[zone].role     = config.zone[zone].role;
            led[zone].scale    = config.zone[zone].scale;
            if (config.zone[zone].type == SHIFTREG) {
                led[zone].srChannel    = config.zone[zone].srChannel;
            }
            numZones++;
        }
    }

    colorBtn = config.colorButton;
    animBtn  = config.animButton;

    // Copy shift register configuration
    shiftReg = config.shiftReg;

    // Set up all LED zones (GPIOLED only — SR zones have no pins to configure)
    for (uint8_t zone = 0; zone < numZones; zone++) {
        if (led[zone].type == GPIOLED) {
            pinMode(led[zone].r, OUTPUT);
            pinMode(led[zone].g, OUTPUT);
            pinMode(led[zone].b, OUTPUT);
        }
    }

    /*
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
    */
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
    //bounceBoot(40);
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
                testLoop();
                //glitchLoop();
                break;
            case 1:
                //cautionCitizen();
                break;
            case 2:
                //slowFade();
                break;
            case 3:
                //photomode1();
                break;
            case 4:
                //photomode2();
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
        fadeToColor(swatch[swNum].primary,      level0,    swatch[swNum].background,   level4,    speed);
        fadeToColor(swatch[swNum].accent,       level1,    swatch[swNum].primary,      level0,    speed);
        fadeToColor(swatch[swNum].midtone,      level2,    swatch[swNum].accent,       level1,    speed);
        fadeToColor(swatch[swNum].contrast,     level3,    swatch[swNum].midtone,      level2,    speed);
        fadeToColor(swatch[swNum].background,   level4,    swatch[swNum].contrast,     level3,    speed);
        fadeToColor(swatch[swNum].background,   level4,    swatch[swNum].background,   level4,    speed);
    }
    showColor(swatch[0].background, 0, swatch[0].background, 0, speed*5);
}

// MARK: testLoop
void testLoop() {
    fadeToColor(swatch[swNum].primary,      level2,    swatch[swNum].primary,      level2,    200);
    fadeToColor(swatch[swNum].accent,       level2,    swatch[swNum].accent,       level2,    200);
    fadeToColor(swatch[swNum].midtone,      level2,    swatch[swNum].midtone,      level2,    200);
    fadeToColor(swatch[swNum].contrast,     level2,    swatch[swNum].contrast,     level2,    200);
    fadeToColor(swatch[swNum].background,   level2,    swatch[swNum].background,   level2,    200);

    //fadeToColor(swatch[swNum].primary,      level0,    swatch[swNum].primary,      level0,    1000);
    //fadeToColor(swatch[swNum].accent,       level0,    swatch[swNum].accent,       level0,    1000);
    //fadeToColor(swatch[swNum].midtone,      level0,    swatch[swNum].midtone,      level0,    1000);
    //fadeToColor(swatch[swNum].contrast,     level0,    swatch[swNum].contrast,     level0,    1000);
    //fadeToColor(swatch[swNum].background,   level0,    swatch[swNum].background,   level0,    1000);
    //fadeToColor(swatch[swNum].background,   level0,    swatch[swNum].background,   level0,    1000);

    showColor(swatch[swNum].primary,        level2,    swatch[swNum].primary,      level2,    200);
    showColor(swatch[swNum].accent,         level2,    swatch[swNum].accent,       level2,    200);
    showColor(swatch[swNum].midtone,        level2,    swatch[swNum].midtone,      level2,    200);
    showColor(swatch[swNum].contrast,       level2,    swatch[swNum].contrast,     level2,    200);
    showColor(swatch[swNum].background,     level2,    swatch[swNum].background,   level2,    200);
}

// -------------------------------------------------------------------------------------
// MARK: glitchLoop
// Advanced neon flicker with 3 different animation patterns selected randomly
void glitchLoop() {
    const uint8_t flickerChance = 15;
    const uint8_t effectChance = 15;
    const int duration = 1000;

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
            uint8_t randomZone = random(0, numZones);
            // Pick a random glitch effect
            switch (random(0, 5)) {
                case 0:
                    glitch1(randomZone, 700);
                    break;
                case 1:
                    glitch2();
                    break;
                case 2:
                    glitch3(randomZone);
                    break;
                case 3:
                    glitch4(6, 700);
                    break;
                case 4:
                    glitch5();
                    break;
            }
            currentTime = millis();
        } else {
            // Normal flicker on both segments
            for (uint8_t zone = 0; zone < numZones; zone++) {
                flicker(zone, flickerChance, 200, 255, zone);
            }
            currentTime = millis();
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: cautionCitizen
// Idle: eyeRipple on SR eye pods (direct call), flicker on core LED.
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
    for (uint8_t s = 0; s < numZones; s++) { if (led[s].type == GPIOLED) { coreSeg = s; break; } }

    srUpdateCallback = nullptr;

    while (true) {
        if (buttonInterruptCheck()) return;

        if (millis() - lastStrobeTime >= STROBE_INTERVAL_MS) {
            // Strobe burst: showColor drives SR directly
            for (uint8_t i = 0; i < STROBE_REPS; i++) {
                if (buttonInterruptCheck()) break;
                showColor(swatch[swNum].accent,   255,    swatch[swNum].primary,   255,    FLASH_ON_MS);
                showColor(swatch[swNum].contrast, 100,    swatch[swNum].contrast,  100,    FLASH_OFF_MS);
            }
            lastStrobeTime = millis();
        } else {
            // Idle: flicker all zones independently
            for (uint8_t zone = 0; zone < numZones; zone++) {
                if (led[zone].type == SHIFTREG) {
                    flicker(zone, 20, 140, 150, 4);
                }
            }
            flicker(coreSeg, 20, 170, 180, 0);
        }
    }
}

// MARK: photomode1
void photomode1() {
    srUpdateCallback = nullptr;
    while (true) {
        if (buttonInterruptCheck()) return;
        // showColor handles all zones internally: color1→GPIOLED, color2→SHIFTREG
        showColor(swatch[swNum].primary, 255, swatch[swNum].primary, 255, 10);
    }
}

// MARK: photomode2
void photomode2() {
    srUpdateCallback = nullptr;
    while (true) {
        if (buttonInterruptCheck()) return;
        // showColor handles all zones internally: color1→GPIOLED, color2→SHIFTREG
        showColor(swatch[swNum].primary, 255, swatch[swNum].background, 255, 10);
    }
}

// MARK: slowFade
// Breathing effect: fades through swatch colors (background→primary) over 1s, then reverses over 3s.
void slowFade() {
    srUpdateCallback = nullptr;
    const uint16_t FADE_UP_TIME   = 250;  // 1000ms / 4 transitions = 250ms each
    const uint16_t FADE_DOWN_TIME = 750;  // 3000ms / 4 transitions = 750ms each

    while (true) {
        if (buttonInterruptCheck()) return;

        // Fade up: background → contrast → midtone → accent → primary (1 second total)
        fadeToColor(swatch[swNum].contrast,   170, swatch[swNum].contrast,   170, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].midtone,    190, swatch[swNum].midtone,    190, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].accent,     200, swatch[swNum].accent,     200, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].primary,    255, swatch[swNum].primary,    255, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        // Fade down: primary → accent → midtone → contrast → background (3 seconds total)
        fadeToColor(swatch[swNum].accent,     200, swatch[swNum].accent,     200, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].midtone,    190, swatch[swNum].midtone,    190, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].contrast,   170, swatch[swNum].contrast,   170, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].background, 150, swatch[swNum].background, 150, FADE_DOWN_TIME);
    }
}

// -------------------------------------------------------------------------------------
// MARK: fadeToColor
// color1 → GPIOLED zones, color2 → SHIFTREG zones.
// If srUpdateCallback is set, it runs during the GPIOLED flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive SHIFTREG directly with color2.
void fadeToColor(const uint8_t color1[3], uint8_t brightness1, const uint8_t color2[3], uint8_t brightness2, const int fadeTime){
    uint8_t startColor[2][3];
    uint8_t targetColor[2][3];
    uint8_t outputColor[2][3];
    uint8_t numRoles = 2;

    // Copy handoverColor to startColor for all zones
    for (uint8_t role = 0; role < numRoles; role++) {
        for (uint8_t i = 0; i < 3; i++) {
            startColor[role][i] = handoverColor[role][i];
        }
    }

    // Calculate the exact target according to brightness multiplier
    for (int i = 0; i < 3; i++) {
        targetColor[0][i] = (color1[i] * brightness1) / 255 ;
        targetColor[1][i] = (color2[i] * brightness2) / 255 ;
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        // Check if any preview or brightness mode should interrupt this animation
        if (buttonInterruptCheck()) return;

        uint8_t fadeRatio = (uint8_t)(((unsigned long)(millis() - startTime) * 255UL) / fadeTime);
        //uint32_t fadeRatio = (unsigned)(millis() - startTime) / fadeTime;
        for (uint8_t zone = 0; zone < numZones; zone++) {
            if (led[zone].role == CORELED) {
                for (int i = 0; i < 3; i++) {
                    outputColor[zone][i] = startColor[zone][i] + (targetColor[0][i] - startColor[zone][i]) * fadeRatio;
                }
                sendToRGB(zone, outputColor[1]);
            }
            if (led[zone].role == ACCTLED) {
                for (int i = 0; i < 3; i++) {
                    outputColor[zone][i] = startColor[zone][i] + (targetColor[1][i] - startColor[zone][i]) * fadeRatio;
                }
                sendToRGB(zone, outputColor[2]);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
// color1 → GPIOLED zones, color2 → SHIFTREG zones.
// If srUpdateCallback is set, it runs during the GPIOLED flush and overrides color2 for SR.
// Set srUpdateCallback = nullptr before calling to drive SHIFTREG directly with color2.
void showColor(const uint8_t color1[3], uint8_t brightness1, const uint8_t color2[3], uint8_t brightness2, int duration){
    uint8_t out1[3], out2[3];
    for (uint8_t c = 0; c < 3; c++) {
        out1[c] = (uint8_t)(((uint16_t)color1[c] * brightness1) >> 8);
        out2[c] = (uint8_t)(((uint16_t)color2[c] * brightness2) >> 8);
    }
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        // Check if any preview or brightness mode should interrupt this animation
        if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) {
            return;
        }
        // SR-first: buffer SHIFTREG color2, then GPIOLED flush sends both.
        // If srUpdateCallback is set it fires during the GPIOLED flush and overrides the buffer.
        for (uint8_t zone = 0; zone < numZones; zone++) {
            if (led[zone].role == CORELED) { sendToRGB(zone, out1); break; }
            if (led[zone].role == ACCTLED) { sendToRGB(zone, out2); break; }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
// Flickers a zone by scaling a swatch color by a random brightness in [min, max].
// swatchIndex selects which swatch color to use:
//   0=primary  1=accent  2=midtone  3=contrast  4=background  5=random
// If pin is an SR zone, suppresses srUpdateCallback and triggers a GPIOLED flush
// so the buffered SR color is output immediately.
void flicker(const uint8_t zone, const uint8_t chance, const uint8_t min, const uint8_t max, const uint8_t swatchIndex){
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
    sendToRGB(zone, outputColor);
    // SR zones buffer only — need a GPIOLED flush to output. Suppress callback so
    // hand-written SR color is not overwritten, then flush via the GPIOLED zone.
    /*
    if (zone < numZones && led[zone].type == SHIFTREG) {
        void (*saved)() = srUpdateCallback;
        srUpdateCallback = nullptr;
        for (uint8_t zone = 0; zone < numZones; zone++) {
            if (led[zone].type == SHIFTREG) { sendToRGB(zone, handoverColor[zone]); break; }
        }
        srUpdateCallback = saved;
    }
    */
}

// -------------------------------------------------------------------------------------
// MARK: glitch1
void glitch1(const uint8_t zone, int duration){
    uint8_t otherSegment = 0;
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        if (buttonInterruptCheck()) return;

        if (led[zone].type == CORELED) {
            showColor(swatch[swNum].contrast,   level3, swatch[swNum].accent,       level1, flickerTime);
            showColor(swatch[swNum].contrast,   level3, swatch[swNum].background,   level4, flickerTime);
        } else {
            showColor(swatch[swNum].accent,     level1, swatch[swNum].contrast,     level3, flickerTime);
            showColor(swatch[swNum].background, level4, swatch[swNum].contrast,     level3, flickerTime);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch2
void glitch2(){
    uint16_t duration = 700;
    // Part 1: Rapidly flash between black and background for 1 second
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        if (buttonInterruptCheck()) return;
        rapidPulse(swatch[swNum].midtone, swatch[swNum].contrast, level2, 50);
    }

    // Part 2: Rapidly fade through all swatch colors from background to primary
    uint8_t fadeTime = duration/5; // Quick fade time between colors
    // Fade through the colors in sequence: background → contrast → midtone → accent → primary
    fadeToColor(swatch[swNum].background, level3, swatch[swNum].background, level4, fadeTime);
    fadeToColor(swatch[swNum].contrast,   level3, swatch[swNum].background, level4, fadeTime);
    fadeToColor(swatch[swNum].midtone,    level2, swatch[swNum].background, level4, fadeTime);
    fadeToColor(swatch[swNum].accent,     level1, swatch[swNum].background, level4, fadeTime);
    fadeToColor(swatch[swNum].primary,    level0, swatch[swNum].background, level4, fadeTime);
}

// -------------------------------------------------------------------------------------
// MARK: glitch3
void glitch3(uint8_t assignedZone) {
    uint8_t duration = 20;
    uint8_t reps = 3;
    uint8_t startColor[numZones][3];

    // Copy handoverColor to startColor for all zones
    for (uint8_t zone = 0; zone < numZones; zone++) {
        for (uint8_t pin = 0; pin < 3; pin++) {
            startColor[zone][pin] = handoverColor[zone][pin];
        }
    }

    // Hold all other zones at their handoverColor, and flash chosen zone between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        if (buttonInterruptCheck()) return;

        for (uint8_t zone = 0; zone < numZones; zone++) {
            if (led[zone].role = assignedZone ) {
                showColor(startColor[zone],               level0,   handoverColor[assignedZone],         level1, duration);
                showColor(swatch[swNum].primary,    level0,   handoverColor[assignedZone],         level1, duration);
            } else {
                showColor(handoverColor[zone],         level0,   startColor[zone],               level1, duration);
                showColor(handoverColor[zone],         level0,   swatch[swNum].primary,    level1, duration);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch4
void glitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();
    while (millis() - start < duration) {
        if (buttonInterruptCheck()) return;

        for (uint8_t segment = 0; segment < numZones; segment++) {
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
        if (buttonInterruptCheck()) return;

        // Get color at this position in the gradient
        gradientPosition(waveform[waveformIndex].waveform[i], outputColor);

        // Show this color on both LEDs briefly
        showColor(outputColor,   level0,   outputColor, level0, 50);

        // Brief black flash every few steps for a glitchy effect
        if (i % 4 == 0) {
            showColor(swatch[swNum].background,   0,   swatch[swNum].background, 0, 50);
        }
    }

    // Then do some rapid random jumps between waveform positions
    for (uint8_t i = 0; i < 8; i++) {
        if (buttonInterruptCheck()) return;

        uint8_t randomPos = random(0, 32);
        gradientPosition(waveform[waveformIndex].waveform[randomPos], outputColor);
        showColor(outputColor,   waveform[waveformIndex].waveform[randomPos],   outputColor, waveform[waveformIndex].waveform[randomPos], 50);

        // Brief flashes to black between jumps
        showColor(swatch[swNum].background,   0,   swatch[swNum].background, 0, 15);
    }

    // End with a final dramatic fade to black
    fadeToColor(swatch[swNum].primary, level0, swatch[swNum].background, 0, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(const uint8_t color1[3], const uint8_t color2[3], uint8_t brightness, int speed){
    showColor(color1, brightness, color2, brightness, speed); // core = color1, eye = color2
    showColor(color2, brightness, color2, brightness, speed); // core = color2, eye = color2
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
    currentBrightness = maxBrightness;

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 20); // Blank all LEDs briefly to better mark the swatch pulse start

    // Phase 1: Fade UP quickly from dark to bright over 0.2 seconds
    for (uint8_t i = 0; i < fadeUpSteps; i++) {
        uint8_t gradientPos = (i * 255) / (fadeUpSteps - 1);
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t zone = 1; zone < numZones; zone++) { sendToRGB(zone, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (uint8_t i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t zone = 1; zone < numZones; zone++) { sendToRGB(zone, outputColor); }
        sendToRGB(0, outputColor);

        delay(fadeDownDuration / fadeDownSteps);
    }

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 20); // Blank all LEDs briefly to better mark the swatch pulse end

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
    currentBrightness = maxBrightness;

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 20); // Blank all LEDs briefly to better mark the animation change start

    // Flash the primary color quickly to indicate animation mode change
    for (uint8_t i = 0; i < numFlashes; i++) {
        // Bright flash
        for (uint8_t zone = 1; zone < numZones; zone++) { sendToRGB(zone, swatch[swNum].primary); }
        sendToRGB(0, swatch[swNum].primary);
        delay(flashDuration);

        // Dark flash
        for (uint8_t zone = 1; zone < numZones; zone++) { sendToRGB(zone, swatch[swNum].background); }
        sendToRGB(0, swatch[swNum].background);
        delay(flashDuration);
    }

    showColor(swatch[swNum].background, 0, swatch[swNum].background, 0, 20); // Blank all LEDs briefly to better mark the animation change end

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Save deferred from button handler — flash erase here is safe outside the PWM loop
    saveSettingsToFlash(swNum, currentBrightness, animationMode);

    // Reset the flag
    animationPreviewActive = false;
}

bool buttonInterruptCheck() {
    if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) {
        return true; // Interrupt detected
    }
    return false; // No interrupt
}
/*
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
// Blends color against base at the given level (0=base, 255=full color) into handoverColor[ch].
static void setSRChannel(uint8_t ch, const uint8_t* color, const uint8_t* base, uint8_t level) {
    for (uint8_t c = 0; c < 3; c++)
        handoverColor[ch][c] = base[c] + (uint8_t)(((int16_t)(color[c] - base[c]) * level) >> 8);
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
// Writes to handoverColor[] via setSRChannel; call this then sendToRGB(0, ...) to flush.
void eyeDoublePulse() {
    const bool mirrorMode = (animationMode == 2);

    const uint8_t flickerMin = 140;  // Minimum background flicker brightness (0-255)
    const uint8_t flickerMax = 160;  // Maximum background flicker brightness (0-255)
    const uint8_t pulsePeak  = 255;  // Peak brightness at the top of the pulse envelope (0-255)

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
                uint8_t bri = flickerMax + (uint8_t)(((uint16_t)(pulsePeak - flickerMax) * lvl) >> 8);
                for (uint8_t c = 0; c < 3; c++) handoverColor[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * bri) >> 8);
            } else {
                for (uint8_t c = 0; c < 3; c++)
                    handoverColor[ch][c] = (uint8_t)(((uint16_t)swatch[swNum].background[c] * flickerBri) >> 8);
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
                uint8_t bri = flickerMax + (uint8_t)(((uint16_t)(pulsePeak - flickerMax) * lvl) >> 8);
                for (uint8_t c = 0; c < 3; c++) handoverColor[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * bri) >> 8);
            } else {
                for (uint8_t c = 0; c < 3; c++)
                    handoverColor[ch][c] = (uint8_t)(((uint16_t)swatch[swNum].background[c] * flickerBri) >> 8);
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
        for (uint8_t c = 0; c < 3; c++) handoverColor[ch][c] = (uint8_t)(((uint16_t)gradColor[c] * brightness) >> 8);
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
*/

// -------------------------------------------------------------------------------------
// MARK: brightnessAdjustmentMode
void brightnessAdjustmentMode() {
    // Triangle wave: min → max → min over CYCLE_MS. One sendToRGB(0,...) call takes
    // Triangle wave: min → max → min over CYCLE_MS. One sendToRGB(0,...) call now takes
    // ~250 ms (slowDown=1 × 256 PWM steps), so a 4 s cycle gives ~16 brightness updates —
    // smooth enough to see the fade. The inner 10-iteration loop from the old version
    // caused only 3–4 updates per cycle, making the fade invisible.
    const uint32_t CYCLE_MS = 4000UL;
    uint8_t whiteColor[3] = {255, 255, 255};
    unsigned long modeStartTime = millis();

    void (*savedCallback)() = srUpdateCallback;
    srUpdateCallback = nullptr;

    // With 256-step PWM resolution, we can now use the full 0-255 brightness range without
    // clamping issues. Previously with 100 steps, high zoneOutputScale values would push
    // the output above the PWM ceiling, but 256 steps provides enough headroom.
    uint8_t originalBrightness = currentBrightness;  // Preserve original brightness
    uint8_t adjustedBrightness = currentBrightness; // will hold the user's chosen brightness
    bool brightnessChanged = false; // Track whether user has adjusted brightness

    while (brightnessAdjustMode) {
        // Direct pin read — bypasses checkButtons debounce for immediate exit on release
        if (digitalRead(colorBtn) != LOW) { brightnessAdjustMode = false; break; }
        uint32_t phase = (millis() - modeStartTime) % CYCLE_MS;
        uint8_t  ratio = (phase < CYCLE_MS / 2)
                       ? (uint8_t)(phase * 255UL / (CYCLE_MS / 2))
                       : (uint8_t)((CYCLE_MS - phase) * 255UL / (CYCLE_MS / 2));

        // Actual brightness: mapped within the hardware min/max limits — this is what gets saved.
        uint8_t newBrightness = minBrightness + (uint8_t)(((uint16_t)(maxBrightness - minBrightness) * ratio) >> 8);

        // Track if user has held long enough to see a significant change (>10% of range)
        if (abs((int16_t)newBrightness - (int16_t)originalBrightness) > (maxBrightness - minBrightness) / 10) {
            brightnessChanged = true;
        }

        adjustedBrightness = newBrightness;
        // Display brightness: with 256 PWM steps, use the ratio directly for preview
        currentBrightness  = ratio;

        // Write white to all SR channels then flush via GPIOLED zone 0.
        // SR sendToRGB calls buffer {255,255,255}; the GPIOLED flush outputs everything.
        for (uint8_t zone = 1; zone < numZones; zone++) { sendToRGB(zone, whiteColor); }
        sendToRGB(0, whiteColor);
    }

    // Restore brightness: use adjusted value if user made a significant change, otherwise restore original
    currentBrightness = brightnessChanged ? adjustedBrightness : originalBrightness;

    srUpdateCallback = savedCallback;
    // Save deferred from button handler — flash erase here is safe outside the PWM loop
    saveSettingsToFlash(swNum, currentBrightness, animationMode);

    // Reset mode flag
    brightnessAdjustMode = false;
}
