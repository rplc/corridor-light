#pragma once

// Wrapper um die ld2410-Lib fuer den HLK-LD2410C.
// Laeuft ueber SoftwareSerial, damit die Hardware-UART (Serial) frei
// bleibt fuer Debug-Logs.
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

} // namespace RadarSensor
