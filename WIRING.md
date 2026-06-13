# Doorbell Wiring Guide

## Parts

| Part | Notes |
|---|---|
| ESP8266 D1 Mini | Any revision |
| DFPlayer Mini | Includes onboard micro-SD slot |
| Micro-SD card | FAT32, ≤32 GB |
| Resistor 1 kΩ | Protects DFPlayer RX from 3.3 V logic |
| Capacitor 100 µF electrolytic | Stabilises DFPlayer VCC during SD card power spikes — **strongly recommended** |
| Push button | Normally-open, doorbell wire |

---

## Pin connections

### ESP8266 D1 Mini → DFPlayer Mini

| D1 Mini pin | Label | DFPlayer pin | Notes |
|---|---|---|---|
| 5V | 5V | VCC | Power — 5 V preferred |
| GND | GND | GND | Common ground |
| D5 (GPIO14) | DF_RX | TX | Serial data from DFPlayer |
| D6 (GPIO12) | DF_TX | RX | Serial data to DFPlayer — **put 1 kΩ resistor in series** |
| D1 (GPIO5) | DF_BUSY | BUSY | LOW while a track plays |

### Volume potentiometer (optional)

Connect a 100 kΩ potentiometer to A0 to control volume at runtime. Enable it by uncommenting `#define USE_POT` in `src/main.cpp`.

| D1 Mini pin | Potentiometer |
|---|---|
| 3.3V | Left leg |
| A0 | Wiper (middle) |
| GND | Right leg |

### Doorbell button

| D1 Mini pin | Button wire |
|---|---|
| D7 (GPIO13) | Wire A (from doorbell) |
| GND | Wire B (from doorbell) |

The internal pull-up is enabled in firmware — no external resistor needed.

### Audio output

Use **one** of these two options depending on your amplifier:

**Option A — External amplifier / line-level output (default)**

Uses DAC_R / DAC_L. The firmware automatically mutes the DAC when idle to suppress hiss. Enable with `#define USE_EXTERNAL_AMP` in `src/main.cpp` (on by default).

| DFPlayer pin | Amplifier input |
|---|---|
| DAC_R | Right channel (or mono) |
| DAC_L | Left channel (optional) |
| GND | Amplifier ground |

If your amplifier is mono, connect DAC_R only (or bridge DAC_R + DAC_L via equal 100 kΩ resistors to a single input).

**Option B — DFPlayer onboard amplifier (up to 3 W, 4–8 Ω speaker)**

Uses SPK_1 / SPK_2. DAC muting is not available for this option — comment out `#define USE_EXTERNAL_AMP` in `src/main.cpp`.

| DFPlayer pin | Speaker |
|---|---|
| SPK_1 | Speaker + |
| SPK_2 | Speaker − |

---

## Wiring diagram

```
                  ┌─────────────────┐
     Doorbell ────┤ D7    D1 Mini   │
     GND ─────────┤ GND             │
                  │             D5  ├──────────────── DFPlayer TX
                  │        1kΩ  D6  ├──┤R├─────────── DFPlayer RX
                  │             D1  ├──────────────── DFPlayer BUSY
                  │             5V  ├──────────────── DFPlayer VCC ──┬── 100µF ──┐
                  │            GND  ├──────────────── DFPlayer GND ──┴───────────┘
                  └─────────────────┘

DFPlayer SPK_1 / SPK_2  →  speaker (option A)
DFPlayer DAC_R / DAC_L  →  external amp input (option B)
```

---

## Perfboard Layout

Suggested layout for a ~7×5 cm perfboard (2.54 mm pitch). Both ICs sit in **2×8 female Dupont pin headers** so they can be unplugged individually. All external wires connect via 2–3 pin female headers on the board edge.

```
        ┌─────────────────────────────────────────────────────────────────────┐
        │                                                                     │
        │  ┌────────────┐   ┌────────────┐   ┌─────┐  ┌─────┐  ┌─────────┐  │
        │  │            │   │            │   │ POT │  │ BTN │  │  AUDIO  │  │
        │  │  D1 MINI   │   │  DFPlayer  │   │     │  │     │  │         │  │
        │  │  2×8 ♀ hdr │   │  2×8 ♀ hdr │   │3-pin│  │2-pin│  │ 3-pin   │  │
        │  │            │   │            │   │ ♀   │  │ ♀   │  │ ♀       │  │
        │  └────────────┘   └────────────┘   └─────┘  └─────┘  └─────────┘  │
        │                                                                     │
        │  ┌─────┐   [R 1kΩ]   [C 100µF]                                     │
        │  │ PWR │                                                            │
        │  │2-pin│                                                            │
        │  │ ♀   │                                                            │
        │  └─────┘                                                            │
        └─────────────────────────────────────────────────────────────────────┘
```

**Headers on the board edge:**

| Header | Pins | Connects to |
|---|---|---|
| PWR | 2 | +5 V supply and GND |
| POT | 3 | Potentiometer legs (3.3 V, wiper, GND) |
| BTN | 2 | Doorbell wires |
| AUDIO | 3 | DAC_R, DAC_L, GND — or SPK_1, SPK_2 for onboard amp |

---

### Point-to-point soldering checklist

Solder in this order to keep things tidy — power rails first, then signal wires, then passives.

| # | From | To | Notes |
|---|---|---|---|
| 1 | PWR header + | D1 Mini 5V | Main +5 V rail |
| 2 | PWR header − | D1 Mini GND | Main GND rail |
| 3 | D1 Mini 5V | DFPlayer VCC | |
| 4 | D1 Mini GND | DFPlayer GND | |
| 5 | DFPlayer VCC | 100 µF cap + | Place cap as close to DFPlayer as possible |
| 6 | DFPlayer GND | 100 µF cap − | Mind polarity — stripe on cap = negative |
| 7 | D1 Mini D5 | DFPlayer TX | Signal wire |
| 8 | D1 Mini D6 | 1 kΩ resistor leg A | |
| 9 | 1 kΩ resistor leg B | DFPlayer RX | Resistor bridges D6 to RX |
| 10 | D1 Mini D1 | DFPlayer BUSY | |
| 11 | D1 Mini D7 | BTN header pin 1 | |
| 12 | D1 Mini GND | BTN header pin 2 | |
| 13 | D1 Mini 3V3 | POT header pin 1 | |
| 14 | D1 Mini A0 | POT header pin 2 | Wiper |
| 15 | D1 Mini GND | POT header pin 3 | |
| 16 | DFPlayer DAC_R | AUDIO header pin 1 | Right / mono to amp |
| 17 | DFPlayer DAC_L | AUDIO header pin 2 | Left to amp (optional) |
| 18 | DFPlayer GND | AUDIO header pin 3 | Signal ground to amp |

> **Tip:** solder the 1 kΩ resistor flat against the board between the D6 pad and the DFPlayer RX pad — no extra wire needed if the pads are close enough.

---

## SD card setup

1. Format the card as **FAT32**. Cards ≥32 GB ship as exFAT — reformat them (DFPlayer Mini supports up to 32 GB FAT32). On Windows use `format E: /FS:FAT32 /Q`; on Linux `sudo mkfs.fat -F 32 /dev/sdX1`; on macOS use Disk Utility and choose MS-DOS (FAT).
2. Place MP3 files in the **root directory**, named with a zero-padded three-digit number:

   ```
   001.mp3
   002.mp3
   003.mp3
   ...
   ```

3. Keep filenames sequential with no gaps — the DFPlayer uses FAT table order, not alphabetical order, so numbering prevents unexpected playback sequences.
4. Maximum 255 files in the root using this naming scheme.

**MP3 encoding requirements** — the DFPlayer requires CBR (constant bitrate) MP3s. VBR files cause static or choppy playback. Re-encode with ffmpeg before copying:

```bash
mkdir -p converted
for i in *.mp3; do
  ffmpeg -i "$i" -codec:a libmp3lame -b:a 128k -ar 44100 -ac 1 "converted/$i"
done
```

Then rename the output files to `001.mp3`, `002.mp3`, … and copy them to the SD card root.

---

## Volume

`VOLUME` is set to `25` (out of 30) in `src/main.cpp`. Adjust to taste before flashing:

```cpp
#define VOLUME  25   // 0–30
```

---

## Status LED (built-in D1 Mini LED)

| Pattern | Meaning |
|---|---|
| Single 600 ms flash on boot | Initialised successfully, ready |
| Rapid continuous blinking | Error — check SD card and DFPlayer wiring |
