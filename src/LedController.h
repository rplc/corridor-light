#pragma once
#include <Arduino.h>

// Steuert das Gate des IRLB8721 per PWM, inkl. weichem Ein-/Ausblenden.
namespace LedController {

void begin();

// Zielzustand setzen (an/aus). Das eigentliche Faden passiert in loop().
void setOn(bool on);

// Zielhelligkeit in Prozent (0-100), gilt fuer den "an"-Zustand.
void setBrightness(uint8_t percent);

bool isOn();
uint8_t brightness();

void loop();

} // namespace LedController
