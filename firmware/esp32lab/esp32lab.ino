// ============================================================================
// ESP32 Lab — WiFi Sensor API Server
//
// Turns a bare ESP32 dev board into a browser-controlled sensor lab.
//
// AP mode (default): connect to "ESP32Lab" hotspot → http://192.168.4.1/
// STA mode:          joins your WiFi → http://esp32lab.local/
//
// Set WIFI_MODE in config.h to switch between modes.
//
// Libraries required (install via firmware/install-libraries.bat):
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
#include "api_ota.h"

// Exposed so api_system can report the correct IP and mode
bool gStaMode = false;

static void setupWiFi() {
#if WIFI_MODE == WIFI_MODE_STA
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
    Serial.printf("[WiFi] Joining '%s'", WIFI_STA_SSID);

    int waited = 0;
    while (WiFi.status() != WL_CONNECTED && waited < WIFI_STA_TIMEOUT_S * 2) {
        delay(500);
        Serial.print(".");
        waited++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        gStaMode = true;
        Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
        return;
    }

    // Fall back to AP if STA connection failed
    Serial.println("\n[WiFi] Could not connect — falling back to hotspot mode");
#endif

    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CONN);
    Serial.printf("[WiFi] Hotspot: %s  IP: %s\n",
                  WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
}

static void setupMDNS() {
    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("[mDNS] http://%s.local/\n", MDNS_HOSTNAME);
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[ESP32 Lab] Starting up...");

    setupWiFi();
    setupMDNS();

    setupWebApp();
    setupSystemApi();
    setupGpioApi();
    setupGroveApi();
    setupOtaApi();

    apiServer.begin();

    if (gStaMode) {
        Serial.printf("[ESP32 Lab] Ready! Browse to http://%s/  or  http://%s.local/\n",
                      WiFi.localIP().toString().c_str(), MDNS_HOSTNAME);
    } else {
        Serial.printf("[ESP32 Lab] Ready! Connect to WiFi '%s' (password: %s)\n",
                      WIFI_AP_SSID, WIFI_AP_PASSWORD);
        Serial.printf("[ESP32 Lab] Browse to http://%s/  or  http://%s.local/\n",
                      WiFi.softAPIP().toString().c_str(), MDNS_HOSTNAME);
    }
}

void loop() {
    apiServer.loop();
    groveLoop();
}
