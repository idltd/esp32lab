// ============================================================================
// ESP32 Lab — WiFi Sensor API Server
//
// Turns a bare ESP32 dev board into a browser-controlled sensor lab.
//
// First boot: creates "ESP32Lab" hotspot → http://192.168.4.1/
// After WiFi config: joins saved network → http://esp32lab.local/
// If saved network unreachable: falls back to hotspot automatically.
//
// Libraries required (install via firmware/install-libraries.bat):
//   - ESPAsyncWebServer + AsyncTCP  (git URL install)
//   - ArduinoJson (v7)
//   - DHT sensor library for ESPx
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
#include "api_ota.h"
#include "wifi_manager.h"

bool gStaMode = false;

static void setupMDNS(const String& hostname) {
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("[mDNS] http://%s.local/\n", hostname.c_str());
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[ESP32 Lab] Starting up...");

    gStaMode = wifiManagerSetup();
    setupMDNS(getDeviceName());

    setupWebApp();
    setupSystemApi();
    setupGpioApi();
    setupGroveApi();
    setupOtaApi();
    setupWifiApi();

    apiServer.begin();

    if (gStaMode) {
        Serial.printf("[ESP32 Lab] Ready! Browse to http://%s/  or  http://%s.local/\n",
                      WiFi.localIP().toString().c_str(), getDeviceName().c_str());
    } else {
        Serial.printf("[ESP32 Lab] Ready! Connect to WiFi '%s' (password: %s)\n",
                      WIFI_AP_SSID, WIFI_AP_PASSWORD);
        Serial.printf("[ESP32 Lab] Browse to http://%s/  or  http://%s.local/\n",
                      WiFi.softAPIP().toString().c_str(), getDeviceName().c_str());
    }
}

void loop() {
    apiServer.loop();
    groveLoop();
}
