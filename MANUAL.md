# ESP32 Lab — Kit Manual

**Version 1.0 draft**

---

## What is ESP32 Lab?

ESP32 Lab is a small electronics kit that lets you connect real-world sensors to a web browser. Plug in a sensor, connect your phone or laptop to the kit's WiFi, open a browser — and you can see live readings from that sensor immediately. No coding required to get started.

The kit is designed to be safe and forgiving. All sensors run on 3.3 volts. The browser interface shows you exactly how to wire each sensor before you connect anything. If something goes wrong, just unplug and try again — you cannot damage a computer or phone by using this kit.

---

## What's in the kit

| Item | Quantity | What it's for |
|------|----------|---------------|
| ESP32 development board (pre-flashed) | 1 | The brain — runs a tiny web server over WiFi |
| USB cable (micro-USB or USB-C, depending on board) | 1 | Power |
| Breadboard (400 holes) | 1 | Connect sensors without soldering |
| Jumper wires, male-to-male | 10 | Wire sensors to the board |
| DHT11 module | 1 | Measures temperature and humidity |
| HC-SR04P ultrasonic module | 1 | Measures distance |
| KY-040 rotary encoder module | 1 | A dial that counts turns |
| LED (any colour) + 330Ω resistor | 1 each | Controlled output experiments |

> **Note:** The HC-SR04**P** in this kit is the 3.3V version. Standard HC-SR04 modules (without the P) look identical but use 5V and will damage the ESP32. If you are sourcing your own, make sure you get the P version.

---

## Part 1 — Getting Started

### Step 1: Power the board

Plug the USB cable into the ESP32 board and connect it to any USB power source — a phone charger, a laptop, or a USB power bank all work. You do not need a computer for this step.

The board does not have an on/off switch. It starts up as soon as it receives power.

### Step 2: Connect to the WiFi

On your phone, tablet, or laptop, go to WiFi settings and connect to:

| | |
|--|--|
| **Network name** | `ESP32Lab` |
| **Password** | `esp32lab` |

> **Phone users:** After connecting, your phone may warn you that this network has no internet access. That is normal — the ESP32 Lab does not connect to the internet. If your phone asks whether to "stay connected", choose yes. You may also need to turn off mobile data temporarily, otherwise your phone will keep using 4G/5G instead of the ESP32's WiFi.

### Step 3: Open the browser

Open any web browser (Chrome, Firefox, Safari) and go to:

**http://192.168.4.1**

You should see the ESP32 Lab interface. The status dot in the top bar will turn green and show "Connected".

If it does not connect, check that you are still on the ESP32Lab WiFi network and try refreshing the page.

---

## Part 2 — The Interface

The interface has three tabs:

**System** — shows information about the ESP32 board itself: chip model, memory usage, how long it has been running, and how many devices are connected to its WiFi.

**GPIO** — lets you control individual pins on the board directly. This is for more advanced experiments (see Part 4).

**Sensors** — the main tab. This is where you select a sensor, follow the wiring guide, and read data from it.

---

## Part 3 — Sensors

Each sensor experiment follows the same steps:

1. Go to the **Sensors** tab
2. Choose the sensor type from the dropdown menu
3. Follow the wiring guide that appears — it shows exactly which sensor pin connects to which ESP32 pin
4. Click **Configure Sensor**
5. Click **Read Once** to take a single reading, or **▶ Stream** to read continuously

The wiring guide in the app shows pin connections. The instructions below explain what each sensor does and suggest things to try.

---

### DHT11 — Temperature and Humidity

**What it does:** Measures air temperature (in °C) and relative humidity (%).

**Wiring:** The module has three pins labelled VCC, GND, and DATA (or S). Connect as shown in the app. No extra components needed — the module has a built-in pull-up resistor.

**Things to try:**
- Breathe gently on the sensor. Watch the humidity reading rise.
- Hold something cold (like a cold drink can) near it. Watch the temperature drop slowly.
- Compare the reading to a room thermometer. How close is it?
- Use **▶ Stream** to watch the values update in real time.

**Notes:** The DHT11 takes a reading once every second or so. If you see an error message, check the wiring — the DATA wire is the most common cause.

---

### HC-SR04P — Ultrasonic Distance

**What it does:** Measures how far away an object is, by sending out a pulse of ultrasound and timing the echo. Range roughly 2cm to 400cm.

**Wiring:** Four pins — VCC, GND, TRIG (trigger), and ECHO. The TRIG and ECHO pins connect to two different ESP32 pins (shown in the app). **Use HC-SR04P only — not standard HC-SR04.**

**Things to try:**
- Hold your hand in front of it at different distances. Watch the reading change.
- Use **▶ Stream** and slowly move your hand towards and away from it.
- Point it at a wall and measure the distance across the room.
- Try pointing it at soft objects (cushions, clothing) vs hard surfaces. Does it read differently?

**Notes:** The sensor works best with flat, solid surfaces perpendicular to the sensor. Soft or angled surfaces may give inconsistent readings.

---

### KY-040 Rotary Encoder — Turn Counter

**What it does:** Counts how many steps a dial has been turned, and in which direction. Clockwise adds to the count; anticlockwise subtracts.

**Wiring:** Three pins used — VCC (+), GND (–), CLK, and DT. Connect as shown in the app.

**Things to try:**
- Turn the dial slowly. Watch the Steps count change.
- Click **Reset Steps** to set it back to zero.
- Use **▶ Stream** to see the count update as you turn.
- Try turning quickly — does it keep up?

**Notes:** The encoder module also has a push-button (SW pin) which is not used in this experiment.

---

### LED — Digital Output

**What it does:** Turns a pin HIGH (3.3V) or LOW (0V) to switch an LED on and off.

**Wiring:** Connect the longer leg of the LED to the 330Ω resistor, then the other end of the resistor to the ESP32 D pin (GPIO4). Connect the shorter leg of the LED to GND. The resistor limits the current so the LED and the ESP32 are not damaged.

**On the Sensors tab:** Select **Digital Output**. Configure it, then use the LOW/HIGH toggle to switch the LED on and off.

**Things to try:**
- Switch it on and off.
- Try connecting two LEDs in parallel (both anodes to the same resistor, both cathodes to GND).

---

### LED Dimmer — PWM Output

**What it does:** Sends a rapidly switching signal that makes an LED appear brighter or dimmer. PWM stands for Pulse Width Modulation.

**Wiring:** Same as Digital Output above — LED + resistor on the D pin.

**On the Sensors tab:** Select **PWM Output**. Configure it, then drag the Duty slider from 0 (off) to 255 (full brightness).

**Things to try:**
- Slide the duty all the way up and down. Notice the LED dims smoothly.
- Set it to 128 (half duty). Does the LED look half as bright, or different?
- Set it to a very low value like 5 or 10 — the LED should flicker just perceptibly.

---

### Digital Input — Button

**What it does:** Reads whether a pin is HIGH or LOW, so you can detect button presses or switches.

**Wiring:** Connect one end of a button to the D pin (GPIO4) and the other end to GND. Use **INPUT_PULLUP** mode (available in the GPIO tab or when using Digital Input sensor type) — this means the pin reads HIGH when the button is not pressed, and LOW when pressed.

Alternatively, just connect a jumper wire from the D pin to GND to simulate a button press.

**Things to try:**
- Press the button. Watch the reading change from HIGH to LOW.
- Use **▶ Stream** and press and release quickly — can you see every press?

---

### DS18B20 — Waterproof Temperature Probe

**What it does:** Measures temperature with a waterproof stainless-steel probe. Accurate to ±0.5°C. Good for measuring water or soil temperature.

**Wiring:** Three wires — red (VCC) to 3.3V, black (GND) to GND, yellow (DATA) to the D pin. **Required:** a 4.7kΩ resistor between 3.3V and the DATA wire. Without this resistor, readings will fail.

**Things to try:**
- Dip the probe in cold water. Watch the temperature drop.
- Hold the probe in your fist. Watch it slowly rise to body temperature.
- Compare it to the DHT11 reading if both are running at once.

---

## Part 4 — GPIO Tab (direct pin control)

The GPIO tab gives you direct control of any safe pin on the ESP32. You can set a pin as input or output, read its value, or write HIGH/LOW to it.

This is lower-level than the Sensors tab — you are controlling the pin directly without any sensor logic.

**Safe pins** on a standard ESP32 DevKit: 4, 5, 13, 14, 16–27, 32–33.

> **Avoid** pins 0, 1, 2, 3, 6–12, and 15 — these are used for boot, USB serial, and internal flash memory.

---

## Part 5 — For Researchers

This section is for people who want to understand how ESP32 Lab works, modify the firmware, or add their own sensors.

### How it works

The ESP32 runs a small web server (ESPAsyncWebServer) that listens on port 80. When you browse to `http://192.168.4.1`, it serves a web application — a set of HTML, CSS, and JavaScript files — directly from the firmware's flash memory.

The web app communicates with the firmware using:
- **REST API** — simple HTTP GET/POST requests for reading sensors and setting modes
- **Server-Sent Events (SSE)** — a one-way stream from the firmware to the browser, used to push sensor readings continuously when streaming is active

No WebSocket is used for sensors — SSE is simpler and sufficient for one-way data.

### Project structure

```
firmware/
  esp32lab/
    esp32lab.ino      Main sketch — WiFi AP setup, module registration
    config.h          All pin assignments, SSID, password, version
    api_server.*      HTTP server wrapper (ESPAsyncWebServer)
    api_system.*      GET /api/system/info
    api_gpio.*        GET/POST /api/gpio/{pin} and /api/gpio/{pin}/mode
    api_grove.*       Sensor catalogue, readings, SSE stream
    api_webapp.*      Embeds the web app into firmware flash
    sketch.yaml       arduino-cli build profile

pwa/
  index.html          Three-tab UI shell
  css/style.css       All styling
  js/api.js           HTTP wrapper (fetch-based)
  js/app.js           Tab manager, connection logic
  js/tab-system.js    System info tab
  js/tab-gpio.js      GPIO tab
  js/tab-grove.js     Sensors tab (wiring guides, streaming)
```

### The API

All endpoints are available at `http://192.168.4.1`.

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/system/info` | Chip, memory, WiFi, uptime |
| GET | `/api/gpio/{pin}` | Read a pin |
| POST | `/api/gpio/{pin}` | Write a pin `{"value": 0}` or `{"value": 1}` |
| POST | `/api/gpio/{pin}/mode` | Set mode `{"mode": "input"}` etc. |
| GET | `/api/grove/sensors` | List all supported sensor types |
| GET | `/api/grove/config` | Current sensor and pin assignments |
| POST | `/api/grove/configure` | Select sensor `{"sensor": "dht11"}` |
| GET | `/api/grove/read` | Take one reading |
| POST | `/api/grove/write` | Write output `{"value": 1}` or `{"duty": 128}` |
| POST | `/api/grove/rotary/reset` | Reset rotary step counter |
| GET | `/api/grove/stream` | SSE stream — subscribe to continuous readings |

You can call these from any HTTP client — curl, Postman, Python, or your own code.

**Example — read the DHT11 from a terminal:**
```
curl http://192.168.4.1/api/grove/read
```
Response:
```json
{"sensor":"dht11","ts":12453,"temperature":"23.5","humidity":"61.0"}
```

**Example — subscribe to the stream (Python):**
```python
import requests

url = 'http://192.168.4.1/api/grove/stream'
with requests.get(url, stream=True) as resp:
    for line in resp.iter_lines():
        if line.startswith(b'data:'):
            print(line[5:].decode())
```

### Changing the sensor pins

Edit `firmware/esp32lab/config.h`:
```cpp
#define GROVE_D_PIN    4    // primary sensor pin
#define GROVE_D2_PIN   5    // secondary (HC-SR04 echo, rotary DT)
```

Any GPIO number valid for your board can be used. Rebuild and reflash after changing.

### Changing WiFi credentials

Also in `config.h`:
```cpp
#define WIFI_AP_SSID      "ESP32Lab"
#define WIFI_AP_PASSWORD  "esp32lab"
```

### Adding a new sensor

1. Add a new `SensorDef` entry to the `SENSORS[]` array in `api_grove.cpp`
2. Add a case in `configureSensor()` to set up the pins
3. Add a case in `takeReading()` to return the data as JSON
4. Add the wiring guide to `buildWiringGuide()` in `pwa/js/tab-grove.js`
5. Re-embed the updated `tab-grove.js` content into `api_webapp.cpp`
6. Rebuild and reflash

### Building the firmware

**Prerequisites:**
- arduino-cli installed
- ESP32 core: `arduino-cli core install esp32:esp32`
- Libraries: run `firmware\install-libraries.bat`

**Compile:**
```
arduino-cli compile --profile default firmware/esp32lab
```

**Upload** (replace COM3 with your port):
```
arduino-cli upload --fqbn esp32:esp32:esp32dev --port COM3 --input-dir "firmware/build/esp32.esp32.esp32dev"
```

**Find your port:**
```
arduino-cli board list
```

### Updating the web app

The web app lives in `pwa/`. After editing, the changed files must be re-embedded into `firmware/esp32lab/api_webapp.cpp` as C raw string literals (see the existing file for the pattern), then the firmware rebuilt and reflashed.

A future improvement would be a script to automate this embedding step.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| Can't find ESP32Lab WiFi | Board not powered, or still booting | Wait 5 seconds after plugging in; check USB cable |
| Browser shows "Network error" | Phone switched back to mobile data | Turn off mobile data; stay on ESP32Lab WiFi |
| Sensor shows error message | Wiring mistake | Recheck the wiring guide; check that VCC is on 3.3V not 5V |
| DHT11 keeps erroring | DATA wire not connected, or no pull-up | Check connection; breakout modules include pull-up, bare sensor needs 10kΩ |
| DS18B20 "sensor not found" | Missing 4.7kΩ pull-up resistor | Add resistor between 3.3V and DATA wire |
| HC-SR04 always errors | Wrong module (HC-SR04 not HC-SR04P) | Check module label — must end in P |
| Distance readings are wild | Object too close, or angled surface | Keep objects at least 5cm away; use flat surfaces |
| Rotary encoder misses steps | Turning too fast, or bad connection | Turn slowly; check CLK and DT wires |

---

## Safety

- All sensors in this kit use **3.3V**. Do not connect them to 5V.
- Do not connect the ESP32 GPIO pins directly to mains electricity or batteries above 3.3V.
- The ESP32 and sensors can be damaged by static electricity — handle by the edges of the board.
- This kit is not waterproof. Keep it away from liquids (except the DS18B20 probe tip, which is designed to be submerged).

---

*ESP32 Lab is open source. Firmware and web app source at: [github link TBD]*
