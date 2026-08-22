#include "RadarSensor.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ld2410.h>
#include "Config.h"

namespace {
SoftwareSerial s_radarSerial(PIN_RADAR_RX, PIN_RADAR_TX); // RX, TX
ld2410 s_radar;
uint32_t s_lastPresenceMs = 0;
bool s_everSeenPresence = false;
} // namespace

namespace RadarSensor {

void begin() {
    s_radarSerial.begin(RADAR_BAUD);
    if (s_radar.begin(s_radarSerial)) {
        Serial.println("[Radar] LD2410 gefunden und initialisiert.");
    } else {
        Serial.println("[Radar] LD2410 NICHT gefunden - Verkabelung/Baudrate pruefen.");
    }
}

void loop() {
    s_radar.read();
    if (presenceDetected()) {
        s_lastPresenceMs = millis();
        s_everSeenPresence = true;
    }
}

bool presenceDetected() {
    return s_radar.presenceDetected();
}

bool presenceHeld() {
    if (!s_everSeenPresence) {
        return false;
    }
    return (millis() - s_lastPresenceMs) < RADAR_HOLD_MS;
}

} // namespace RadarSensor
