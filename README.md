# ESP32 Lab

A browser-based sensor lab for ESP32 development boards. Flash the firmware, power the board, and control sensors from any browser — no app, no cloud, no accounts.

Designed as a low-cost, low-risk platform for hands-on electronics learning. Everything runs locally on the device.

---

## Supported boards

| Board | CPU architecture | FQBN | build.bat target |
|-------|-----------------|------|-----------------|
| ESP32-C3 Mini | RISC-V | `esp32:esp32:esp32c3` | `c3` |
| ESP32 DevKit (WROOM-32) | Xtensa LX6 | `esp32:esp32:esp32` | `devkit` |

### Why two binaries?

ESP32 variants use different CPU architectures. The C3 runs a **RISC-V** core; the classic DevKit runs an **Xtensa LX6** core. These use entirely different instruction sets — a binary compiled for one will simply not run on the other. This is a fundamental hardware constraint, not a software choice.

Within each architecture, however, the firmware handles variation automatically. At boot it calls `ESP.getChipModel()` to identify the exact chip (ESP32, ESP32-C3, ESP32-S2, ESP32-S3, ESP32-C6…) and uses that to configure:

- The GPIO pin range (e.g. 0–21 on the C3, 0–39 on the classic ESP32)
- Which pins are reserved (flash, USB, strapping pins — touching these crashes or bricks the board)
- The default LED pin and whether it is active-high or active-low

So: **one codebase, one `master` branch** — but you must flash the binary that matches your board's CPU architecture.

---

## Features

**Three-tab web UI** served directly from the device:

- **System** — chip model, memory usage, uptime, WiFi status, OTA firmware update, device naming, WiFi provisioning
- **GPIO** — read/write individual pins, set mode (input/output/pullup/pulldown), live log
- **Sensors** — select a sensor, follow the wiring guide, read once or stream continuously

**Supported sensors:**

| Sensor | What it measures |
|--------|-----------------|
| Digital Input | Button, switch, any digital signal |
| Digital Output | LED, relay, buzzer |
| Analog Input | Potentiometer, light sensor, 0–3.3V range |
| PWM Output | LED dimming, buzzer tone |
| DHT11 | Temperature + humidity |
| DS18B20 | Waterproof temperature probe |
| HC-SR04P | Ultrasonic distance |
| Rotary Encoder | Turn count + direction (KY-040) |

**WiFi provisioning (no reflash required):**
- First boot creates an `ESP32Lab` hotspot
- Connect via browser → System tab → WiFi Configuration
- Scan for nearby networks, enter password, connect
- Device saves credentials to NVS and reboots onto your network
- Falls back to hotspot automatically if the saved network is unreachable

**Other features:**
- Browser-based OTA firmware updates (System tab → Firmware Update)
- Per-device naming stored in NVS — default based on MAC address (e.g. `esp32lab-dba8`), accessible via `http://esp32lab-dba8.local/`
- LED identify button — blinks the built-in LED so you know which device you're talking to
- Configurable LED pin via UI — set and save from the browser, no reflash
- Runtime board self-detection — GPIO limits, reserved pins, and LED polarity set automatically at boot
- JS/CSS assets cached in browser with firmware-version cache busting

---

## Getting started

### 1. Prerequisites (first time only)

Install the ESP32 board support and required libraries:

```
arduino-cli core install esp32:esp32
firmware\install-libraries.bat
```

### 2. Build and flash

Run `build.bat` from the repo root:

```bat
build.bat                           compile both boards
build.bat c3                        compile C3 Mini only
build.bat devkit                    compile DevKit only
build.bat c3 COM36                  compile C3 + flash via USB
build.bat devkit 192.168.0.117      compile DevKit + flash via OTA
```

Compiled binaries land in `firmware/build/esp32c3/` and `firmware/build/esp32dev/`.

Pre-built binaries are also available on the [Releases page](https://github.com/idltd/esp32lab/releases).

### 3. Connect

**First boot (no saved WiFi):**
1. Connect to `ESP32Lab` WiFi (password: `esp32lab`)
2. Open `http://192.168.4.1`

**After joining your network:**
- Open `http://esp32lab-XXXX.local/` (where XXXX = last 4 chars of MAC)
- Or use the IP address shown in your router's device list

> **Phone tip:** disable mobile data before connecting in hotspot mode, otherwise the phone stays on 4G/5G.

### 4. Wire a sensor

Click **Sensors**, pick a type from the dropdown, follow the colour-coded wiring guide, click **Configure Sensor**, then **Read Once** or **▶ Stream**.

---

## Configuration

Most settings are configured via the browser UI and stored in NVS — no reflash needed:
- **WiFi** — System tab → WiFi Configuration
- **LED pin** — System tab → Device → LED Pin
- **Device name** — System tab → Device → Name

`firmware/esp32lab/config.h` contains only the fixed compile-time settings:

```cpp
// Hotspot credentials (AP fallback when no network is saved or reachable)
#define WIFI_AP_SSID      "ESP32Lab"
#define WIFI_AP_PASSWORD  "esp32lab"

// Grove sensor port pins
#define GROVE_D_PIN   4   // primary sensor data pin
#define GROVE_D2_PIN  5   // secondary (HC-SR04 echo, rotary DT)
```

Board-specific settings (GPIO limits, reserved pins, LED pin and polarity) are detected automatically at runtime via `ESP.getChipModel()`.

---

## Project structure

```
firmware/
  esp32lab/
    config.h          Hotspot credentials, Grove pins, firmware version
    board.*           Runtime chip detection — GPIO limits, reserved pins, LED polarity
    esp32lab.ino      Setup / loop
    api_server.*      ESPAsyncWebServer wrapper
    api_system.*      /api/system/* — info, identify, LED pin, rename, OTA
    api_gpio.*        /api/gpio/* — pin read/write/mode
    api_grove.*       /api/grove/* — sensor config, read, SSE stream
    api_ota.*         /api/system/update — OTA upload handler
    api_webapp.*      Embedded web app (all HTML/CSS/JS as raw strings)
    wifi_manager.*    NVS WiFi provisioning + /api/wifi/* endpoints
    sketch.yaml       arduino-cli build metadata
  build/
    esp32c3/          Compiled output — C3 Mini (gitignored)
    esp32dev/         Compiled output — DevKit (gitignored)
  install-libraries.bat

pwa/                  Web app source (kept in sync with api_webapp.cpp)
  index.html
  css/style.css
  js/api.js           HTTP wrapper
  js/app.js           Tab manager, connection logic
  js/tab-system.js    System tab (WiFi, OTA, naming, LED pin, identify)
  js/tab-gpio.js      GPIO tab (dynamic board-aware pin list)
  js/tab-grove.js     Sensors tab (wiring guides, streaming)

build.bat             Unified build + upload script (c3 / devkit / both)
MANUAL.md             End-user kit manual
```

---

## Release workflow

1. Bump `FIRMWARE_VERSION` in `config.h`
2. Build both boards: `build.bat`
3. Commit and tag: `git tag vX.Y.Z && git push origin master --tags`
4. Create a GitHub release and attach both `.bin` files from `firmware/build/`

---

## Version history

| Version | Changes |
|---------|---------|
| 1.6.0 | Runtime board self-detection, configurable LED pin via UI, dynamic GPIO safe-pin list, unified build.bat, single master branch |
| 1.5.0 | Device naming (NVS, MAC-based default), LED identify button, asset caching with version-based cache busting |
| 1.4.0 | WiFi network scan in provisioning UI |
| 1.3.0 | NVS-based WiFi provisioning via web UI (no reflash), boot-time auto-reconnect, AP fallback |
| 1.2.0 | Browser-based OTA firmware updates |
| 1.1.0 | Station mode (join home network, compile-time config) |
| 1.0.0 | Initial release — System, GPIO, Sensors tabs; 8 sensor types; wiring guides |

---

## Safety notes

- ESP32 GPIO is **3.3V only** — never connect 5V signals directly to a GPIO pin
- Pins connected to internal flash will crash or brick the board — the firmware refuses to touch them
- **HC-SR04P** (3.3V) is required; the standard HC-SR04 outputs 5V on ECHO and will damage the ESP32. If you only have a 5V HC-SR04, use a voltage divider on the ECHO line (10kΩ to pin, 20kΩ to GND)
- DS18B20 requires a 4.7kΩ pull-up resistor between DATA and 3.3V

---

## Adding a sensor type

1. Add a `SensorDef` entry to `SENSORS[]` in `api_grove.cpp`
2. Handle it in `configureSensor()` and `takeReading()`
3. Add its wiring guide to `buildWiringGuide()` in `pwa/js/tab-grove.js`
4. Re-embed the updated JS into `api_webapp.cpp` (FILE_GROVE_JS section)
5. Run `build.bat` and flash
