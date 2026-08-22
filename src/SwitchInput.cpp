#include "SwitchInput.h"
#include <Arduino.h>
#include "Config.h"

namespace {
bool s_debouncedOpen = true; // Default: offen (HIGH), passend zu INPUT_PULLUP
bool s_lastRawOpen = true;
uint32_t s_lastChangeMs = 0;
} // namespace

namespace SwitchInput {

void begin() {
    pinMode(PIN_SWITCH, INPUT_PULLUP);
    s_debouncedOpen = digitalRead(PIN_SWITCH) == HIGH;
    s_lastRawOpen = s_debouncedOpen;
}

void loop() {
    bool rawOpen = digitalRead(PIN_SWITCH) == HIGH;
    uint32_t now = millis();

    if (rawOpen != s_lastRawOpen) {
        s_lastRawOpen = rawOpen;
        s_lastChangeMs = now;
    }

    if (rawOpen != s_debouncedOpen && (now - s_lastChangeMs) >= SWITCH_DEBOUNCE_MS) {
        s_debouncedOpen = rawOpen;
    }
}

bool isOpen() {
    return s_debouncedOpen;
}

} // namespace SwitchInput
