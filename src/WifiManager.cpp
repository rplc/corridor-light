#include "WifiManager.h"
#include <ESP8266WiFi.h>

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

    Serial.printf("[WiFi] Verbinde mit \"%s\"", WIFI_SSID);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Verbunden, IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] Kein Connect innerhalb 15s, loop() versucht es weiter.");
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
    Serial.println("[WiFi] Nicht verbunden, versuche Reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

} // namespace WifiManager
