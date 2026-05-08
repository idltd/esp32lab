#include "config.h"
#include "api_server.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>

static bool tryConnect(const String& ssid, const String& pass) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.printf("[WiFi] Trying '%s'", ssid.c_str());
    for (int i = 0; i < WIFI_STA_TIMEOUT_S * 2; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connection failed — falling back to hotspot.");
    WiFi.disconnect(true);
    delay(100);
    return false;
}

static void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CONN);
    Serial.printf("[WiFi] Hotspot: %s  IP: %s\n",
                  WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
}

bool wifiManagerSetup() {
    Preferences prefs;
    prefs.begin("esp32lab", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();

    if (ssid.length() > 0) {
        Serial.printf("[WiFi] Stored credentials for '%s'\n", ssid.c_str());
        if (tryConnect(ssid, pass)) return true;
    }

    startAP();
    return false;
}

// ── API endpoints ─────────────────────────────────────────────────────────────

void setupWifiApi() {
    // GET /api/wifi/config
    apiServer.http().on("/api/wifi/config", HTTP_GET, [](AsyncWebServerRequest* req) {
        Preferences prefs;
        prefs.begin("esp32lab", true);
        String savedSsid = prefs.getString("ssid", "");
        prefs.end();

        bool sta = (WiFi.status() == WL_CONNECTED);
        JsonDocument doc;
        doc["mode"]       = sta ? "sta" : "ap";
        doc["ssid"]       = sta ? WiFi.SSID() : String(WIFI_AP_SSID);
        doc["ip"]         = sta ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        doc["saved_ssid"] = savedSsid;
        String json; serializeJson(doc, json);
        req->send(200, "application/json", json);
    });

    // POST /api/wifi/connect  { "ssid": "...", "password": "..." }
    apiServer.http().on("/api/wifi/connect", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                ApiServer::sendError(req, 400, "Invalid JSON");
                return;
            }
            const char* ssid = doc["ssid"] | "";
            const char* pass = doc["password"] | "";
            if (strlen(ssid) == 0) {
                ApiServer::sendError(req, 400, "ssid required");
                return;
            }
            Preferences prefs;
            prefs.begin("esp32lab", false);
            prefs.putString("ssid", ssid);
            prefs.putString("pass", pass);
            prefs.end();
            Serial.printf("[WiFi] Credentials saved for '%s' — restarting.\n", ssid);
            req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Restarting...\"}");
            delay(500);
            ESP.restart();
        }
    );

    // GET /api/wifi/scan
    apiServer.http().on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest* req) {
        int n = WiFi.scanComplete();
        if (n == WIFI_SCAN_RUNNING) {
            req->send(202, "application/json", "{\"status\":\"scanning\"}");
            return;
        }
        if (n == WIFI_SCAN_FAILED) {
            WiFi.scanNetworks(true);  // start async scan
            req->send(202, "application/json", "{\"status\":\"scanning\"}");
            return;
        }
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (int i = 0; i < n; i++) {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0) continue;  // skip hidden networks
            bool dup = false;
            for (JsonObject o : arr) {
                if (o["ssid"].as<String>() == ssid) { dup = true; break; }
            }
            if (!dup) {
                JsonObject net = arr.add<JsonObject>();
                net["ssid"]   = ssid;
                net["rssi"]   = WiFi.RSSI(i);
                net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            }
        }
        WiFi.scanDelete();
        String json; serializeJson(doc, json);
        req->send(200, "application/json", json);
    });

    // POST /api/wifi/forget
    apiServer.http().on("/api/wifi/forget", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            Preferences prefs;
            prefs.begin("esp32lab", false);
            prefs.clear();
            prefs.end();
            Serial.println("[WiFi] Credentials cleared — restarting.");
            req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Restarting...\"}");
            delay(500);
            ESP.restart();
        }
    );

    Serial.println("[API] WiFi endpoints registered");
}
