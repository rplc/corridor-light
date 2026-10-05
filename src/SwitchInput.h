#pragma once

// Entprellter Schalter-Input. Verwendet den internen Pullup des ESP8266,
// keine externen Widerstaende noetig.
//
// Verkabelung (SPDT/Reed-Wechsler):
//   C (Common)  -> GND
//   N.C. (Normally Closed) -> PIN_SWITCH (D5)
// Daraus folgt:
//   Kontakt geschlossen -> NC verbindet mit C/GND -> Pin liest LOW
//   Kontakt offen       -> NC trennt von GND       -> Pin liest HIGH
//
// Damit bleibt isOpen() = HIGH und die Anwendungslogik unveraendert.
namespace SwitchInput {

void begin();
void loop();

// true, wenn der Schalter (entprellt) OFFEN ist -> soll den Strip aktivieren
bool isOpen();

} // namespace SwitchInput
