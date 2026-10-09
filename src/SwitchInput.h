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
// Die Anwendungslogik verwendet den LOW-Pegel als Trigger. Ein Kabelbruch
// ergibt HIGH und loest damit kein Licht aus. Den Reed-Kontakt so einsetzen,
// dass der Kontakt im gewuenschten Triggerzustand nach GND schliesst.
namespace SwitchInput {

void begin();
void loop();

// true, wenn der NC-Eingang (entprellt) aktiv LOW ist -> Licht-Trigger
bool isTriggered();

} // namespace SwitchInput
