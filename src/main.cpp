#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <ESP8266WiFi.h>

// ── Pin assignments ──────────────────────────────────────────────────────────
#define PIN_BUTTON  D7  // GPIO13 — doorbell wire (active-low, internal pull-up)
#define PIN_DF_RX   D5  // GPIO14 — data from DFPlayer TX
#define PIN_DF_TX   D6  // GPIO12 — data to DFPlayer RX (via 1 kΩ series resistor)
#define PIN_DF_BUSY D1  // GPIO5  — DFPlayer BUSY line (LOW = currently playing)

// ── Config ───────────────────────────────────────────────────────────────────
#define DEBOUNCE_MS 80  // ms to wait for button contact to settle

// Uncomment to control volume via a potentiometer on A0 (10 kΩ, wired 3.3V→wiper→GND).
// When disabled, VOLUME is used instead.
#define USE_POT
#define VOLUME 30       // 0–30, used only when USE_POT is not defined

SoftwareSerial dfSerial(PIN_DF_RX, PIN_DF_TX);
DFRobotDFPlayerMini player;

int totalTracks = 0;
int lastTrack   = -1;

bool isPlaying()
{
    return digitalRead(PIN_DF_BUSY) == LOW;
}

void blinkPanic(const char *msg)
{
    Serial.println(msg);
    while (true)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(150);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
    }
}

#ifdef USE_POT
int readVolume()
{
    return map(analogRead(A0), 0, 1023, 0, 30);
}
#endif

void setup()
{
    Serial.begin(115200);
    Serial.println("\n[doorbell] boot");

    // Kill WiFi immediately — not needed, saves ~70 mA.
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);

    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_DF_BUSY, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // active-low, so HIGH = off

    dfSerial.begin(9600);
    delay(1000); // give DFPlayer time to power up before talking to it

    // isACK=false, doReset=false: skip handshake entirely — more tolerant of clones
    player.begin(dfSerial, false, false);
    delay(500);
    Serial.println("[doorbell] DFPlayer init sent");

#ifdef USE_POT
    player.volume(readVolume());
    Serial.print("[doorbell] volume (pot): ");
    Serial.println(readVolume());
    randomSeed(analogRead(A0) ^ (millis() << 8));
#else
    player.volume(VOLUME);
    Serial.print("[doorbell] volume (fixed): ");
    Serial.println(VOLUME);
    randomSeed(analogRead(A0) ^ (millis() << 8));
#endif

    delay(200);

    for (int attempt = 1; attempt <= 5 && totalTracks <= 0; attempt++) {
        totalTracks = player.readFileCounts();
        Serial.print("[doorbell] readFileCounts() attempt ");
        Serial.print(attempt);
        Serial.print(" returned: ");
        Serial.println(totalTracks);
        if (totalTracks <= 0) delay(500);
    }
    if (totalTracks <= 0)
    {
        Serial.println("[doorbell] check: DFPlayer TX->D5, DFPlayer RX->D6, VCC->5V, 100uF cap on VCC/GND");
        blinkPanic("[doorbell] halted");
    }

    // Single long flash → ready.
    digitalWrite(LED_BUILTIN, LOW);
    delay(600);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
#ifdef USE_POT
    static int lastVol = -1;
    static unsigned long lastVolCheck = 0;
    if (millis() - lastVolCheck > 200) {
        lastVolCheck = millis();
        int vol = readVolume();
        if (vol != lastVol) {
            player.volume(vol);
            lastVol = vol;
        }
    }
#endif

    static bool stableState = HIGH;
    static bool lastRaw     = HIGH;
    static unsigned long lastChange = 0;

    bool raw = digitalRead(PIN_BUTTON);

    if (raw != lastRaw)
    {
        lastChange = millis();
        lastRaw    = raw;
    }

    // Stable edge detected after debounce window
    if ((millis() - lastChange) > DEBOUNCE_MS && raw != stableState)
    {
        stableState = raw;

        // Falling edge = button pressed; ignore while a track is still playing.
        if (stableState == LOW && !isPlaying())
        {
            int track;
            do { track = random(1, totalTracks + 1); } while (track == lastTrack && totalTracks > 1);
            lastTrack = track;
            Serial.print("[doorbell] playing track ");
            Serial.println(track);
            player.play(track);
        }
    }
}
