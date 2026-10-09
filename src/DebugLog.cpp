#include "DebugLog.h"
#include <stdarg.h>
#include <string.h>

namespace {
constexpr size_t BUFFER_SIZE = 700; // passt bequem in den 1024B-MQTT-Puffer
char s_buffer[BUFFER_SIZE];
size_t s_bufferLen = 0;

// Haengt `line` (ohne Newline) an den Puffer an. Ist nicht genug Platz,
// werden von vorne ganze (aeltere) Zeilen entfernt, bis es passt - FIFO.
void appendLine(const char* line, size_t lineLen) {
    if (lineLen >= BUFFER_SIZE) {
        // Einzelne Zeile laenger als der ganze Puffer: abschneiden.
        lineLen = BUFFER_SIZE - 1;
    }
    size_t needed = lineLen + 1; // +1 fuer das trennende '\n'

    if (s_bufferLen + needed > BUFFER_SIZE) {
        size_t toDrop = s_bufferLen + needed - BUFFER_SIZE;
        size_t dropPos = toDrop;
        while (dropPos < s_bufferLen && s_buffer[dropPos] != '\n') {
            dropPos++;
        }
        if (dropPos < s_bufferLen) {
            dropPos++; // Newline selbst mit entfernen
        } else {
            dropPos = s_bufferLen; // keine Newline gefunden -> alles verwerfen
        }
        size_t remaining = s_bufferLen - dropPos;
        memmove(s_buffer, s_buffer + dropPos, remaining);
        s_bufferLen = remaining;
    }

    memcpy(s_buffer + s_bufferLen, line, lineLen);
    s_bufferLen += lineLen;
    s_buffer[s_bufferLen++] = '\n';
}
} // namespace

namespace DebugLog {

void begin() {
    s_bufferLen = 0;
}

void logf(const char* fmt, ...) {
    char line[160];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    if (written < 0) {
        return;
    }
    size_t lineLen = static_cast<size_t>(written);
    if (lineLen >= sizeof(line)) {
        lineLen = sizeof(line) - 1; // vsnprintf hat schon abgeschnitten
    }

    Serial1.println(line);
#if MQTT_DEBUG_LOG
    appendLine(line, lineLen);
#endif
}

bool consumePending(char* out, size_t outSize) {
    if (s_bufferLen == 0 || outSize == 0) {
        return false;
    }
    size_t copyLen = s_bufferLen < outSize - 1 ? s_bufferLen : outSize - 1;
    memcpy(out, s_buffer, copyLen);
    out[copyLen] = '\0';
    s_bufferLen = 0;
    return true;
}

} // namespace DebugLog
