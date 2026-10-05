#include "OtaManager.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "DebugLog.h"

namespace OtaManager {

void begin() {
    ArduinoOTA.setHostname(MQTT_CLIENT_ID);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    // Waehrend eines laufenden Updates bewusst NUR auf Serial loggen (nicht
    // ueber DebugLog/MQTT) - zusaetzliche Netzwerk-I/O waehrend des
    // Flash-Schreibens wollen wir nicht riskieren.
    ArduinoOTA.onStart([]() {
        Serial.println("[OTA] Update startet...");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("[OTA] Update fertig, Neustart.");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Fortschritt: %u%%\r", (progress * 100) / total);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Fehler[%u]\n", error);
    });

    ArduinoOTA.begin();
    DebugLog::logf("[OTA] Bereit.");
}

void loop() {
    ArduinoOTA.handle();
}

} // namespace OtaManager
