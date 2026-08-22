#include <Arduino.h>
#include "Config.h"
#include "WifiManager.h"
#include "OtaManager.h"
#include "MqttHandler.h"
#include "LedController.h"
#include "SwitchInput.h"
#include "RadarSensor.h"

namespace {

bool s_lastPublishedLedOn = false;
uint32_t s_lastStatePublishMs = 0;

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
    Serial.begin(115200);
    Serial.println("\n[Boot] ESP8266 LED-Strip-Controller");

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

    bool desiredOn = computeDesiredLedState();
    LedController::setOn(desiredOn);
    LedController::loop();

    uint32_t now = millis();
    bool ledOn = LedController::isOn();
    bool ledChanged = ledOn != s_lastPublishedLedOn;
    bool intervalElapsed = now - s_lastStatePublishMs >= STATE_PUBLISH_INTERVAL_MS;

    if (MqttHandler::isConnected() && (ledChanged || intervalElapsed)) {
        MqttHandler::publishState(ledOn, RadarSensor::presenceHeld(), SwitchInput::isOpen());
        s_lastPublishedLedOn = ledOn;
        s_lastStatePublishMs = now;
    }
}
