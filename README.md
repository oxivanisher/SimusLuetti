# SimusLuetti

Replaces a standard doorbell with an MP3-playing one. When the doorbell button is pressed, a random track is played from an SD card. The same track is never played twice in a row.

## Hardware

- ESP8266 D1 Mini
- DFPlayer Mini
- Micro-SD card (FAT32, up to 32 GB)
- Existing doorbell button and speakers with amplifier

WiFi is disabled at boot — the device runs fully standalone.

## Setup

1. Flash the firmware using [PlatformIO](https://platformio.org/): `pio run --target upload`
2. Prepare the SD card and wire everything up according to [WIRING.md](WIRING.md)
3. Power on — a single long LED flash means it's ready

## Configuration

Two constants at the top of [src/main.cpp](src/main.cpp):

| Constant | Default | Description |
|---|---|---|
| `VOLUME` | `30` | Playback volume (0–30) |
| `DEBOUNCE_MS` | `80` | Button debounce window in milliseconds |
