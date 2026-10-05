#include "WifiManager.h"
#include <ESP8266WiFi.h>
#include "DebugLog.h"

namespace {
constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
uint32_t s_lastAttempt = 0;
} // namespace

namespace WifiManager {

void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial1.printf("[WiFi] Verbinde mit \"%s\"", WIFI_SSID);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(250);
        Serial1.print(".");
    }
    Serial1.println();

    if (WiFi.status() == WL_CONNECTED) {
        DebugLog::logf("[WiFi] Verbunden, IP: %s", WiFi.localIP().toString().c_str());
    } else {
        DebugLog::logf("[WiFi] Kein Connect innerhalb 15s, loop() versucht es weiter.");
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
    uint32_t now = millis();
    if (now - s_lastAttempt < RECONNECT_INTERVAL_MS) {
        return;
    }
    s_lastAttempt = now;
    DebugLog::logf("[WiFi] Nicht verbunden, versuche Reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

} // namespace WifiManager
