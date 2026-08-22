#pragma once
#include <Arduino.h>

// Steuert das Gate des IRLB8721 per PWM, inkl. weichem Ein-/Ausblenden.
namespace LedController {

void begin();

// Zielzustand setzen (an/aus). Das eigentliche Faden passiert in loop().
void setOn(bool on);

bool isOn();

void loop();

} // namespace LedController
