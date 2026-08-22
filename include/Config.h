#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pin-Belegung (Wemos D1 Mini / ESP8266)
//
// Bewusst NUR "sichere" GPIOs verwendet (keine Boot-Strapping-Pins
// D3/GPIO0, D4/GPIO2, D8/GPIO15, kein D0/GPIO16, kein RX/TX-Pin,
// da Serial fuer Debug-Logs frei bleiben soll).
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_LED_GATE   = 5;   // D1 -> Gate IRLB8721 (ueber 220R)
constexpr uint8_t PIN_SWITCH     = 4;   // D2 -> Schalter gegen GND, INPUT_PULLUP
constexpr uint8_t PIN_RADAR_RX   = 14;  // D5 -> ESP RX, verbunden mit Radar TX
constexpr uint8_t PIN_RADAR_TX   = 12;  // D6 -> ESP TX, verbunden mit Radar RX

// ---------------------------------------------------------------------------
// MQTT
// ---------------------------------------------------------------------------
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

constexpr const char* MQTT_CLIENT_ID    = "esp8266-ledstrip1";
constexpr const char* MQTT_TOPIC_SET    = "home/ledstrip1/set";
constexpr const char* MQTT_TOPIC_STATE  = "home/ledstrip1/state";

// ---------------------------------------------------------------------------
// Verhalten
// ---------------------------------------------------------------------------
// Wie lange der Strip nach dem letzten Radar-Trigger noch "an" haelt,
// bevor er (im auto-Modus) wieder ausgeht. Verhindert Geflacker, wenn der
// Sensor kurz keine Praesenz mehr meldet.
constexpr uint32_t RADAR_HOLD_MS       = 5000;

// Software-Entprellung fuer den Schalter
constexpr uint32_t SWITCH_DEBOUNCE_MS  = 50;

// Weiches Ein-/Ausblenden des LED-Streifens (PWM), 0 = hart schalten
constexpr uint16_t LED_FADE_MS         = 400;
constexpr uint8_t  LED_PWM_MAX         = 255;

// LD2410 UART-Baudrate (Werksvorgabe des Moduls)
constexpr uint32_t RADAR_BAUD          = 256000;

// Wie oft der State-Topic gepublished wird, auch ohne Aenderung (Keepalive
// fuer ioBroker o.ae., verhindert "stale" Werte)
constexpr uint32_t STATE_PUBLISH_INTERVAL_MS = 30000;
