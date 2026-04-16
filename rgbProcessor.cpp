#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "flashStorage.h"

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;
uint8_t colorIndex = 0;

uint8_t debounceStart = 0;

// Animation mode (0 = glitchLoop, 1 = eye animation, expand as needed)
uint8_t animationMode = 0;
const uint8_t numAnimationModes = 3;

// Swatch preview animation flag
bool swatchPreviewActive = false;

// Animation preview flag
bool animationPreviewActive = false;

// Brightness adjustment mode variables
unsigned long buttonPressStartTime = 0;
bool buttonHeldFor2Seconds = false;

// Replace std::array with plain C arrays to save significant flash memory
float tuneRatio[3] = {1.0, 1.0, 1.0};
uint8_t handoverColor[MAX_LED_SEGMENTS][3] = {};

// Reference to the led array and buttons defined in the main sketch
extern ledSegment led[MAX_LED_SEGMENTS];
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern float currentBrightness;

// MARK: Gamma Correction ------------------------------
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
    2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,   5,
    5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,   9,
    9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15,  15,
   16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23,  24,
   24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34,  35,
   35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  49,
   50, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,  66,
   67, 68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85,  86,
   87, 89, 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109, 110,
  112,114,115,117,119,120,122,124,126,127,129,131,133,135,137, 138,
  140,142,144,146,148,150,152,154,156,158,160,162,164,167,169, 171,
  173,175,177,180,182,184,186,189,191,193,196,198,200,203,205, 208,
  210,213,215,218,220,223,225,228,231,233,236,239,241,244,247, 249,
  252,255 };

// MARK: Luminance Calculation ------------------------------
// Function to calculate luminance ratios for consistent color output
void calculateLuminance() {
    // Calculate the effective luminance per mA for each LED
    float rEfficiency = red.luminance / red.mA;
    float gEfficiency = green.luminance / green.mA;
    float bEfficiency = blue.luminance / blue.mA;

    // Find the most efficient LED (highest luminance per mA)
    float maxEfficiency = max(rEfficiency, max(gEfficiency, bEfficiency));

    // Scale all LEDs DOWN from the most efficient one
    // This ensures no LED is driven above its baseline
    tuneRatio[0] = rEfficiency / maxEfficiency; // Red ratio (≤ 1.0)
    tuneRatio[1] = gEfficiency / maxEfficiency; // Green ratio (≤ 1.0)
    tuneRatio[2] = bEfficiency / maxEfficiency; // Blue ratio (≤ 1.0)
}

// MARK: Button Handling ------------------------------
void checkButtons() {
    if (debounceStart > 0) {
        debounceStart--;
    } else {
        // Check color button
        uint8_t colorButtonState = digitalRead(colorBtn);

        if (colorButtonState != colorButtonLastState) {
            if (colorButtonState == LOW) {
                // Button just pressed - start timing
                buttonPressStartTime = millis();
                buttonHeldFor2Seconds = false;
            } else {
                // Button just released
                if (!buttonHeldFor2Seconds) {
                    // Short press - change swatch
                    swNum = (swNum + 1) % numSwatches;
                    saveSettingsToFlash(swNum, currentBrightness, animationMode);
                    swatchPreviewActive = true;
                } else {
                    // Long press was released - exit brightness mode and save brightness
                    brightnessAdjustMode = false;
                    // Save both swatch and current brightness to flash
                    saveSettingsToFlash(swNum, currentBrightness, animationMode);
                }
                buttonHeldFor2Seconds = false;
            }
            colorButtonLastState = colorButtonState;
        } else if (colorButtonState == LOW && !buttonHeldFor2Seconds) {
            // Button is being held - check if trigger time has passed
            if (millis() - buttonPressStartTime >= brightnessModeTriggerTime) {
                buttonHeldFor2Seconds = true;
                brightnessAdjustMode = true;
            }
        }

        // Check animation button
        uint8_t animButtonState = digitalRead(animBtn);

        if (animButtonState != animButtonLastState) {
            if (animButtonState == LOW) {
                // Button just pressed - cycle to next animation mode
                animationMode = (animationMode + 1) % numAnimationModes;
                saveSettingsToFlash(swNum, currentBrightness, animationMode);
                animationPreviewActive = true;
            }
            animButtonLastState = animButtonState;
        }

        // Restart the debounce timer
        debounceStart = 10; // Reduced for more responsive long press detection
    }
}

// MARK: Light sensor ------------------------------

// MARK: Shift Register Output ------------------------------
// Reference to shift register globals defined in the main sketch
extern shiftRegPins shiftReg;
extern uint8_t numShiftRegChannels;

// Current colors for each shift register channel (up to 4)
uint8_t shiftRegColors[4][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};

// MARK: RGB Processing ------------------------------
// Each sendToRGB call dispatches on segment type:
//   GPIO segment  — gamma-corrects the colour, runs a full software-PWM frame that drives
//                    the GPIO pins AND clocks shiftRegColors to all SR channels simultaneously.
//   SR segment    — buffers the colour into shiftRegColors[]; the buffer is output on the
//                    next GPIO segment’s PWM frame, keeping all outputs in temporal sync.
// Call SR segments before the GPIO segment within one animation step for zero lag.
extern uint8_t numLEDs;
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]) {

    // Always record this colour as the handover value for transition animations
    if (segment < numLEDs) {
        for (uint8_t i = 0; i < 3; i++) {
            handoverColor[segment][i] = rgbValue[i];
        }
    }

    // SR segment: buffer the colour and return early.
    // It will be clocked out during the next GPIO segment’s PWM frame.
    if (segment < numLEDs && led[segment].isSR) {
        uint8_t ch = led[segment].srChannel;
        for (uint8_t i = 0; i < 3; i++) {
            shiftRegColors[ch][i] = rgbValue[i];
        }
        return;
    }

    // GPIO segment: run full software-PWM loop ----------------------------------

    // Brightness adjustment mode: suppress SR output — pods show black
    if (brightnessAdjustMode && numShiftRegChannels > 0) {
        memset(shiftRegColors, 0, sizeof(shiftRegColors));
    }

    // Gamma-correct the direct segment colour, then apply per-type output scale
    int tunedRGB[3];
    for (uint8_t pin = 0; pin < 3; pin++) {
        float adjustedValue = rgbValue[pin] * tuneRatio[pin] * currentBrightness;
        int   constrained   = constrain((int)adjustedValue, 0, 255);
        tunedRGB[pin]       = constrain((int)(pgm_read_byte(&gamma8[constrained]) * coreOutputScale), 0, 255);
    }

    // Gamma-correct all SR channel colours, then apply per-type output scale
    uint8_t tunedSR[4][3];
    if (numShiftRegChannels > 0) {
        for (uint8_t ch = 0; ch < 4; ch++) {
            for (uint8_t pin = 0; pin < 3; pin++) {
                float adjustedValue = shiftRegColors[ch][pin] * tuneRatio[pin] * currentBrightness;
                int   constrained   = constrain((int)adjustedValue, 0, 255);
                tunedSR[ch][pin]    = constrain((int)(pgm_read_byte(&gamma8[constrained]) * srOutputScale), 0, 255);
            }
        }
    }

    // Combined software-PWM loop.
    // Direct segment: active-HIGH (LOW = on).
    // SR channels:    active-LOW  (HIGH = off) — bytes start 0xFF, bits CLEARED to turn on.
    // QG/QH of SR1 (BTN1/BTN2) never cleared — remain HIGH at all times.
    for (int brightness = 0; brightness < 100; brightness++) {

        // Drive direct LED only for valid segments
        if (segment < numLEDs) {
            digitalWrite(led[segment].red,   brightness < tunedRGB[0] ? LOW : HIGH);
            digitalWrite(led[segment].green, brightness < tunedRGB[1] ? LOW : HIGH);
            digitalWrite(led[segment].blue,  brightness < tunedRGB[2] ? LOW : HIGH);
        }

        if (numShiftRegChannels > 0) {
            uint8_t sr1Byte = 0xFF; // U19: bits 0-5 = LEDs, bits 6-7 = BTN1/BTN2 (stay HIGH)
            uint8_t sr2Byte = 0xFF; // U20: bits 0-5 = LEDs, bits 6-7 unused (stay HIGH)

            // U19 (SR1): Ch2 QA-QC, Ch3 QD-QF
            if (brightness < tunedSR[0][0]) sr1Byte &= ~(1 << 0); // QA = RED1
            if (brightness < tunedSR[0][1]) sr1Byte &= ~(1 << 1); // QB = GREEN1
            if (brightness < tunedSR[0][2]) sr1Byte &= ~(1 << 2); // QC = BLUE1
            if (brightness < tunedSR[1][0]) sr1Byte &= ~(1 << 3); // QD = RED2
            if (brightness < tunedSR[1][1]) sr1Byte &= ~(1 << 4); // QE = GREEN2
            if (brightness < tunedSR[1][2]) sr1Byte &= ~(1 << 5); // QF = BLUE2

            // U20 (SR2): Ch4 QA-QC, Ch5 QD-QF
            if (brightness < tunedSR[2][0]) sr2Byte &= ~(1 << 0); // QA = RED3
            if (brightness < tunedSR[2][1]) sr2Byte &= ~(1 << 1); // QB = GREEN3
            if (brightness < tunedSR[2][2]) sr2Byte &= ~(1 << 2); // QC = BLUE3
            if (brightness < tunedSR[3][0]) sr2Byte &= ~(1 << 3); // QD = RED4
            if (brightness < tunedSR[3][1]) sr2Byte &= ~(1 << 4); // QE = GREEN4
            if (brightness < tunedSR[3][2]) sr2Byte &= ~(1 << 5); // QF = BLUE4

            digitalWrite(shiftReg.rclk, LOW);
            shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, sr2Byte); // U20 first
            shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, sr1Byte); // U19 second
            digitalWrite(shiftReg.rclk, HIGH);
        }
    }

    // Return all outputs to idle-off after the PWM frame
    if (segment < numLEDs) {
        digitalWrite(led[segment].red,   HIGH);
        digitalWrite(led[segment].green, HIGH);
        digitalWrite(led[segment].blue,  HIGH);
    }
    if (numShiftRegChannels > 0) {
        digitalWrite(shiftReg.rclk, LOW);
        shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, 0xFF);
        shiftOut(shiftReg.ser, shiftReg.srclk, MSBFIRST, 0xFF);
        digitalWrite(shiftReg.rclk, HIGH);
    }

    // Check buttons once per frame, with debounce handling
    checkButtons();
    // Slow down to prevent excessive CPU usage
    delay(slowDown);
}

// MARK: Role-based output ------------------------------
// Convenience wrapper: calls sendToRGB for every segment whose role matches.
// For correct SR/GPIO ordering the caller should invoke ROLE_EYE first (SR buffer),
// then ROLE_CORE (triggers the PWM frame that flushes the SR buffer).
void sendToRole(uint8_t role, const uint8_t color[3]) {
    for (uint8_t seg = 0; seg < numLEDs; seg++) {
        if (led[seg].role == role) {
            sendToRGB(seg, color);
        }
    }
}
