# ESP32 Lab — Let's Get Started!

---

## What is ESP32 Lab?

ESP32 Lab is a kit that lets you connect sensors to a web browser.

A sensor is something that measures the real world — temperature, distance, light, movement. You'll connect real sensors to a tiny computer chip, and watch the readings appear live in your browser. On your phone or laptop.

No coding needed to start. You just wire it up and look.

---

## What's in your kit

| Thing | What it does |
|-------|-------------|
| ESP32-C3 board | The brain — a tiny computer that runs a mini website over WiFi |
| USB cable | Gives it power (like charging a phone) |
| Breadboard | A board you can plug wires and components into without soldering |
| Jumper wires | Short wires to connect things together |
| DHT11 module | Measures temperature and humidity |
| HC-SR04P module | Measures distance using sound |
| KY-040 rotary encoder | A dial that counts how many times you turn it |
| LED + 330Ω resistor | A light you can switch on and off |

> **Important:** The distance sensor in this kit is an HC-SR04**P** — that letter P matters! There's another version without the P that looks identical but can damage the board. If you ever buy your own, make sure it has the P.

---

## Part 1 — Plugging it in

### Step 1: Give it power

Plug the USB cable into the ESP32 board and into any USB charger, laptop, or power bank.

No on/off switch — it starts up as soon as it's plugged in.

### Step 2: Connect to its WiFi

The board creates its own WiFi network. Find it on your phone, tablet, or laptop:

| WiFi name | `ESP32Lab_XXXX` (the Xs will be different on yours) |
|---|---|
| Password | `esp32lab` |

> **Phone users:** Your phone might say "this WiFi has no internet." That's fine — it's supposed to. The ESP32 Lab is its own little network, not connected to the internet. If your phone asks whether to stay connected, say yes. You might also need to turn off mobile data, or your phone will use 4G instead of the board's WiFi.

### Step 3: Open the browser

On most phones, a page pops up automatically as soon as you connect to the board's WiFi — you'll be taken straight to the ESP32 Lab.

If that doesn't happen, open Chrome, Firefox, or Safari and go to:

**http://192.168.4.1**

You should see the ESP32 Lab page. The dot at the top will go green when it's connected.

If nothing shows up, check you're still on the ESP32Lab WiFi and refresh the page.

---

### Want to use your home WiFi instead? (optional)

By default the board creates its own WiFi hotspot. That works fine anywhere — no router needed.

But if you'd like it to join your home WiFi (so you don't have to keep switching):

1. Connect to the ESP32Lab hotspot and open the app as normal
2. Go to the **System** tab, scroll down to **WiFi Configuration**
3. Click **Scan**, find your network, enter the password, click **Connect**
4. The board restarts and joins your WiFi

After that you can reach it at **http://esp32lab.local** without switching WiFi. If that doesn't work, check your router for a device called `esp32lab` and use its IP address.

If it ever can't find your WiFi, it automatically goes back to hotspot mode — so you can always reach it.

---

## Part 2 — What you're looking at

At the top there are three tabs:

**System** — tells you about the board itself (memory, WiFi, uptime). Also where you can update the firmware and change which pins the sensors use (Grove Pins).

**GPIO** — lets you control individual pins on the board directly. More on this in Part 4.

**Sensors** — this is the main one. Pick a sensor, wire it up, get readings.

---

## Part 3 — Trying the sensors

Every sensor works the same way:

1. Go to the **Sensors** tab
2. Pick the sensor type from the dropdown list
3. A wiring guide appears — it shows you exactly which wire goes where
4. Wire it up
5. Click **Configure Sensor**
6. Click **Read Once** for one reading, or **▶ Stream** to keep reading

---

### DHT11 — Temperature and Humidity

**What it measures:** Air temperature (°C) and humidity (how much water is in the air, as a %).

**Wiring:** Three wires: VCC, GND, and DATA. The app shows you where each one goes. The module has everything it needs built in.

**Things to try:**
- Breathe on it gently. Watch the humidity go up.
- Hold something cold near it (like a cold drink). Watch the temperature slowly drop.
- How close is it to a normal thermometer?
- Use **▶ Stream** and watch the numbers update live.

*If you see an error, check the DATA wire — that's usually the problem.*

---

### HC-SR04P — Distance

**What it measures:** How far away something is. It sends out a sound pulse you can't hear, and times how long the echo takes to bounce back. A bit like a bat.

**Wiring:** Four wires — VCC, GND, TRIG, and ECHO. The app shows where each one goes.

**Things to try:**
- Hold your hand in front of it at different distances.
- Use **▶ Stream** and slowly move your hand closer and further away.
- Point it at a wall and try to measure the room.
- Does it read soft things (a cushion, a jumper) differently from hard things?

*It works best when something flat is directly in front of it.*

---

### KY-040 — Rotary Encoder (Turn Counter)

**What it does:** Counts how many clicks the dial has been turned. Turn clockwise: the number goes up. Turn anticlockwise: it goes down.

**Wiring:** Four wires — VCC (+), GND (–), CLK, DT. The app shows where.

**Things to try:**
- Turn the dial and watch the Steps count.
- Press **Reset Steps** to go back to zero.
- Turn it really fast — does it keep up?

---

### Digital Output — Switching an LED on and off

**What it does:** Sets a pin to HIGH (3.3V) or LOW (0V) to switch an LED on or off.

**Wiring:** The longer leg of the LED → 330Ω resistor → GPIO4. The shorter leg → GND. The resistor is important — without it, too much current flows and the LED burns out.

Select **Digital Output** in the app, then use the LOW/HIGH button to switch the LED.

**Things to try:**
- Switch it on and off.
- Try two LEDs: connect both longer legs to the same resistor, both shorter legs to GND.

---

### PWM Output — Making an LED dimmer

**What it does:** Makes an LED appear brighter or dimmer by flicking it on and off incredibly fast — faster than your eyes can see. The more time it spends on versus off, the brighter it looks.

> This is called **Pulse Width Modulation**, or PWM. It's how your phone's screen brightness works. How dimmers on light switches work. How the speed of electric motors is controlled. The same idea is everywhere once you know it.

**Wiring:** Same as above — LED + resistor on GPIO4.

Select **PWM Output**, then drag the Duty slider from 0 (off) to 255 (full brightness).

**Things to try:**
- Drag the slider up and down slowly. Notice the LED fades smoothly.
- Set it to 128. Does it look exactly half as bright?
- Set it to something tiny like 5 or 10. Can you see it flickering?

---

### Digital Input — Detecting a button press

**What it does:** Reads whether a pin is HIGH or LOW — so you can tell if a button is pressed or not.

**Wiring:** One end of a button to GPIO4, the other end to GND. Use **INPUT_PULLUP** mode.

(No button? Touch a jumper wire from GPIO4 to GND — that works too.)

**Things to try:**
- Press the button and watch the reading change from HIGH to LOW.
- Use **▶ Stream** and tap quickly. Can you catch every press?

---

### Analogue Input — Reading a sliding value

**What it does:** Instead of just HIGH or LOW, an analogue pin reads any voltage between 0V and 3.3V. It gives you a number between 0 and 4095. A potentiometer (a dial with a wiper) or a light sensor are good things to try.

Connect one to the analogue pin shown in the app and turn it or shade it.

> Most things in the real world aren't just on or off. Temperature slides between hot and cold. Light fades. Volume goes up and down. Analogue inputs let your microcontroller measure that in-between world.

---

### DS18B20 — Waterproof Temperature Probe

**What it does:** Measures temperature with a metal probe that you can dip in water or push into soil. Accurate to within half a degree.

**Wiring:** Three wires — red to 3.3V, black to GND, yellow to GPIO4. You **must** also add a 4.7kΩ resistor between 3.3V and the yellow wire. Without it, nothing will work.

**Things to try:**
- Dip the probe in cold water. Watch the reading drop.
- Hold it in your fist. Watch it slowly climb towards body temperature (about 37°C).
- Compare it with the DHT11 reading at the same time.

---

## Part 4 — GPIO tab

The GPIO tab lets you control individual pins on the board directly — without using a named sensor. You can set a pin as an input or an output, read its value, or switch it on or off yourself.

The app automatically shows only the pins that are safe to use. Some pins on the board are connected to the chip's own internal memory — the app hides those so you can't accidentally cause problems.

---

## Part 5 — When things go wrong

| What's happening | Most likely cause | What to try |
|-----------------|-------------------|-------------|
| Can't see the ESP32Lab WiFi | Board not powered, or still starting up | Wait 5 seconds after plugging in; check the cable |
| Browser won't load the page | Phone switched back to mobile data | Turn off mobile data; check you're on ESP32Lab WiFi |
| Need to wipe the saved WiFi and start over | Wrong password saved, or moved to a different network | Triple-click the BOOT button within 2 seconds — the board resets and goes back to hotspot mode |
| Sensor shows an error | Wiring mistake | Check the wiring guide; make sure VCC is on 3.3V not 5V |
| DHT11 keeps erroring | DATA wire not connected | Check all three wires |
| DS18B20 "not found" | Missing 4.7kΩ resistor | Add the resistor between 3.3V and the DATA wire |
| Distance sensor errors | Wrong sensor (not the P version) | Check the label on the module |
| Distance readings jump around | Object too close or at an angle | Keep objects at least 5cm away; face them flat-on |
| Rotary encoder misses turns | Turning too fast | Slow down; check CLK and DT wires are connected |

---

## Staying safe

- All the sensors in this kit run on **3.3V**. Don't connect them to 5V.
- Don't connect the board's pins to a mains plug, or to a battery bigger than 3.3V.
- Pick the board up by its edges — static electricity from your fingers can damage it.
- Keep everything away from water. (The DS18B20 metal probe tip is fine in water, but the rest isn't.)

---

## What's next?

You've connected sensors, read data, and controlled an LED. What could you make with these building blocks?

The next project is the **ESP32 Meter** — you build a test instrument on a breadboard that can measure voltage, identify resistors, test diodes, and measure capacitors. It teaches you what the components you've been connecting actually are and how they work.

After that: if you want to write your own code for the board, there's a coding stream. If you want to build something that lasts and learn to solder, there's a hardware stream.

You've started something.
