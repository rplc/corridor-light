#include "SwitchInput.h"
#include <Arduino.h>
#include "Config.h"

namespace {
bool s_debouncedTriggered = false;
bool s_lastRawTriggered = false;
uint32_t s_lastChangeMs = 0;
} // namespace

namespace SwitchInput {

void begin() {
    pinMode(PIN_SWITCH, INPUT_PULLUP);
    s_debouncedTriggered = digitalRead(PIN_SWITCH) == LOW;
    s_lastRawTriggered = s_debouncedTriggered;
}

void loop() {
    bool rawTriggered = digitalRead(PIN_SWITCH) == LOW;
    uint32_t now = millis();

    if (rawTriggered != s_lastRawTriggered) {
        s_lastRawTriggered = rawTriggered;
        s_lastChangeMs = now;
    }

    if (rawTriggered != s_debouncedTriggered && (now - s_lastChangeMs) >= SWITCH_DEBOUNCE_MS) {
        s_debouncedTriggered = rawTriggered;
    }
}

bool isTriggered() {
    return s_debouncedTriggered;
}

} // namespace SwitchInput
