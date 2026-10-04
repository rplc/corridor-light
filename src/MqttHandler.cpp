#include "MqttHandler.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#if RADAR_DEBUG_TELEMETRY
#include "RadarSensor.h"
#endif

namespace {

WiFiClient s_wifiClient;
PubSubClient s_mqttClient(s_wifiClient);

bool s_radarArmed = false;
bool s_switchArmed = false;
MqttHandler::Mode s_mode = MqttHandler::Mode::Off;
uint8_t s_brightnessPercent = DEFAULT_BRIGHTNESS_PCT;

bool s_hasPendingRadarConfig = false;
RadarConfigRequest s_pendingRadarConfig;

uint32_t s_lastReconnectAttempt = 0;
constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;

MqttHandler::Mode parseMode(const char* raw) {
    if (raw == nullptr) return MqttHandler::Mode::Off;
    if (strcmp(raw, "on") == 0) return MqttHandler::Mode::On;
    if (strcmp(raw, "auto") == 0) return MqttHandler::Mode::Auto;
    return MqttHandler::Mode::Off;
}

const char* modeToString(MqttHandler::Mode m) {
    switch (m) {
        case MqttHandler::Mode::On:   return "on";
        case MqttHandler::Mode::Auto: return "auto";
        default:                      return "off";
    }
}

uint8_t clampPercent(long v) {
    if (v < 0) return 0;
    if (v > 100) return 100;
    return static_cast<uint8_t>(v);
}

// Parst ein optionales "radar_config"-Objekt. Gibt true zurueck, wenn eines
// vorhanden war (auch wenn es inhaltlich leer ist).
bool parseRadarConfig(JsonVariantConst radarConfigJson, RadarConfigRequest& out) {
    if (radarConfigJson.isNull() || !radarConfigJson.is<JsonObjectConst>()) {
        return false;
    }

    out = RadarConfigRequest{}; // frisch befuellen

    if (radarConfigJson["max_moving_gate"].is<int>() && radarConfigJson["max_stationary_gate"].is<int>()) {
        out.hasMaxValues = true;
        out.maxMovingGate = radarConfigJson["max_moving_gate"].as<uint8_t>();
        out.maxStationaryGate = radarConfigJson["max_stationary_gate"].as<uint8_t>();
        out.timeoutSeconds = radarConfigJson["timeout_s"].is<int>()
                                  ? radarConfigJson["timeout_s"].as<uint16_t>()
                                  : 5;
    }

    JsonArrayConst gates = radarConfigJson["gate_sensitivity"].as<JsonArrayConst>();
    for (JsonVariantConst gateEntry : gates) {
        if (out.gateSensitivityCount >= RADAR_MAX_GATE_COUNT) break;
        JsonObjectConst g = gateEntry.as<JsonObjectConst>();
        if (!g["gate"].is<int>()) continue;
        RadarGateSensitivity entry;
        entry.gate = g["gate"].as<uint8_t>();
        entry.moving = g["moving"].is<int>() ? g["moving"].as<uint8_t>() : 0;
        entry.stationary = g["stationary"].is<int>() ? g["stationary"].as<uint8_t>() : 0;
        out.gateSensitivity[out.gateSensitivityCount++] = entry;
    }

    return true;
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    (void)topic;
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
        Serial.printf("[MQTT] JSON-Parsefehler: %s\n", err.c_str());
        return;
    }

    if (doc["radar_armed"].is<bool>()) {
        s_radarArmed = doc["radar_armed"].as<bool>();
    }
    if (doc["switch_armed"].is<bool>()) {
        s_switchArmed = doc["switch_armed"].as<bool>();
    }
    if (doc["mode"].is<const char*>()) {
        s_mode = parseMode(doc["mode"].as<const char*>());
    }
    if (doc["brightness"].is<int>()) {
        s_brightnessPercent = clampPercent(doc["brightness"].as<long>());
    }

    if (parseRadarConfig(doc["radar_config"], s_pendingRadarConfig)) {
        s_hasPendingRadarConfig = true;
        Serial.println("[MQTT] radar_config empfangen, wird an RadarSensor weitergereicht.");
    }

    Serial.printf("[MQTT] Kommando: radar_armed=%d switch_armed=%d mode=%s brightness=%u%%\n",
                  s_radarArmed, s_switchArmed, modeToString(s_mode), s_brightnessPercent);
}

bool tryConnect() {
    Serial.print("[MQTT] Verbinde zum Broker...");
    bool ok;
    if (strlen(MQTT_USER) > 0) {
        ok = s_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
    } else {
        ok = s_mqttClient.connect(MQTT_CLIENT_ID);
    }

    if (ok) {
        Serial.println(" verbunden.");
        s_mqttClient.subscribe(MQTT_TOPIC);
    } else {
        Serial.printf(" fehlgeschlagen, rc=%d\n", s_mqttClient.state());
    }
    return ok;
}

} // namespace

namespace MqttHandler {

void begin() {
    s_mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    s_mqttClient.setBufferSize(1024); // radar_config/radar-debug mit gate_sensitivity-Array braucht mehr als die 256B-Default-Puffergroesse
    s_mqttClient.setCallback(onMqttMessage);
}

void loop() {
    if (!s_mqttClient.connected()) {
        uint32_t now = millis();
        if (now - s_lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
            s_lastReconnectAttempt = now;
            tryConnect();
        }
        return;
    }
    s_mqttClient.loop();
}

bool isConnected() {
    return s_mqttClient.connected();
}

bool radarArmed() { return s_radarArmed; }
bool switchArmed() { return s_switchArmed; }
Mode mode() { return s_mode; }
uint8_t brightness() { return s_brightnessPercent; }

bool consumeRadarConfigRequest(RadarConfigRequest& out) {
    if (!s_hasPendingRadarConfig) {
        return false;
    }
    out = s_pendingRadarConfig;
    s_hasPendingRadarConfig = false;
    return true;
}

void publishState(bool ledOn, bool radarPresence, bool switchOpen) {
    if (!s_mqttClient.connected()) {
        return;
    }

    JsonDocument doc;
    doc["led_on"] = ledOn;
    doc["brightness"] = s_brightnessPercent;
    doc["radar_armed"] = s_radarArmed;
    doc["radar_presence"] = radarPresence;
    doc["switch_armed"] = s_switchArmed;
    doc["switch_open"] = switchOpen;
    doc["mode"] = modeToString(s_mode);
    doc["ts"] = millis();

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    s_mqttClient.publish(MQTT_TOPIC, reinterpret_cast<const uint8_t*>(buffer), n, true);
}

#if RADAR_DEBUG_TELEMETRY
void publishRadarDebug() {
    if (!s_mqttClient.connected()) {
        return;
    }

    JsonDocument doc;
    doc["presence"] = RadarSensor::presenceDetected();

    JsonObject moving = doc["moving"].to<JsonObject>();
    moving["detected"] = RadarSensor::movingTargetDetected();
    moving["distance_cm"] = RadarSensor::movingTargetDistanceCm();
    moving["energy"] = RadarSensor::movingTargetEnergy();

    JsonObject stationary = doc["stationary"].to<JsonObject>();
    stationary["detected"] = RadarSensor::stationaryTargetDetected();
    stationary["distance_cm"] = RadarSensor::stationaryTargetDistanceCm();
    stationary["energy"] = RadarSensor::stationaryTargetEnergy();

    uint8_t maxGate = RadarSensor::maxGate();
    doc["max_gate"] = maxGate;
    doc["max_moving_gate"] = RadarSensor::maxMovingGate();
    doc["max_stationary_gate"] = RadarSensor::maxStationaryGate();

    JsonArray gates = doc["gate_sensitivity"].to<JsonArray>();
    for (uint8_t gate = 0; gate <= maxGate && gate < RADAR_MAX_GATE_COUNT; gate++) {
        JsonObject g = gates.add<JsonObject>();
        g["gate"] = gate;
        g["moving"] = RadarSensor::motionSensitivity(gate);
        g["stationary"] = RadarSensor::stationarySensitivity(gate);
    }

    doc["ts"] = millis();

    char buffer[1024];
    size_t n = serializeJson(doc, buffer);
    s_mqttClient.publish(MQTT_TOPIC_RADAR_DEBUG, reinterpret_cast<const uint8_t*>(buffer), n, false);
}
#endif

} // namespace MqttHandler
