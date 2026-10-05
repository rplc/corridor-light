#include <Arduino.h>
#include "Config.h"
#include "DebugLog.h"
#include "WifiManager.h"
#include "OtaManager.h"
#include "MqttHandler.h"
#include "LedController.h"
#include "SwitchInput.h"
#include "RadarSensor.h"
#include "RadarTypes.h"

namespace {

bool s_lastPublishedLedOn = false;
bool s_lastPublishedPresence = false;
bool s_lastPublishedSwitchOpen = false;
uint8_t s_lastPublishedBrightness = 0;
bool s_havePublished = false;

#if RADAR_DEBUG_TELEMETRY
uint32_t s_lastRadarTelemetryMs = 0;
#endif

// Kernlogik: aus mode + radar_armed/switch_armed + Sensor-Zustaenden den
// gewuenschten LED-Zustand ableiten.
bool computeDesiredLedState() {
    using Mode = MqttHandler::Mode;
    switch (MqttHandler::mode()) {
        case Mode::On:
            return true;
        case Mode::Off:
            return false;
        case Mode::Auto:
        default: {
            bool byRadar  = MqttHandler::radarArmed() && RadarSensor::presenceHeld();
            bool bySwitch = MqttHandler::switchArmed() && SwitchInput::isOpen();
            return byRadar || bySwitch;
        }
    }
}

} // namespace

void setup() {
    Serial1.begin(DEBUG_BAUD);
    DebugLog::begin();
    DebugLog::logf("[Boot] ESP8266 LED-Strip-Controller");

    LedController::begin();   // zuerst: Gate sicher auf LOW ziehen
    SwitchInput::begin();
    RadarSensor::begin();

    WifiManager::begin();
    OtaManager::begin();
    MqttHandler::begin();
}

void loop() {
    WifiManager::loop();
    OtaManager::loop();
    MqttHandler::loop();
    SwitchInput::loop();
    RadarSensor::loop();

    // Per MQTT empfangene Radar-Konfiguration (Fein-Tuning) anwenden, falls
    // seit dem letzten Durchlauf eine angekommen ist.
    RadarConfigRequest radarConfig;
    if (MqttHandler::consumeRadarConfigRequest(radarConfig)) {
        RadarSensor::applyConfig(radarConfig);
    }

    bool desiredOn = computeDesiredLedState();
    LedController::setBrightness(MqttHandler::brightness());
    LedController::setOn(desiredOn);
    LedController::loop();

    bool ledOn = LedController::isOn();
    bool presence = RadarSensor::presenceHeld();
    bool switchOpen = SwitchInput::isOpen();
    uint8_t brightness = LedController::brightness();

    bool changed = !s_havePublished
                   || ledOn != s_lastPublishedLedOn
                   || presence != s_lastPublishedPresence
                   || switchOpen != s_lastPublishedSwitchOpen
                   || brightness != s_lastPublishedBrightness;

    if (MqttHandler::isConnected() && changed) {
        MqttHandler::publishState(ledOn, presence, switchOpen);
        s_lastPublishedLedOn = ledOn;
        s_lastPublishedPresence = presence;
        s_lastPublishedSwitchOpen = switchOpen;
        s_lastPublishedBrightness = brightness;
        s_havePublished = true;
    }

#if RADAR_DEBUG_TELEMETRY
    uint32_t now = millis();
    if (MqttHandler::isConnected() && now - s_lastRadarTelemetryMs >= RADAR_TELEMETRY_INTERVAL_MS) {
        s_lastRadarTelemetryMs = now;
        MqttHandler::publishRadarDebug();
    }
#endif

    // Gepufferte Log-Zeilen (siehe DebugLog) rausschicken, sobald MQTT
    // verbunden ist - Ersatz fuer den Serial-Monitor, wenn der ohne USB
    // (Strombudget) nicht nutzbar ist.
    static char s_logBuffer[700];
    if (MqttHandler::isConnected() && DebugLog::consumePending(s_logBuffer, sizeof(s_logBuffer))) {
        MqttHandler::publishLog(s_logBuffer);
    }
}
