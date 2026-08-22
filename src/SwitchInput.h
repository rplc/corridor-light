#pragma once

// Entprellter Schalter-Input. Verwendet den internen Pullup des ESP8266,
// keine externen Widerstaende noetig. Schalter liegt zwischen GPIO und GND:
//   offen   -> Pin liest HIGH
//   geschlossen -> Pin liest LOW
namespace SwitchInput {

void begin();
void loop();

// true, wenn der Schalter (entprellt) OFFEN ist -> soll den Strip aktivieren
bool isOpen();

} // namespace SwitchInput
