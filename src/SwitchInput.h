#pragma once

// Entprellter Schalter-Input. Verwendet den internen Pullup des ESP8266,
// keine externen Widerstaende noetig.
//
// Verkabelung (SPDT-Schalter, siehe wiring-diagram.svg):
//   C (Common)  -> GND
//   N.O. (Normally Open) -> PIN_SWITCH (D2)
// Daraus folgt:
//   Schalter offen       -> Kontakt zu GND unterbrochen -> Pin liest HIGH
//   Schalter geschlossen  -> N.O. verbindet mit C/GND    -> Pin liest LOW
//
// Wichtig: Haengt stattdessen N.C. (Normally Closed) an PIN_SWITCH, kehrt
// sich die Logik um (offen=LOW) - dann muesste isOpen() invertiert werden.
namespace SwitchInput {

void begin();
void loop();

// true, wenn der Schalter (entprellt) OFFEN ist -> soll den Strip aktivieren
bool isOpen();

} // namespace SwitchInput
