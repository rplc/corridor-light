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

| Pin | GPIO | Funktion |
|---|---:|---|
| D1 | 5 | Gate IRLB8721 (über 220R) |
| D5 | 14 | Reed N.C. (C an GND, INPUT_PULLUP) |
| D7 | 13 | UART0 RX ← Radar TX (`Serial.swap()`) |
| D8 | 15 | UART0 TX → Radar RX (`Serial.swap()`) |
| D4 | 2 | UART1 TX → Debug (optional) |
| 5V | – | von UBEC (12V→5V) |
| GND | – | gemeinsame Masse mit PSU/Radar/MOSFET |

Der LD2410 hängt jetzt an der **Hardware-UART0** des ESP8266. `Serial.swap()`
legt UART0 auf **D7/GPIO13 = RX** und **D8/GPIO15 = TX**. Das ist bei
256000 Baud deutlich sinnvoller als SoftwareSerial. D8/GPIO15 ist zwar ein
Boot-Strapping-Pin, muss beim Reset LOW sein und wird auf dem Wemos bereits
entsprechend beschaltet; der RX-Eingang des Radars ist hochohmig und zieht
ihn nicht hoch.

Da UART0 damit exklusiv vom Radar verwendet wird, laufen Debug-Ausgaben
über **UART1 / Serial1 auf D4/GPIO2**. Für den normalen Betrieb wird dieser
Pin nicht benötigt, weil die Logs zusätzlich über MQTT auf
`hallway-light/log` ausgegeben werden.

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

Nein, der interne `INPUT_PULLUP` des ESP8266 reicht. Verwendet wird der
**N.C.**-Kontakt (Normally Closed):

- **COM → GND**
- **NC → D5/GPIO14**

Damit gilt:

- Kontakt geschlossen → GPIO LOW
- Kontakt offen → GPIO HIGH

Die Firmware interpretiert HIGH als `switch_open=true` und entprellt das
Signal über `SWITCH_DEBOUNCE_MS`.

## MQTT

Ein einziges Topic für Kommando und Status: `hallway-light`. Der ESP ist
darauf subscribed und published seinen eigenen Status (retained) auf
dasselbe Topic.

Steuerkommando (retained empfehlenswert):

```json
{ "radar_armed": true, "switch_armed": false, "mode": "auto", "brightness": 100 }
```

- `mode = "on"` / `"off"`: Strip manuell an/aus, radar/switch werden ignoriert.
- `mode = "auto"`: Strip an, wenn (`radar_armed` UND Radar erkennt Präsenz)
  ODER (`switch_armed` UND Schalter ist offen).
- `brightness` (0-100, optional): Zielhelligkeit in %, gilt für **jeden**
  "an"-Zustand (egal ob durch `mode: "on"` oder durch Radar/Switch im
  `auto`-Modus ausgelöst) — ein einziger Helligkeitsregler statt zweier
  getrennter Konzepte. Fehlt das Feld, bleibt der zuletzt gesetzte Wert
  erhalten (Default beim Boot: `DEFAULT_BRIGHTNESS_PCT` in `Config.h`,
  100%). Änderungen werden weich eingeblendet (`LED_FADE_MS`).

Der ESP ergänzt beim Publishen eigene Felder (`led_on`, `radar_presence`,
`switch_open`, `ts`), gepublished wird nur bei tatsächlicher Änderung,
kein periodisches Keepalive. Da der ESP sein eigenes Topic subscribed hat,
bekommt er seinen Status-Publish auch selbst wieder zugestellt — das ist
unkritisch, weil dabei nur dieselben `radar_armed`/`switch_armed`/`mode`/
`brightness`-Werte erneut gesetzt werden (idempotent).

Client-ID/Topic/Pins lassen sich in `include/Config.h` anpassen.

## Logs ohne Serial-Monitor (MQTT-Log)

Wenn ESP + Radar zusammen so viel Strom ziehen, dass ein USB-Port/Kabel
während des Betriebs nicht mehr ausreicht (Serial-Monitor dann nicht
nutzbar), gibt es einen zweiten Kanal: **`hallway-light/log`** — nicht
retained, reiner Live-Stream derselben Zeilen, die auch auf Serial
stünden (`DebugLog::logf(...)` in `src/DebugLog.*`).

- Jede geloggte Zeile geht **immer** auf Serial1/D4 (falls angeschlossen) UND in
  einen kleinen internen Puffer (700 Bytes).
- Sobald MQTT verbunden ist, wird der Puffer in `main.cpp` geleert und auf
  `hallway-light/log` published.
- Läuft der Puffer zwischendurch voll (z.B. weil WLAN/MQTT länger nicht
  verbunden sind), fallen die ältesten Zeilen raus (FIFO) — das ist ein
  Live-Debug-Werkzeug, kein vollständiges Log.
- Boot-Meldungen wie `[Radar] LD2410 gefunden...` oder `[Radar] LD2410
  NICHT gefunden...` entstehen **vor** WiFi/MQTT, landen also zunächst nur
  im Puffer und werden nachgeliefert, sobald die Verbindung steht.
- Bewusst **nicht** über diesen Kanal geloggt: alles, was während eines
  laufenden OTA-Updates passiert (`ArduinoOTA.onStart/onProgress/onError`
  in `OtaManager.cpp`) — da wollen wir keine zusätzliche Netzwerk-I/O
  parallel zum Flash-Schreiben riskieren. Diese Meldungen bleiben Serial1-only.

Einfach den Topic abonnieren (`mosquitto_sub -t hallway-light/log -v` o.ä.),
während der ESP ohne USB läuft.

## Radar fein-tunen (Gate-Sensitivität, Reichweite, Timeout)

Der LD2410 unterteilt die Reichweite in bis zu 9 "Gates" (je ~0,75m) mit
je einer eigenen Sensitivität (0-100) für bewegte und stehende Ziele.
Darüber steuert man, ab welcher Signalstärke ("Energie") etwas als
Präsenz zählt — z.B. um kleine/entfernte Ziele zu ignorieren.

**Konfigurieren per MQTT** (auf dem normalen `hallway-light`-Topic, zusammen
mit oder unabhängig von den anderen Feldern):

```json
{
  "radar_config": {
    "max_moving_gate": 4,
    "max_stationary_gate": 4,
    "timeout_s": 5,
    "gate_sensitivity": [
      { "gate": 2, "moving": 60, "stationary": 50 },
      { "gate": 3, "moving": 70, "stationary": 60 }
    ]
  }
}
```

- `max_moving_gate` / `max_stationary_gate`: ab welchem Gate bewegte bzw.
  stehende Ziele ignoriert werden (reduziert effektiv die Reichweite).
- `timeout_s`: wie lange der Sensor nach dem letzten Ziel noch "Presence"
  meldet, bevor er auf "kein Ziel" zurückfällt (zusätzlich zum
  software-seitigen `RADAR_HOLD_MS` in `Config.h`).
- `gate_sensitivity`: pro Gate `moving`/`stationary` (0-100, höher =
  unempfindlicher). `max_moving_gate`/`max_stationary_gate` sind nur
  gemeinsam wirksam (beide Felder nötig), `gate_sensitivity` kann auch
  allein geschickt werden, mit beliebig vielen Gates im Array.

`radar_config` wird **nicht** dauerhaft gespeichert/retained-relevant
gehalten, sondern bei Empfang einmalig an den Sensor durchgereicht
(`RadarSensor::applyConfig`) und taucht nicht im Status-Publish auf.

**Live-Messwerte zum Tuning**: Mit dem Environment `d1_mini_radar_tuning`
flashen (`pio run -e d1_mini_radar_tuning -t upload`, bzw. die OTA-Variante
davon ableiten) — das setzt das Build-Flag `RADAR_DEBUG_TELEMETRY=1` und
published danach alle 1s (`RADAR_TELEMETRY_INTERVAL_MS`) die rohen
Messwerte auf `hallway-light/radar-debug`:

```json
{
  "presence": true,
  "moving": { "detected": true, "distance_cm": 180, "energy": 72 },
  "stationary": { "detected": false, "distance_cm": 0, "energy": 0 },
  "max_gate": 8, "max_moving_gate": 8, "max_stationary_gate": 8,
  "gate_sensitivity": [ { "gate": 0, "moving": 50, "stationary": 40 }, ... ],
  "ts": 123456
}
```

Damit lässt sich live beobachten, welche `energy`-Werte ein Zielobjekt in
welcher Distanz erzeugt, und die Sensitivität passend hochdrehen (um
kleine Ziele auszublenden) oder runterdrehen (um empfindlicher zu werden).
Danach wieder mit `d1_mini`/`d1_mini_ota` (ohne das Flag) flashen, damit im
Normalbetrieb kein zusätzlicher MQTT-Traffic anfällt.

**Hinweis zur Lib-Version:** `setGateSensitivity()`/`setMaxValues()` sind
zwar in der README der `ld2410`-Lib dokumentiert, aber im aktuell neuesten
Release-Tag (`v0.2.2`) noch nicht enthalten — die README spiegelt bereits
den `main`-Branch wider. `platformio.ini` hängt die Lib deshalb bewusst
direkt vom `main`-Branch ab (`https://github.com/ncmreynolds/ld2410.git`)
statt von einer Registry-Version. Nachteil: `main` kann sich jederzeit
ändern. Sobald ein Release mit diesen Methoden getaggt ist, auf
`ncmreynolds/ld2410@^<neue-version>` zurückstellen (siehe Kommentar in
`platformio.ini`).

**Hinweis zur Zuverlässigkeit:** Der Radar hängt jetzt über die
Hardware-UART0 des ESP8266 bei 256000 Baud. Die `ld2410`-Bibliothek
bevorzugt für diese hohe Baudrate eine Hardware-UART; SoftwareSerial war
daher bewusst nur eine Übergangslösung. UART0 wird mit `Serial.swap()` auf
D7/D8 gelegt, während UART1/`Serial1` auf D4 für Debug-Ausgaben frei bleibt.
