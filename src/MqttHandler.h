#pragma once
#include <Arduino.h>
#include "RadarTypes.h"
#include "Config.h" // fuer RADAR_DEBUG_TELEMETRY

// Verbindet mit dem MQTT-Broker. Kommando und Status laufen ueber dasselbe
// Topic (MQTT_TOPIC): der ESP ist darauf subscribed UND published seinen
// eigenen Status dorthin (retained).
//
// Erwartetes Kommando-JSON (alle Felder optional, letzter bekannter Wert
// bleibt erhalten, falls ein Feld fehlt):
//   {
//     "radar_armed": true, "switch_armed": false, "mode": "auto",
//     "brightness": 100,
//     "radar_config": {
//       "max_moving_gate": 4, "max_stationary_gate": 4, "timeout_s": 5,
//       "gate_sensitivity": [ {"gate": 2, "moving": 60, "stationary": 50} ]
//     }
//   }
// mode: "auto" | "on" | "off"
// brightness: 0-100 (%), gilt fuer den "an"-Zustand, egal ob durch mode=on
//             oder durch Radar/Switch im auto-Modus ausgeloest
// radar_config: optional, stoesst eine einmalige Rekonfiguration des
//             LD2410 an (siehe RadarSensor::applyConfig) - wird nicht
//             dauerhaft gespeichert, sondern direkt an den Sensor
//             durchgereicht
//
// Publiziertes Status-JSON (gleiches Topic, retained):
//   { ...obiges plus... "led_on": false, "radar_presence": false,
//     "switch_open": true, "ts": 123 }
//
// Da der ESP sein eigenes Topic subscribed, bekommt er auch seinen eigenen
// Status-Publish wieder als Nachricht zurueck. Das ist harmlos: radar_armed/
// switch_armed/mode/brightness werden dabei auf genau denselben Wert
// "gesetzt", den sie schon hatten (idempotent) - zusaetzliche Felder wie
// led_on werden beim Parsen schlicht ignoriert, und radar_config ist im
// Status-Publish nicht enthalten (kein Loop-Risiko dort).
namespace MqttHandler {

enum class Mode : uint8_t { Auto, On, Off };

void begin();
void loop();

bool isConnected();

bool radarArmed();
bool switchArmed();
Mode mode();
uint8_t brightness();

// Liefert true und befuellt `out`, wenn seit dem letzten Aufruf ein
// radar_config-Kommando empfangen wurde (einmalig abzuholen - main.cpp
// sollte das jeden loop()-Durchlauf pruefen und bei true an
// RadarSensor::applyConfig() weiterreichen).
bool consumeRadarConfigRequest(RadarConfigRequest& out);

// Aktuellen Zustand (u.a. ob der Strip gerade an ist) auf MQTT_TOPIC
// publishen (retained). Wird von main.cpp nur bei Aenderung aufgerufen,
// kein periodisches Keepalive.
void publishState(bool ledOn, bool radarPresence, bool switchOpen);

// Published einen Log-Text (siehe DebugLog) auf MQTT_TOPIC_LOG, NICHT
// retained (reiner Live-Stream, kein Zustand). Wird von main.cpp
// aufgerufen, sobald DebugLog::consumePending() etwas liefert.
void publishLog(const char* text);

#if RADAR_DEBUG_TELEMETRY
// Nur vorhanden, wenn das Build-Flag RADAR_DEBUG_TELEMETRY=1 gesetzt ist.
// Published die rohen Radar-Messwerte auf MQTT_TOPIC_RADAR_DEBUG, damit
// man beim Fein-Tunen (Sensitivitaet/Gates) live mitlesen kann.
void publishRadarDebug();
#endif

} // namespace MqttHandler
