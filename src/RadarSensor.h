#pragma once
#include <Arduino.h>
#include "RadarTypes.h"

// Wrapper um die ld2410-Lib (ncmreynolds/ld2410) fuer den HLK-LD2410C.
// Der Radar laeuft ueber die Hardware-UART0 des ESP8266 mit 256000 Baud.
// Serial.swap() legt UART0 auf D7/GPIO13 (RX) und D8/GPIO15 (TX).
//
// UART1 (Serial1, nur TX) bleibt fuer Debug-Ausgaben reserviert.
//
// Liefert zusaetzlich ein "gehaltenes" Praesenz-Signal (presenceHeld()),
// das nach dem letzten erkannten Ziel noch RADAR_HOLD_MS lang true bleibt,
// um Geflacker bei kurzen Aussetzern des Sensors zu vermeiden.
namespace RadarSensor {

void begin();
void loop();

// Rohes Sensor-Ergebnis (bewegtes oder stehendes Ziel erkannt)
bool presenceDetected();

// Presence inkl. Hold-Timer -> das sollte die Anwendungslogik verwenden
bool presenceHeld();

// ---- Fein-Tuning / Telemetrie (fuer RADAR_DEBUG_TELEMETRY) ----

bool movingTargetDetected();
uint16_t movingTargetDistanceCm();
uint8_t movingTargetEnergy();

bool stationaryTargetDetected();
uint16_t stationaryTargetDistanceCm();
uint8_t stationaryTargetEnergy();

// Aktuell konfigurierte Gates/Sensitivitaeten (einmal bei begin() und nach
// jeder applyConfig() per requestCurrentConfiguration() aktualisiert).
uint8_t maxGate();
uint8_t maxMovingGate();
uint8_t maxStationaryGate();
uint8_t motionSensitivity(uint8_t gate);
uint8_t stationarySensitivity(uint8_t gate);

// Wendet ein per MQTT empfangenes radar_config-Kommando an
// (setMaxValues/setGateSensitivity ueber die ld2410-Lib).
void applyConfig(const RadarConfigRequest& config);

} // namespace RadarSensor
