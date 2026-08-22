#include "LedController.h"
#include "Config.h"

namespace {
bool s_targetOn = false;
uint8_t s_currentPwm = 0;
uint32_t s_lastStepMs = 0;

// Schrittweite so waehlen, dass ein voller Fade ~LED_FADE_MS dauert.
constexpr uint32_t STEP_INTERVAL_MS = LED_FADE_MS / LED_PWM_MAX > 0
                                           ? LED_FADE_MS / LED_PWM_MAX
                                           : 1;
} // namespace

namespace LedController {

void begin() {
    pinMode(PIN_LED_GATE, OUTPUT);
    analogWriteRange(LED_PWM_MAX);
    analogWrite(PIN_LED_GATE, 0);
}

void setOn(bool on) {
    s_targetOn = on;
}

bool isOn() {
    return s_targetOn;
}

void loop() {
    uint8_t target = s_targetOn ? LED_PWM_MAX : 0;
    if (s_currentPwm == target) {
        return;
    }

    uint32_t now = millis();
    if (now - s_lastStepMs < STEP_INTERVAL_MS) {
        return;
    }
    s_lastStepMs = now;

    if (s_currentPwm < target) {
        s_currentPwm++;
    } else {
        s_currentPwm--;
    }
    analogWrite(PIN_LED_GATE, s_currentPwm);
}

} // namespace LedController
