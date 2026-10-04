#pragma once
#include <Arduino.h>
#include "RadarTypes.h"

// Wrapper um die ld2410-Lib (ncmreynolds/ld2410) fuer den HLK-LD2410C.
// Laeuft ueber SoftwareSerial, damit die Hardware-UART (Serial) frei
// bleibt fuer Debug-Logs.
//
// Hinweis: Die Lib empfiehlt eigentlich eine Hardware-UART, da der LD2410
// mit 256000 Baud kommuniziert - SoftwareSerial ist bei der Baudrate
// weniger robust. Fuer reine Praesenzerkennung (normaler Betrieb) hat sich
// das in der Praxis als ausreichend erwiesen; bei den synchronen
// Konfigurationsbefehlen (setMaxValues/setGateSensitivity) kann es im
// Zweifel zu einem Timeout kommen - die Rueckgabewerte/Logs zeigen das an,
// einfach erneut senden.
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
