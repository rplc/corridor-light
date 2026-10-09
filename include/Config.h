#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pin-Belegung (Wemos D1 Mini / ESP8266)
//
// UART0 wird fuer den LD2410 verwendet und per Serial.swap() auf
// GPIO13/GPIO15 gelegt:
//   D7/GPIO13 = UART0 RX <- Radar TX
//   D8/GPIO15 = UART0 TX -> Radar RX
//
// D8/GPIO15 ist ein Boot-Strapping-Pin. Der Radar-RX-Eingang ist hochohmig,
// daher zieht er GPIO15 beim Reset nicht hoch. Der Wemos benoetigt fuer
// normalen Flash-Boot GPIO15 LOW.
//
// Der Debug-Output laeuft separat ueber UART1 (Serial1) auf D4/GPIO2.
// Der Schalter wurde deshalb von D2 auf D5 verschoben.
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_LED_GATE   = 5;   // D1 -> Gate IRLB8721 (ueber 220R)
constexpr uint8_t PIN_SWITCH     = 14;  // D5 -> Reed COM/NC, INPUT_PULLUP
constexpr uint8_t PIN_RADAR_RX   = 13;  // D7/GPIO13 -> UART0 RX <- Radar TX
constexpr uint8_t PIN_RADAR_TX   = 15;  // D8/GPIO15 -> UART0 TX -> Radar RX
constexpr uint8_t PIN_DEBUG_TX   = 2;   // D4/GPIO2 -> UART1 TX (optional)

// ---------------------------------------------------------------------------
// MQTT
// ---------------------------------------------------------------------------
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

constexpr const char* MQTT_CLIENT_ID = "hallway-light";
constexpr const char* MQTT_TOPIC     = "hallway-light";

// Live-Log-Kanal (siehe DebugLog) - Ersatz fuer den Serial-Monitor, wenn
// ESP+Radar zusammen zu viel Strom fuer einen schwachen USB-Port ziehen.
// Nicht retained, reiner Stream.
constexpr const char* MQTT_TOPIC_LOG = "hallway-light/log";

// ---------------------------------------------------------------------------
// Verhalten
// ---------------------------------------------------------------------------
// Wie lange der Strip nach dem letzten Radar-Trigger noch "an" haelt,
// bevor er (im auto-Modus) wieder ausgeht. Verhindert Geflacker, wenn der
// Sensor kurz keine Praesenz mehr meldet.
constexpr uint32_t DEFAULT_RADAR_TIMEOUT_MS = 5000;

// Software-Entprellung fuer den Schalter
constexpr uint32_t SWITCH_DEBOUNCE_MS  = 50;

// Weiches Ein-/Ausblenden des LED-Streifens (PWM), 0 = hart schalten
constexpr uint16_t LED_FADE_MS         = 2000;
constexpr uint8_t  LED_PWM_MAX         = 255;

// Default-Helligkeiten (0-100%) fuer die drei Ausloeser.
constexpr uint8_t DEFAULT_BRIGHTNESS_FORCED_ON_PCT = 100;
constexpr uint8_t DEFAULT_BRIGHTNESS_RADAR_PCT = 100;
constexpr uint8_t DEFAULT_BRIGHTNESS_SWITCH_PCT = 100;

// LD2410 UART-Baudrate (Werksvorgabe des Moduls)
constexpr uint32_t RADAR_BAUD          = 256000;

// Debug-UART (UART1, nur TX auf D4/GPIO2). Die normalen Serial-Logs
// werden dorthin umgeleitet, weil UART0 exklusiv fuer das Radar arbeitet.
constexpr uint32_t DEBUG_BAUD          = 115200;

// ---------------------------------------------------------------------------
// Radar-Tuning / Debug-Telemetrie
//
// RADAR_DEBUG_TELEMETRY schaltet das zusaetzliche Publishen der rohen
// Radar-Messwerte (Distanz/Energie pro Ziel, aktuelle Gate-Sensitivitaeten)
// auf einem eigenen Debug-Topic frei. Standardmaessig AUS (0), damit im
// Normalbetrieb kein unnoetiger MQTT-Traffic entsteht. Zum Fein-Tunen per
// Build-Flag aktivieren, siehe platformio.ini env "d1_mini_radar_tuning"
// (-D RADAR_DEBUG_TELEMETRY=1).
// ---------------------------------------------------------------------------
#ifndef RADAR_DEBUG_TELEMETRY
#define RADAR_DEBUG_TELEMETRY 0
#endif

// MQTT_DEBUG_LOG schaltet das Weiterleiten der normalen Debug-/Log-Meldungen
// (DebugLog) an MQTT_TOPIC_LOG frei. Die Ausgabe ueber Serial1 bleibt davon
// unberuehrt. Standardmaessig AUS (0), damit im Produktivbetrieb kein
// zusaetzlicher MQTT-Traffic und kein Log-Puffer benoetigt wird.
// Zum Debuggen per Build-Flag aktivieren:
//   -D MQTT_DEBUG_LOG=1
#ifndef MQTT_DEBUG_LOG
#define MQTT_DEBUG_LOG 0
#endif

constexpr const char* MQTT_TOPIC_RADAR_DEBUG   = "hallway-light/radar-debug";
constexpr uint32_t    RADAR_TELEMETRY_INTERVAL_MS = 1000;
