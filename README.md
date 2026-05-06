# ESP32 Lab

A browser-based sensor lab for ESP32 development boards. Flash the firmware, power the board, and control sensors from any browser — no app, no cloud, no accounts.

Designed as a low-cost, low-risk platform for hands-on electronics learning. Everything runs locally on the device.

---

## Supported boards

| Branch | Board | FQBN |
|--------|-------|------|
| `board/esp32-c3-mini` | ESP32-C3 Mini | `esp32:esp32:esp32c3` |
| `board/esp32-devkit` | ESP32 DevKit (WROOM-32) | `esp32:esp32:esp32` |

`master` is a read-only baseline — all development happens on the board branches.

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

Check out the branch for your board, then run `build.bat`:

```bat
git checkout board/esp32-c3-mini    (or board/esp32-devkit)

build.bat                           compile only
build.bat COM36                     compile + flash via USB
build.bat 192.168.0.117             compile + flash via OTA
```

The compiled binary lands in `firmware/build/esp32c3/` or `firmware/build/esp32dev/`.

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

`firmware/esp32lab/config.h` contains the board-specific settings:

```cpp
// Hotspot credentials (used when no network is saved, or saved network fails)
#define WIFI_AP_SSID      "ESP32Lab"
#define WIFI_AP_PASSWORD  "esp32lab"

// Sensor port pins
#define GROVE_D_PIN   4   // primary sensor data pin
#define GROVE_D2_PIN  5   // secondary (HC-SR04 echo, rotary DT)

// Built-in LED pin for the Identify feature (-1 to disable)
#define STATUS_LED_PIN  8    // C3 Mini: 8 (active-low)
                             // DevKit:  2 (active-high)
```

WiFi credentials for your home network are set via the browser UI and stored in NVS — no config file editing or reflash needed.

---

## Project structure

```
firmware/
  esp32lab/
    config.h          Board-specific config (pins, version)
    esp32lab.ino      Setup / loop
    api_server.*      ESPAsyncWebServer wrapper
    api_system.*      /api/system/* — info, identify, rename, OTA
    api_gpio.*        /api/gpio/* — pin read/write/mode
    api_grove.*       /api/grove/* — sensor config, read, SSE stream
    api_ota.*         /api/system/update — OTA upload handler
    api_webapp.*      Embedded web app (all HTML/CSS/JS as raw strings)
    wifi_manager.*    NVS WiFi provisioning + /api/wifi/* endpoints
    sketch.yaml       arduino-cli build metadata
  build/
    esp32c3/          Compiled output for C3 branch (gitignored)
    esp32dev/         Compiled output for DevKit branch (gitignored)
  install-libraries.bat

pwa/                  Web app source (kept in sync with api_webapp.cpp)
  index.html
  css/style.css
  js/api.js           HTTP wrapper
  js/app.js           Tab manager, connection logic
  js/tab-system.js    System tab (WiFi, OTA, naming, identify)
  js/tab-gpio.js      GPIO tab
  js/tab-grove.js     Sensors tab (wiring guides, streaming)

build.bat             Build + optional upload script
MANUAL.md             End-user kit manual
dist/                 Release binaries — copy here manually (gitignored)
```

---

## Release workflow

1. Tag the commit: `git tag v1.5.0`
2. Copy the bin to `dist/` with a descriptive name:
   - `dist/esp32lab-c3-v1.5.0.bin`
   - `dist/esp32lab-devkit-v1.5.0.bin`
3. Attach to a GitHub release if distributing

---

## Version history

| Version | Changes |
|---------|---------|
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
