#pragma once
#include <Arduino.h>

// Verbindet mit dem MQTT-Broker. Kommando und Status laufen ueber dasselbe
// Topic (MQTT_TOPIC): der ESP ist darauf subscribed UND published seinen
// eigenen Status dorthin (retained).
//
// Erwartetes/publiziertes JSON:
//   { "radar_armed": true, "switch_armed": false, "mode": "auto",
//     "led_on": false, "radar_presence": false, "switch_open": true, "ts": 123 }
// mode: "auto" | "on" | "off"
//
// Da der ESP sein eigenes Topic subscribed, bekommt er auch seinen eigenen
// Status-Publish wieder als Nachricht zurueck. Das ist harmlos: radar_armed/
// switch_armed/mode werden dabei auf genau denselben Wert "gesetzt", den sie
// schon hatten (idempotent) - zusaetzliche Felder wie led_on werden beim
// Parsen schlicht ignoriert.
namespace MqttHandler {

enum class Mode : uint8_t { Auto, On, Off };

void begin();
void loop();

bool isConnected();

bool radarArmed();
bool switchArmed();
Mode mode();

// Aktuellen Zustand (u.a. ob der Strip gerade an ist) auf MQTT_TOPIC
// publishen (retained). Wird von main.cpp nur bei Aenderung aufgerufen.
void publishState(bool ledOn, bool radarPresence, bool switchOpen);

} // namespace MqttHandler
