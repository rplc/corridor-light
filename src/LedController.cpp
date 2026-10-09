#include "LedController.h"
#include "Config.h"

namespace {
bool s_targetOn = false;
uint8_t s_brightnessPercent = DEFAULT_BRIGHTNESS_FORCED_ON_PCT;
uint8_t s_currentPwm = 0;
uint32_t s_lastStepMs = 0;

// Schrittweite so waehlen, dass ein voller Fade ~LED_FADE_MS dauert.
constexpr uint32_t STEP_INTERVAL_MS = LED_FADE_MS / LED_PWM_MAX > 0
                                           ? LED_FADE_MS / LED_PWM_MAX
                                           : 1;

uint8_t percentToPwm(uint8_t percent) {
    if (percent > 100) percent = 100;
    return static_cast<uint8_t>((static_cast<uint16_t>(percent) * LED_PWM_MAX) / 100);
}
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

void setBrightness(uint8_t percent) {
    s_brightnessPercent = percent > 100 ? 100 : percent;
}

bool isOn() {
    return s_targetOn;
}

uint8_t brightness() {
    return s_brightnessPercent;
}

void loop() {
    uint8_t target = s_targetOn ? percentToPwm(s_brightnessPercent) : 0;
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
