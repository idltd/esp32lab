#include "config.h"
#include "api_server.h"
#include "api_system.h"
#include <WiFi.h>
#include <Preferences.h>
#include <ESPmDNS.h>

extern bool gStaMode;

String getDeviceName() {
    Preferences prefs;
    prefs.begin("esp32lab", true);
    String name = prefs.getString("devname", "");
    prefs.end();
    if (name.length() > 0) return name;
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char buf[24];
    snprintf(buf, sizeof(buf), "esp32lab-%02x%02x", mac[4], mac[5]);
    return String(buf);
}

void setupSystemApi() {
#if STATUS_LED_PIN >= 0
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, !STATUS_LED_ON);  // ensure off at boot
#endif

    // GET /api/system/info
    apiServer.http().on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest* req) {
        ApiServer::restRequestCount++;
        JsonDocument doc;

        doc["name"]        = getDeviceName();
        doc["chip"]["model"]    = ESP.getChipModel();
        doc["chip"]["revision"] = ESP.getChipRevision();
        doc["chip"]["cores"]    = ESP.getChipCores();
        doc["chip"]["freq_mhz"] = ESP.getCpuFreqMHz();

        doc["heap"]["free"]     = ESP.getFreeHeap();
        doc["heap"]["total"]    = ESP.getHeapSize();
        doc["heap"]["min_free"] = ESP.getMinFreeHeap();

        doc["psram"]["free"]  = ESP.getFreePsram();
        doc["psram"]["total"] = ESP.getPsramSize();

        doc["firmware"]    = FIRMWARE_VERSION;
        doc["uptime_ms"]   = millis();
        doc["sdk_version"] = ESP.getSdkVersion();

        doc["flash"]["size"]  = ESP.getFlashChipSize();
        doc["flash"]["speed"] = ESP.getFlashChipSpeed();

        if (gStaMode) {
            doc["wifi"]["mode"]    = "station";
            doc["wifi"]["ssid"]    = WiFi.SSID();
            doc["wifi"]["ip"]      = WiFi.localIP().toString();
            doc["wifi"]["rssi"]    = WiFi.RSSI();
        } else {
            doc["wifi"]["mode"]    = "hotspot";
            doc["wifi"]["ssid"]    = WIFI_AP_SSID;
            doc["wifi"]["ip"]      = WiFi.softAPIP().toString();
            doc["wifi"]["clients"] = WiFi.softAPgetStationNum();
        }

        doc["ws_clients"] = apiServer.wsClientCount();
        doc["traffic"]["rest_requests"] = ApiServer::restRequestCount;
        doc["traffic"]["ws_messages"]   = ApiServer::wsMessageCount;

        String json; serializeJson(doc, json);
        req->send(200, "application/json", json);
    });

    // POST /api/system/identify  — blink the status LED
    apiServer.http().on("/api/system/identify", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t*, size_t, size_t, size_t) {
            req->send(200, "application/json", "{\"status\":\"ok\"}");
#if STATUS_LED_PIN >= 0
            xTaskCreate([](void*) {
                for (int i = 0; i < 8; i++) {
                    digitalWrite(STATUS_LED_PIN, i % 2 == 0 ? STATUS_LED_ON : !STATUS_LED_ON);
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
                digitalWrite(STATUS_LED_PIN, !STATUS_LED_ON);
                vTaskDelete(NULL);
            }, "identify", 1024, NULL, 1, NULL);
#endif
        }
    );

    // POST /api/system/name  { "name": "esp32lab-02" }
    apiServer.http().on("/api/system/name", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                ApiServer::sendError(req, 400, "Invalid JSON");
                return;
            }
            const char* name = doc["name"] | "";
            if (strlen(name) == 0 || strlen(name) > 63) {
                ApiServer::sendError(req, 400, "name must be 1-63 characters");
                return;
            }
            Preferences prefs;
            prefs.begin("esp32lab", false);
            prefs.putString("devname", name);
            prefs.end();
            Serial.printf("[System] Device name set to '%s' — restarting.\n", name);
            req->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Restarting...\"}");
            delay(500);
            ESP.restart();
        }
    );

    Serial.println("[API] System endpoints registered");
}
