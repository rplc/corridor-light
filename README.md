# ESP8266 12V LED-Strip-Controller

Steuert einen 12V-LED-Strip über einen IRLB8721-MOSFET, aktiviert durch
HLK-LD2410C-Radar und/oder einen Schalter, ferngesteuert per MQTT/JSON.
OTA-fähig, dauerhaft im WLAN.

## Setup

1. `platformio.local.ini.example` nach `platformio.local.ini` kopieren und
   ausfüllen (WLAN, OTA-Passwort, MQTT-Server/User/Passwort, `ota_host` für
   spätere OTA-Uploads).
2. Erstes Flashen per USB: `pio run -e d1_mini -t upload`
3. Danach kabellos: `pio run -e d1_mini_ota -t upload`

## Pinbelegung (Wemos D1 Mini)

| Pin        | GPIO | Funktion                          |
|------------|------|------------------------------------|
| D1         | 5    | Gate IRLB8721 (über 220R)          |
| D2         | 4    | Schalter (gegen GND, INPUT_PULLUP) |
| D5         | 14   | SoftwareSerial RX ← Radar TX       |
| D6         | 12   | SoftwareSerial TX → Radar RX       |
| 5V         | –    | von UBEC (12V→5V)                  |
| GND        | –    | gemeinsame Masse mit PSU/Radar/MOSFET |

Bewusst **keine** Boot-Strapping-Pins (D3/GPIO0, D4/GPIO2, D8/GPIO15) und
kein D0/GPIO16 verwendet, RX/TX bleiben für den Debug-Serial-Monitor frei.

## Deine Hardware-Fragen

**IRLB8721 – braucht es noch mehr als ESP-GPIO → Gate?**

Der IRLB8721 ist ein Logic-Level-MOSFET mit niedriger Schwellspannung, er
schaltet auch mit 3,3V Gate-Spannung sauber durch – ein Levelshifter ist
für reines An/Aus (bzw. langsames Fade wie hier) **nicht nötig**. Zwei
Dinge solltest du trotzdem einbauen (im Schema enthalten):

- **220Ω Gate-Widerstand** in Reihe zwischen GPIO und Gate: begrenzt den
  Ladestrom der Gate-Kapazität und dämpft Ringing beim Schalten.
- **10kΩ Pulldown** vom Gate nach GND: hält den MOSFET sicher aus, solange
  der ESP bootet/resettet und der GPIO kurz hochohmig/undefiniert ist –
  sonst kann der Strip beim Einschalten kurz aufflackern.

Bei sehr langen/starken Strips (hoher Dauerstrom) lohnt es, den Rds(on)
des IRLB8721 bei 3,3V Gate-Spannung im Datenblatt zu checken bzw. den
MOSFET mit Kühlkörper zu versehen – für einen normalen 12V-Strip-Abschnitt
ist das aber unkritisch. Eine Freilaufdiode ist nicht nötig, der LED-Strip
ist eine ohmsche/kapazitive, keine induktive Last.

**Schalter – Pullup/Pulldown nötig?**

Nein, der interne `INPUT_PULLUP` des ESP8266 reicht, kein externer
Widerstand nötig. Schalter liegt zwischen GPIO und GND:
offen = HIGH, geschlossen = LOW. Die Firmware entprellt das Signal
softwareseitig (`SWITCH_DEBOUNCE_MS` in `Config.h`).

## MQTT

Steuerkommando (retained empfehlenswert) auf `home/ledstrip1/set`:

```json
{ "radar_armed": true, "switch_armed": false, "mode": "auto" }
```

- `mode = "on"` / `"off"`: Strip manuell an/aus, radar/switch werden ignoriert.
- `mode = "auto"`: Strip an, wenn (`radar_armed` UND Radar erkennt Präsenz)
  ODER (`switch_armed` UND Schalter ist offen).

Status wird (retained) auf `home/ledstrip1/state` gepublished, bei jeder
Zustandsänderung sowie alle 30s als Keepalive.

Topics/Client-ID lassen sich in `include/Config.h` anpassen.
