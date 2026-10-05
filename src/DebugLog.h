#pragma once
#include <Arduino.h>

// Printf-artiger Logger: schreibt IMMER (falls vorhanden) auf Serial1 (UART1/D4) UND
// puffert dieselbe Zeile zusaetzlich in einem kleinen Ringpuffer, damit
// main.cpp sie bei bestehender MQTT-Verbindung zusaetzlich published
// (siehe MqttHandler::publishLog / MQTT_TOPIC_LOG).
//
// Hintergrund: Wenn Radar + ESP zusammen zu viel Strom fuer einen
// schwachen USB-Port ziehen, ist waehrend des Betriebs kein Serial-Monitor
// nutzbar - der MQTT-Kanal ist dann der einzige Weg, Boot-/Laufzeit-Logs
// (z.B. "LD2410 NICHT gefunden") ueberhaupt zu sehen.
//
// Der Puffer ist bewusst klein und FIFO-begrenzt (aelteste Zeilen fallen
// raus, wenn er voll laeuft, z.B. weil WLAN/MQTT laenger nicht verbunden
// sind) - das ist ein Live-Debug-Werkzeug, kein vollstaendiges Log.
namespace DebugLog {

void begin();

void logf(const char* fmt, ...);

// Liefert true und kopiert die seit dem letzten Aufruf gepufferten
// Log-Zeilen (nullterminiert) nach `out`, wenn welche vorhanden sind.
// Leert danach den Puffer. `out` sollte mindestens so gross sein wie der
// interne Puffer (siehe DebugLog.cpp), sonst wird abgeschnitten.
bool consumePending(char* out, size_t outSize);

} // namespace DebugLog
