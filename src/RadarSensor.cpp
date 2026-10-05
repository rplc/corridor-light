#include "RadarSensor.h"
#include <Arduino.h>
#include <ld2410.h>
#include "Config.h"
#include "DebugLog.h"

namespace {
ld2410 s_radar;
uint32_t s_lastPresenceMs = 0;
bool s_everSeenPresence = false;
} // namespace

namespace RadarSensor {

void begin() {
    // UART0 ist exklusiv fuer den LD2410. Nach Serial.begin() wird er mit
    // Serial.swap() auf D7/GPIO13 (RX) und D8/GPIO15 (TX) gelegt.
    // Die Debug-Ausgaben laufen separat ueber Serial1 (siehe DebugLog).
    Serial.begin(RADAR_BAUD, SERIAL_8N1);
    Serial.swap();

    // Hinweis: begin()/requestCurrentConfiguration() laufen VOR
    // WiFi/MQTT (siehe main.cpp setup()) - diese Meldungen landen also
    // erstmal nur im DebugLog-Puffer und werden gesendet, sobald MQTT
    // steht. Auf Serial stehen sie trotzdem sofort.
    if (s_radar.begin(Serial)) {
        DebugLog::logf("[Radar] LD2410 gefunden und initialisiert.");
        if (s_radar.requestCurrentConfiguration()) {
            DebugLog::logf("[Radar] Konfiguration gelesen: max_gate=%u max_moving_gate=%u max_stationary_gate=%u",
                           s_radar.max_gate, s_radar.max_moving_gate, s_radar.max_stationary_gate);
        } else {
            DebugLog::logf("[Radar] Konnte aktuelle Konfiguration nicht lesen.");
        }
    } else {
        DebugLog::logf("[Radar] LD2410 NICHT gefunden - Verkabelung/Baudrate pruefen.");
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

bool movingTargetDetected() {
    return s_radar.movingTargetDetected();
}

uint16_t movingTargetDistanceCm() {
    return s_radar.movingTargetDistance();
}

uint8_t movingTargetEnergy() {
    return s_radar.movingTargetEnergy();
}

bool stationaryTargetDetected() {
    return s_radar.stationaryTargetDetected();
}

uint16_t stationaryTargetDistanceCm() {
    return s_radar.stationaryTargetDistance();
}

uint8_t stationaryTargetEnergy() {
    return s_radar.stationaryTargetEnergy();
}

uint8_t maxGate() {
    return s_radar.max_gate;
}

uint8_t maxMovingGate() {
    return s_radar.max_moving_gate;
}

uint8_t maxStationaryGate() {
    return s_radar.max_stationary_gate;
}

uint8_t motionSensitivity(uint8_t gate) {
    if (gate > s_radar.max_gate) return 0;
    return s_radar.motion_sensitivity[gate];
}

uint8_t stationarySensitivity(uint8_t gate) {
    if (gate > s_radar.max_gate) return 0;
    return s_radar.stationary_sensitivity[gate];
}

void applyConfig(const RadarConfigRequest& config) {
    bool anyChange = false;

    if (config.hasMaxValues) {
        bool ok = s_radar.setMaxValues(config.maxMovingGate, config.maxStationaryGate, config.timeoutSeconds);
        DebugLog::logf("[Radar] setMaxValues(moving=%u, stationary=%u, timeout=%us) -> %s",
                       config.maxMovingGate, config.maxStationaryGate, config.timeoutSeconds,
                       ok ? "OK" : "FEHLER/Timeout");
        anyChange = true;
    }

    for (uint8_t i = 0; i < config.gateSensitivityCount; i++) {
        const RadarGateSensitivity& g = config.gateSensitivity[i];
        bool ok = s_radar.setGateSensitivityThreshold(g.gate, g.moving, g.stationary);
        DebugLog::logf("[Radar] setGateSensitivityThreshold(gate=%u, moving=%u, stationary=%u) -> %s",
                       g.gate, g.moving, g.stationary, ok ? "OK" : "FEHLER/Timeout");
        anyChange = true;
    }

    if (anyChange) {
        // Cache (max_gate, Sensitivitaets-Arrays, ...) aktualisieren, damit
        // motionSensitivity()/stationarySensitivity()/maxGate() etc. den
        // neuen Stand widerspiegeln (z.B. fuer die Debug-Telemetrie).
        s_radar.requestCurrentConfiguration();
    }
}

} // namespace RadarSensor
