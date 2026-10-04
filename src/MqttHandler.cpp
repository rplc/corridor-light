#include "MqttHandler.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

namespace {

WiFiClient s_wifiClient;
PubSubClient s_mqttClient(s_wifiClient);

bool s_radarArmed = false;
bool s_switchArmed = false;
MqttHandler::Mode s_mode = MqttHandler::Mode::Off;

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

    Serial.printf("[MQTT] Kommando: radar_armed=%d switch_armed=%d mode=%s\n",
                  s_radarArmed, s_switchArmed, modeToString(s_mode));
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

void publishState(bool ledOn, bool radarPresence, bool switchOpen) {
    if (!s_mqttClient.connected()) {
        return;
    }

    JsonDocument doc;
    doc["led_on"] = ledOn;
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

} // namespace MqttHandler
