#pragma once
#include <Arduino.h>

// Verbindet mit dem WLAN und haelt die Verbindung dauerhaft aufrecht.
// SSID/Passwort kommen als Build-Flags aus platformio.local.ini.
namespace WifiManager {

void begin();

// In loop() aufrufen: kuemmert sich um Reconnect, falls die Verbindung
// mal weg ist (z.B. nach Router-Neustart).
void loop();

bool isConnected();

} // namespace WifiManager
