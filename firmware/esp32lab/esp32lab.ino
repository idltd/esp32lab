// ============================================================================
// ESP32 Lab — WiFi Sensor API Server
//
// Turns a bare ESP32 dev board into a browser-controlled sensor lab.
// Connect to "ESP32Lab" WiFi AP → open http://192.168.4.1/ in any browser.
//
// Supported: digital I/O, analog read, PWM, DHT11, DS18B20, HC-SR04, rotary encoder.
//
// Libraries required (install via Arduino Library Manager or install-libraries.bat):
//   - ESPAsyncWebServer + AsyncTCP  (git URL install)
//   - ArduinoJson (v7)
//   - DHTesp
//   - OneWire
//   - DallasTemperature
// ============================================================================

#include <WiFi.h>
#include <ESPmDNS.h>

#include "config.h"
#include "api_server.h"
#include "api_webapp.h"
#include "api_system.h"
#include "api_gpio.h"
#include "api_grove.h"

static void setupWiFiAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CONN);
    Serial.printf("[WiFi] AP: %s  IP: %s\n",
                  WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
}

static void setupMDNS() {
    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("[mDNS] %s.local\n", MDNS_HOSTNAME);
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[ESP32 Lab] Starting up...");

    setupWiFiAP();
    setupMDNS();

    setupWebApp();
    setupSystemApi();
    setupGpioApi();
    setupGroveApi();

    apiServer.begin();

    Serial.printf("[ESP32 Lab] Ready! Connect to WiFi '%s' (password: %s)\n",
                  WIFI_AP_SSID, WIFI_AP_PASSWORD);
    Serial.printf("[ESP32 Lab] Open browser: http://%s/  or  http://%s.local/\n",
                  WiFi.softAPIP().toString().c_str(), MDNS_HOSTNAME);
}

void loop() {
    apiServer.loop();
    groveLoop();
}
