#pragma once
#include <Arduino.h>

// Verbindet mit dem MQTT-Broker, empfaengt das JSON-Steuerkommando auf
// MQTT_TOPIC_SET und haelt den zuletzt empfangenen Steuerzustand vor.
//
// Erwartetes JSON auf MQTT_TOPIC_SET:
//   { "radar_armed": true, "switch_armed": false, "mode": "auto" }
// mode: "auto" | "on" | "off"
namespace MqttHandler {

enum class Mode : uint8_t { Auto, On, Off };

void begin();
void loop();

bool isConnected();

bool radarArmed();
bool switchArmed();
Mode mode();

// Aktuellen Zustand (u.a. ob der Strip gerade an ist) auf MQTT_TOPIC_STATE
// publishen (retained). Wird von main.cpp bei Aenderung + periodisch aufgerufen.
void publishState(bool ledOn, bool radarPresence, bool switchOpen);

} // namespace MqttHandler
