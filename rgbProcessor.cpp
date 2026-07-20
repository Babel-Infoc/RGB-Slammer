#include <Arduino.h>
#include "hardware.h"
#include "swatches.h"
#include "flashStorage.h"

hardwareConfig config;

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;
uint8_t colorIndex = 0;

uint8_t debounceStart = 0;

// Reference to the led array and buttons defined in the main sketch
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern uint8_t currentBrightness;
extern zoneConfig led[5];

// Animation mode (0 = glitchLoop, 1 = eye animation, expand as needed)
uint8_t animationMode = 0;
const uint8_t numAnimationModes = 5;

// Swatch preview animation flag
bool swatchPreviewActive = false;

// Animation preview flag
bool animationPreviewActive = false;

// Brightness adjustment mode variables
unsigned long buttonPressStartTime = 0;
bool buttonHeldFor2Seconds = false;

uint8_t tuneRatio[3] = {255, 255, 255};
uint8_t handoverColor[5][3] = {};

// Gamma Correction
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

// MARK: Luminance Calculation
void calculateLuminance() {
    // Calculate output efficiency for each color channel
    uint8_t efficiency[3];
    for (uint8_t i = 0; i < 3; i++) {
        efficiency[i] = config.tuning[i][2] * 255 / (config.tuning[i][1]   ? config.tuning[i][1]   : 1);
    }

    // Select the least efficient channel as the reference
    uint16_t lowestEfficiency = min(efficiency[0], min(efficiency[1], efficiency[2]));

    // If lowestEfficiency=0, then all color values are equal and balanced, set all to max brightness 65535
    if (lowestEfficiency == 0) {
        tuneRatio[0] = tuneRatio[1] = tuneRatio[2] = 255;
        return;
    }

    for (uint8_t i = 0; i < 3; i++) {
        // More efficient channel → smaller ratio → less output → balanced white.
        tuneRatio[i]  = (uint16_t)min((uint32_t)255, (uint32_t)lowestEfficiency * 255 / efficiency[i]);
    }
}

// MARK: Button Handling
void checkButtons() {
    // Don't process button events while a preview animation is playing.
    // sendToRGB calls checkButtons on every PWM frame, so without this guard
    // a button release detected mid-animation would advance swNum a second time.
    if (swatchPreviewActive || animationPreviewActive) return;

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
                    // Short press - change swatch (flash save deferred to end of swatchPreview)
                    swNum = (swNum + 1) % numSwatches;
                    swatchPreviewActive = true;
                } else {
                    // Long press was released - exit brightness mode (flash save deferred to end of brightnessAdjustmentMode)
                    brightnessAdjustMode = false;
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

        // Check animation button (skipped when no button is wired to this config)
        if (animBtn != PIN_NONE) {
            uint8_t animButtonState = digitalRead(animBtn);

            if (animButtonState != animButtonLastState) {
                if (animButtonState == LOW) {
                    // Button just pressed - cycle to next animation mode (flash save deferred to end of animationPreview)
                    animationMode = (animationMode + 1) % numAnimationModes;
                    animationPreviewActive = true;
                }
                animButtonLastState = animButtonState;
            }
        }

        // Restart the debounce timer
        debounceStart = 20; // Reduced for more responsive long press detection
    }
}

// MARK: Shift Register Output
// Reference to shift register globals defined in the main sketch
extern shiftRegPins shiftReg;
extern uint8_t srLEDs;

// SR update callback — called by sendToRGB before every GPIOLEDLED (Core) PWM flush.
// Set from the animation loop to eyeDoublePulse, eyeScatter, or any other function
// that writes handoverColor[]. nullptr = no update; SR holds its previous state.
void (*srUpdateCallback)() = nullptr;

// ============================================================================
// MARK: RGB Processing

void sendToRGB(const uint8_t zone, const uint8_t rgbValue[3]) {
    uint32_t rRatio, gRatio, bRatio;
    int tunedRGB[3];
    // Record the handover color
    for (uint8_t i = 0; i < 3; i++) {
        handoverColor[zone][i] = rgbValue[i];
        // If the zone in question is a shift register zone, bounce out now
        // all SR colors will be calculated in one go later
        if (led[zone].type == SHIFTREG) return;
    }

    // Let the registered SR callback update handoverColor[] for this frame.
    if (srUpdateCallback) srUpdateCallback();

    // Gamma-correct the direct zone colour, then apply per-zone output scale.
    // Pipeline: tuneRatio → gamma8 (perceptual→physical) → currentBrightness (linear) → zoneOutputScale.
    // Brightness is applied as a linear multiplier after gamma-correcting the colour.
    // Applying gamma to currentBrightness would collapse the effective range.
    uint8_t tunedColor[3];

    // Apply white balance
    for (uint8_t i = 0; i < 3; i++) {
        // Apply white balance adjustments
        tunedColor[i]    = (uint8_t)(((uint16_t)rgbValue[i]     * tuneRatio[i]) >> 8);
        // Apply currentBrightness modifier
        tunedColor[i]    = (uint8_t)(((uint16_t)tunedColor[i]   * currentBrightness) >> 8);
        // Apply per zone brightness scaling
        //tunedColor[i]    = (uint8_t)(((uint16_t)tunedColor[i]   * led[zone].scale) >> 8);
        // Apply gamma correction
        //tunedColor[i]    = pgm_read_byte(&gamma8[tunedColor[i]]);
    }

    // Combined software-PWM loop with 256 steps for better low-brightness resolution.
    // Direct zone: active-HIGH (LOW = on).
    // SR channels:    active-LOW  (HIGH = off) — bytes start 0xFF, bits CLEARED to turn on.
    // QG/QH of SR1 (BTN1/BTN2) never cleared — remain HIGH at all times.
    // SR bytes are only re-clocked when they change, reducing shiftOut calls.
    uint8_t prev_sr1 = 0, prev_sr2 = 0; // initialised to 0 so first iteration always clocks
    for (int brightness = 0; brightness < 256; brightness++) {
        // Drive direct LED only for valid zones
        if (led[zone].type == GPIOLED) {
            digitalWrite(led[zone].r, brightness < tunedColor[0]     ? LOW : HIGH);
            digitalWrite(led[zone].g, brightness < tunedColor[1]     ? LOW : HIGH);
            digitalWrite(led[zone].b, brightness < tunedColor[2]     ? LOW : HIGH);
        }
    }

        /*
    // PWM Conversion
    if (led[zone].type == GPIOLED) {
        digitalWrite(led[zone].r, HIGH);
        digitalWrite(led[zone].g, HIGH);
        digitalWrite(led[zone].b, HIGH);
    }
    */

    // Check buttons once per frame, with debounce handling
    checkButtons();
    // Slow down to prevent excessive CPU usage
    delay(config.slowDown);
}
