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
bool s_lastPublishedPresence = false;
bool s_lastPublishedSwitchOpen = false;
bool s_havePublished = false;

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

    bool ledOn = LedController::isOn();
    bool presence = RadarSensor::presenceHeld();
    bool switchOpen = SwitchInput::isOpen();

    bool changed = !s_havePublished
                   || ledOn != s_lastPublishedLedOn
                   || presence != s_lastPublishedPresence
                   || switchOpen != s_lastPublishedSwitchOpen;

    if (MqttHandler::isConnected() && changed) {
        MqttHandler::publishState(ledOn, presence, switchOpen);
        s_lastPublishedLedOn = ledOn;
        s_lastPublishedPresence = presence;
        s_lastPublishedSwitchOpen = switchOpen;
        s_havePublished = true;
    }
}
