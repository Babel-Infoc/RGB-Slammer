/*
RGB Slammer
Written by Tully Jagoe 2025
MIT License

This script is best edited in VSCode for color token selection in swatches.cpp

MOTOR CONTROL:
- Motor uses two digital pins (A & B) configured per hardware in pinouts.h
- Motor direction is controlled by pin polarity (no H-bridge required)
- Available functions:
  * motorForward()  - Rotate motor forward (PinA=HIGH, PinB=LOW)
  * motorReverse()  - Rotate motor reverse (PinA=LOW, PinB=HIGH)
  * motorOff()      - Stop motor (both pins LOW)
  * motorStop()     - Alias for motorOff()
- Motor is integrated into glitch effects for synchronized animations
- WARNING: Never set both pins HIGH simultaneously (creates short circuit)
*/

#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "waveforms.h"
#include "flashStorage.h"
#include "pinouts.h"

// Forward declarations
void animationPreview();

// Number of LED segments
const uint8_t numLEDs = 3;

// Select which hardware configuration to use
ConfigType activeConfig = NANOFRAME;

// Define the LED array and button pins according to the active configuration
ledSegment led[3];
uint8_t colorBtn;
uint8_t animBtn;
uint8_t motorPinA;
uint8_t motorPinB;

// Set the default brightness modifier, 0.0 to 0.65 max
float currentBrightness = 0.4;
// Temporary brightness value to be used when previewing the new swatch after changing it
float pulseBrightness = 0.65;

// Brightness adjustment settings
const float minBrightness = 0.3;
const float maxBrightness = 0.6;
const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// Slow down all animations by this amou nt (in milliseconds)
const uint8_t slowDown = 1;

// Motor control settings
bool motorEnabled = true;       // Whether motor control is enabled for this hardware
uint8_t maxMotorSpeed = 255;    // Global motor speed limit (0-255, default: 180 = ~70% speed)

// LED Color tuning
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values for standard forward current
// {mA, luminosity}
const luminance red       = {5, 45};
const luminance green     = {5, 45};
const luminance blue      = {5, 55};

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig(activeConfig);

    // Copy pin configuration from the selected hardware profile
    led[0] = config.leds[0];
    led[1] = config.leds[1];
    led[2] = config.leds[2];
    colorBtn = config.colorButton;
    animBtn = config.animButton;
    motorPinA = config.motorPinA;
    motorPinB = config.motorPinB;

    // Set up motor pins (if configured)
    if (motorPinA != 0 && motorPinB != 0) {
        pinMode(motorPinA, OUTPUT);
        pinMode(motorPinB, OUTPUT);
        digitalWrite(motorPinA, LOW);
        digitalWrite(motorPinB, LOW); // Start with motor off
    } else {
        motorEnabled = false;
    }

    // Set up all LED segments
    for (uint8_t segment = 0; segment < 3; segment++) {
        // Only initialize non-zero pins (skip unconnected channels)
        if (led[segment].red != 0 || led[segment].green != 0 || led[segment].blue != 0) {
            pinMode(led[segment].red, OUTPUT);
            pinMode(led[segment].green, OUTPUT);
            pinMode(led[segment].blue, OUTPUT);
        }
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
        swNum = 23;
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
                glitchLoopWithAutoFocus(70, 20, 1000);
                break;
            // Add more animation modes here as you create them:
            // case 1:
            //     smoothPulseLoop();
            //     break;
            // case 2:
            //     strobeLoop();
            //     break;
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

    // Start motor during glitch effects
    if (motorEnabled && effectTrigger) {
        if (random(0, 2) == 0) {
            motorForward(); // Random direction
        } else {
            motorReverse();
        }
    }

    while (currentTime - startTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            motorOff(); // Turn off motor when interrupted
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
            // Normal flicker on both segments
            flicker(0, flickerChance, 200, 255);
            flicker(1, flickerChance, 0, 200);
            currentTime = millis();
        }
    }

    // Turn off motor when effect is done
    motorOff();
}

// -------------------------------------------------------------------------------------
// MARK: fadeToColor
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const int fadeTime){
    uint8_t startColor[3][3];
    uint8_t output[3][3];

    // Copy handoverColor to startColor
    for (int segment = 0; segment < numLEDs; segment++) {
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
                output[0][pin] = startColor[0][pin] + (color1[pin] - startColor[0][pin]) * fadeRatio;
                output[1][pin] = startColor[1][pin] + (color2[pin] - startColor[1][pin]) * fadeRatio;
                output[2][pin] = startColor[2][pin] + (color1[pin] - startColor[2][pin]) * fadeRatio;
            }
        sendToRGB(0, output[0]);
        sendToRGB(1, output[1]);
        sendToRGB(2, output[2]);
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
        sendToRGB(0, color1);
        sendToRGB(1, color2);
        sendToRGB(2, color1);
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
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        if (segment % 2 == 0) {
            showColor(swatch[swNum].contrast, swatch[swNum].accent, 50);
            showColor(swatch[swNum].contrast, swatch[swNum].background, 50);
        } else {
            showColor(swatch[swNum].accent, swatch[swNum].contrast, 50);
            showColor(swatch[swNum].background, swatch[swNum].contrast, 50);
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
    // Flash the specified segment between startColor and color2, keep others at handoverColor
    for (int reps = 0; reps < 3; reps++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            return; // Immediately exit to allow swatch preview to play
        }

        // Flash once with startColor
        for (uint8_t seg = 0; seg < numLEDs; seg++) {
            if (seg == segment) {
                sendToRGB(seg, startColor);
            } else {
                sendToRGB(seg, handoverColor[seg]);
            }
        }
        delay(duration);

        // Flash once with color2
        for (uint8_t seg = 0; seg < numLEDs; seg++) {
            if (seg == segment) {
                sendToRGB(seg, color2);
            } else {
                sendToRGB(seg, handoverColor[seg]);
            }
        }
        delay(duration);
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch4
void glitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();

    // Run motor during rapid color changes
    if (motorEnabled) {
        motorForward();
    }

    while (millis() - start < duration) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            motorOff();
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

    motorOff();
}

// -------------------------------------------------------------------------------------
// MARK: glitch5
void glitch5(){
    // Use one of the waveform arrays from waveforms.cpp
    uint8_t waveformIndex = random(0, 2); // Choose between the two available waveforms
    uint8_t outputColor[3];

    // Start motor as waveform plays
    if (motorEnabled) {
        motorReverse(); // Use reverse for this effect
    }

    // First play through the waveform once
    for (uint8_t i = 0; i < 32; i++) {
        // Check if swatch preview should interrupt this animation
        if (swatchPreviewActive) {
            motorOff();
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
            motorOff();
            return; // Immediately exit to allow swatch preview to play
        }

        uint8_t randomPos = random(0, 32);
        gradientPosition(waveform[waveformIndex].waveform[randomPos], outputColor);
        showColor(outputColor, outputColor, 30);

        // Brief flashes to black between jumps
        uint8_t blackColor[3] = {0, 0, 0};
        showColor(blackColor, blackColor, 15);
    }

    // End with a final dramatic fade to black and stop motor
    motorOff();
    fadeToColor(swatch[swNum].contrast, swatch[swNum].background, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t color1[3], uint8_t color2[3], int speed){
    showColor(color1, color2, speed);
    showColor(color2, color1, speed);
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

        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            sendToRGB(segment, outputColor);
        }

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (int i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            sendToRGB(segment, outputColor);
        }

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
        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            sendToRGB(segment, swatch[swNum].primary);
        }
        delay(flashDuration);

        // Dark flash
        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            sendToRGB(segment, swatch[swNum].background);
        }
        delay(flashDuration);
    }

    // Restore original brightness
    currentBrightness = originalBrightness;

    // Reset the flag
    animationPreviewActive = false;
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
        // Use direct LED control to avoid button checking interference
        for (int i = 0; i < 10; i++) { // Display multiple times per step for stability
            for (uint8_t segment = 0; segment < numLEDs; segment++) {
                sendToRGB(segment, whiteColor);
            }
            delay(stepDuration / 10);
        }
    }

    // Reset mode flag
    brightnessAdjustMode = false;
}

// -------------------------------------------------------------------------------------
// MARK: Motor Control Functions
// -------------------------------------------------------------------------------------

// Rotate motor forward (PinA HIGH, PinB LOW)
void motorForward() {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return; // Motor not available on this hardware
    }

    digitalWrite(motorPinA, HIGH);
    digitalWrite(motorPinB, LOW);
}

// Rotate motor in reverse (PinA LOW, PinB HIGH)
void motorReverse() {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return; // Motor not available on this hardware
    }

    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, HIGH);
}

// Stop motor (both pins LOW)
void motorOff() {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return;
    }

    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, LOW);
}

// Alias for motorOff
void motorStop() {
    motorOff();
}

// Software PWM for motor control (manually toggles pins for speed control)
// Call this repeatedly in a loop for continuous speed control
void motorPWMCycle(uint8_t speed, bool forward) {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return;
    }

    if (speed == 0) {
        motorOff();
        return;
    }

    // Software PWM: toggle the active pin HIGH for (speed) portion of cycle
    if (forward) {
        // Forward: PinA HIGH, PinB LOW
        digitalWrite(motorPinA, HIGH);
        digitalWrite(motorPinB, LOW);
        delayMicroseconds(speed * 2); // ON time proportional to speed

        // OFF time: both pins LOW
        digitalWrite(motorPinA, LOW);
        digitalWrite(motorPinB, LOW);
        delayMicroseconds((255 - speed) * 2); // OFF time
    } else {
        // Reverse: PinA LOW, PinB HIGH
        digitalWrite(motorPinA, LOW);
        digitalWrite(motorPinB, HIGH);
        delayMicroseconds(speed * 2); // ON time proportional to speed

        // OFF time: both pins LOW
        digitalWrite(motorPinA, LOW);
        digitalWrite(motorPinB, LOW);
        delayMicroseconds((255 - speed) * 2); // OFF time
    }
}

// Motor control with PWM speed (0-255), scaled by maxMotorSpeed
// Note: Speed control via software PWM doesn't work well for motors on non-PWM pins
// This now just sets direction and ignores speed parameter
void motorForwardSpeed(uint8_t speed) {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return;
    }

    // For non-PWM pins, just use full digital control
    digitalWrite(motorPinA, HIGH);
    digitalWrite(motorPinB, LOW);
}

void motorReverseSpeed(uint8_t speed) {
    if (!motorEnabled || motorPinA == 0 || motorPinB == 0) {
        return;
    }

    // For non-PWM pins, just use full digital control
    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, HIGH);
}

// -------------------------------------------------------------------------------------
// MARK: motorDemo
// Demo function: alternates motor direction with pauses
void motorDemo() {
    // Rotate forward for 1 second
    motorForward();
    delay(1000);

    // Pause for 1 second
    motorOff();
    delay(1000);

    // Rotate reverse for 1 second
    motorReverse();
    delay(1000);

    // Pause for 1 second
    motorOff();
    delay(1000);
}

// -------------------------------------------------------------------------------------
// MARK: motorAutoFocus
// Simulates camera auto-focus or sensor target detection behavior
// Randomly rotates in different directions at varying speeds with pauses
void motorAutoFocus() {
    unsigned long startTime = millis();

    // Run for a cycle duration
    while (millis() - startTime < 5000) { // 5 second cycles
        // Check if swatch preview should interrupt
        if (swatchPreviewActive) {
            motorOff();
            return;
        }

        // Randomly decide action: 0=stop, 1=forward, 2=reverse
        uint8_t action = random(0, 3);

        // Random duration (50-500ms for quick, searching movements)
        int duration = random(50, 500);

        switch(action) {
            case 0:
                // Stop/pause (simulating lock-on or search pause)
                motorOff();
                break;
            case 1:
                // Move forward (full speed)
                motorForward();
                break;
            case 2:
                // Move reverse (full speed)
                motorReverse();
                break;
        }

        delay(duration);
    }

    // End with motor off
    motorOff();
}

// -------------------------------------------------------------------------------------
// MARK: glitchLoopWithAutoFocus
// Combines glitchLoop LED animation with motorAutoFocus motor behavior
void glitchLoopWithAutoFocus(const uint8_t flickerChance, const uint8_t effectChance, const int duration) {
    unsigned long startTime = millis();
    unsigned long lastMotorChange = millis();
    unsigned long motorAnimCheck = millis();
    int motorDuration = random(50, 200); // Initial random motor duration
    bool effectTrigger = random(0, 100) < effectChance;

    while (millis() - startTime < duration) {
        // Check if swatch preview should interrupt
        if (swatchPreviewActive) {
            motorOff();
            return;
        }

        // Check for a motor animation trigger
        if (millis() - motorAnimCheck >= 800) {
            motorAnimCheck = millis();
            if (random(0, 100) < 40) {
                // Wiggle function
                unsigned long wiggleStart = millis();
                while (millis() - wiggleStart < 1000) {
                    motorForward();
                    delay(100);
                    motorReverse();
                    delay(100);
                }
                motorOff();
                lastMotorChange = millis(); // Reset motor timer after wiggle
            }
        }

        // Handle motor auto-focus behavior (heavily weighted toward OFF)
        if (millis() - lastMotorChange >= motorDuration) {
            // Time to change motor action - heavily weighted toward OFF (0-7=off, 8=forward, 9=reverse)
            uint8_t action = random(0, 10);
            motorDuration = random(100, 400); // Longer durations for more stability
            lastMotorChange = millis();

            if (action < 8) {
                // 80% chance of being off
                motorOff();
            } else if (action == 8) {
                // 10% chance forward
                uint8_t speed = random(120, 256);
                motorForwardSpeed(speed);
            } else {
                // 10% chance reverse
                uint8_t speed = random(120, 256);
                motorReverseSpeed(speed);
            }
        }

        // Run glitch loop behavior
        if (effectTrigger) {
            // Apply a special effect
            uint8_t flickerSegment = random(0, numLEDs);
            switch (random(0, 4)) {
                case 0:
                    glitch1(flickerSegment, 700);
                    break;
                case 1:
                    glitch2(swatch[swNum].midtone, swatch[swNum].contrast, 700);
                    break;
                case 2:
                    glitch3(flickerSegment, swatch[swNum].primary, 20, 3);
                    break;
                case 3:
                    glitch4(6, 700);
                    break;
                /*case 4:
                    glitch5();
                    break;
                case 5:
                    fakeMorse(65, 210, 400);
                    break;
                    */
            }
        } else {
            // Normal flicker on segments
            flicker(0, flickerChance, 200, 255);
            flicker(1, flickerChance, 0, 200);
        }

        // Check if we should trigger a new effect
        if (random(0, 100) < effectChance) {
            effectTrigger = true;
        } else {
            effectTrigger = false;
        }
    }

    motorOff();
}
