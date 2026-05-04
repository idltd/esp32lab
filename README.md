# ESP32 Lab

A browser-based sensor lab for bare ESP32 development boards. Flash the firmware, connect to the WiFi access point, and experiment with sensors directly from any browser — no app to install, no cloud, no accounts.

Designed as a low-cost, low-risk platform for hands-on electronics learning.

## What it does

The ESP32 creates a WiFi access point. Any device (phone, tablet, laptop) can connect to it and open a browser to control sensors on the breadboard in real time.

**Three tabs:**
- **System** — chip info, memory, uptime
- **GPIO** — read/write individual GPIO pins
- **Sensors** — plug in a sensor, select its type, and start reading data immediately

**Supported sensors:**
| Sensor | What it measures |
|--------|-----------------|
| Digital Input | Button, switch, any digital signal |
| Digital Output | LED, relay, buzzer |
| Analog Input | Potentiometer, light sensor, anything 0–3.3V |
| PWM Output | LED dimming, buzzer tone |
| DHT11 | Temperature + humidity |
| DS18B20 | Waterproof temperature probe |
| HC-SR04P | Ultrasonic distance (3.3V version) |
| Rotary Encoder | Turns to steps (KY-040 module) |

Each sensor shows a **wiring guide** in the browser — colour-coded sensor pin → ESP32 pin table, with safety warnings where relevant.

## Hardware required

- Any ESP32 dev board (ESP32-WROOM-32 / DevKit V1 recommended)
- USB cable for flashing
- Breadboard + jumper wires
- Sensors from the list above (cheap modules from AliExpress/Amazon)

**Default sensor pins:** GPIO4 (D) and GPIO5 (D2). Change in `firmware/esp32lab/config.h` if needed.

## Getting started

### 1. Flash the firmware

**First time only — install libraries:**
```
firmware\install-libraries.bat
```

**Install ESP32 board support (first time only):**
```
arduino-cli core install esp32:esp32
```

**Compile:**
```
arduino-cli compile --profile default firmware/esp32lab
```

**Upload** (replace `COM3` with your port):
```
arduino-cli upload --fqbn esp32:esp32:esp32dev --port COM3 --input-dir firmware/build/esp32.esp32.esp32dev
```

### 2. Connect

1. On your phone or laptop, connect to WiFi: **`ESP32Lab`** / password **`esp32lab`**
2. Open browser and go to **`http://192.168.4.1`**
3. The app connects automatically and shows the System tab

> **Phone users:** disable mobile data before connecting — otherwise the phone stays on mobile internet and can't reach the ESP32.

### 3. Wire a sensor

Click the **Sensors** tab, pick a sensor type from the dropdown, and follow the wiring guide that appears. Then click **Configure Sensor**, and use **Read Once** or **▶ Stream** to get readings.

## Configuration

Edit `firmware/esp32lab/config.h` to change:
- WiFi SSID / password
- Grove D pin (default GPIO4) and D2 pin (default GPIO5)
- mDNS hostname (default `esp32lab` → `http://esp32lab.local`)

## Project structure

```
firmware/
  esp32lab/           Arduino sketch
    config.h          All configuration
    esp32lab.ino      Main sketch (setup/loop)
    api_server.*      HTTP + WebSocket server
    api_system.*      System info endpoint
    api_gpio.*        GPIO read/write endpoints
    api_grove.*       Sensor endpoints + SSE stream
    api_webapp.*      Embedded web app (no LittleFS needed)
    sketch.yaml       arduino-cli build profile
  install-libraries.bat
pwa/                  Web app source (mirrored into api_webapp.cpp)
  index.html
  css/style.css
  js/app.js, api.js, tab-*.js
```

## Version history

- 1.0.0 — initial release (System, GPIO, Sensors tabs; 8 sensor types; wiring guides)

## Safety notes

- ESP32 GPIO is **3.3V only** — never connect 5V signals directly
- Pins 6–11 are connected to internal flash — **never use these**
- The HC-SR04P (3.3V) is required — standard HC-SR04 outputs 5V on ECHO and will damage the ESP32
- DS18B20 requires a 4.7kΩ pull-up resistor between DATA and 3.3V

## Adding more sensor types

Add a new entry to the `SENSORS[]` array in `api_grove.cpp`, handle it in `configureSensor()` and `takeReading()`, then add its wiring guide to `buildWiringGuide()` in `pwa/js/tab-grove.js` and re-embed with `api_webapp.cpp`.
