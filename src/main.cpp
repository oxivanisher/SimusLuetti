#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <ESP8266WiFi.h>

// ── Pin assignments ──────────────────────────────────────────────────────────
#define PIN_BUTTON   D7   // GPIO13 — doorbell wire (active-low, internal pull-up)
#define PIN_DF_RX    D5   // GPIO14 — data from DFPlayer TX
#define PIN_DF_TX    D6   // GPIO12 — data to DFPlayer RX (via 1 kΩ series resistor)
#define PIN_DF_BUSY  D1   // GPIO5  — DFPlayer BUSY line (LOW = currently playing)

// ── Config ───────────────────────────────────────────────────────────────────
#define VOLUME       25   // 0–30
#define DEBOUNCE_MS  80   // ms to wait for button contact to settle

SoftwareSerial dfSerial(PIN_DF_RX, PIN_DF_TX);
DFRobotDFPlayerMini player;

int totalTracks = 0;

bool isPlaying() {
    return digitalRead(PIN_DF_BUSY) == LOW;
}

void blinkPanic() {
    // Indicates DFPlayer did not initialise — check wiring and SD card.
    while (true) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(150);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
    }
}

void setup() {
    // Kill WiFi immediately — not needed, saves ~70 mA.
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);

    pinMode(PIN_BUTTON,  INPUT_PULLUP);
    pinMode(PIN_DF_BUSY, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // active-low, so HIGH = off

    dfSerial.begin(9600);

    // false = no ACK (more robust across DFPlayer clone variants)
    if (!player.begin(dfSerial, false, true)) {
        blinkPanic();
    }

    player.volume(VOLUME);
    delay(200);

    totalTracks = player.readFileCounts();
    if (totalTracks <= 0) {
        blinkPanic(); // SD card empty or unreadable
    }

    // Mix analog noise + time for a good-enough random seed.
    randomSeed(analogRead(A0) ^ (millis() << 8));

    // Single long flash → ready.
    digitalWrite(LED_BUILTIN, LOW);
    delay(600);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    static bool stableState  = HIGH;
    static bool lastRaw      = HIGH;
    static unsigned long lastChange = 0;

    bool raw = digitalRead(PIN_BUTTON);

    if (raw != lastRaw) {
        lastChange = millis();
        lastRaw    = raw;
    }

    // Stable edge detected after debounce window
    if ((millis() - lastChange) > DEBOUNCE_MS && raw != stableState) {
        stableState = raw;

        // Falling edge = button pressed; ignore while a track is still playing.
        if (stableState == LOW && !isPlaying()) {
            player.play(random(1, totalTracks + 1));
        }
    }
}
