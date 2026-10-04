#pragma once
#include <Arduino.h>

// Der LD2410 hat je nach Variante bis zu 9 Gates (0-8), je ca. 0,75m.
constexpr uint8_t RADAR_MAX_GATE_COUNT = 9;

struct RadarGateSensitivity {
    uint8_t gate = 0;
    uint8_t moving = 0;      // 0-100
    uint8_t stationary = 0;  // 0-100
};

// Von MqttHandler aus einem "radar_config"-Objekt befuellt, von main.cpp
// konsumiert und an RadarSensor::applyConfig() weitergereicht.
struct RadarConfigRequest {
    bool hasMaxValues = false;
    uint8_t maxMovingGate = 0;
    uint8_t maxStationaryGate = 0;
    uint16_t timeoutSeconds = 0;

    uint8_t gateSensitivityCount = 0;
    RadarGateSensitivity gateSensitivity[RADAR_MAX_GATE_COUNT];
};
