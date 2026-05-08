#include "config.h"
#include "board.h"
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

static int effectiveLedPin() {
    Preferences prefs;
    prefs.begin("esp32lab", true);
    int pin = prefs.getInt("ledpin", boardDefaultLedPin());
    prefs.end();
    return pin;
}

static void initLedPin(int pin) {
    if (pin < 0) return;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, boardLedOn() == HIGH ? LOW : HIGH);  // ensure off
}

void setupSystemApi() {
    initLedPin(effectiveLedPin());

    // GET /api/system/info
    apiServer.http().on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest* req) {
        ApiServer::restRequestCount++;
        JsonDocument doc;

        doc["name"]     = getDeviceName();
        doc["board"]    = boardName();
        doc["gpio_max"] = boardGpioMax();
        doc["led_pin"]  = effectiveLedPin();

        JsonArray safe = doc["gpio_safe"].to<JsonArray>();
        for (int i = 0; i <= boardGpioMax(); i++) {
            if (!boardPinReserved(i)) safe.add(i);
        }

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
            doc["wifi"]["mode"] = "station";
            doc["wifi"]["ssid"] = WiFi.SSID();
            doc["wifi"]["ip"]   = WiFi.localIP().toString();
            doc["wifi"]["rssi"] = WiFi.RSSI();
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

    // POST /api/system/identify  — blink the LED
    apiServer.http().on("/api/system/identify", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t*, size_t, size_t, size_t) {
            int pin = effectiveLedPin();
            if (pin < 0) {
                req->send(200, "application/json", "{\"status\":\"no_led\"}");
                return;
            }
            req->send(200, "application/json", "{\"status\":\"ok\"}");
            int* args = new int[2]{pin, boardLedOn()};
            xTaskCreate([](void* arg) {
                int p  = ((int*)arg)[0];
                int on = ((int*)arg)[1];
                delete[] (int*)arg;
                for (int i = 0; i < 8; i++) {
                    digitalWrite(p, i % 2 == 0 ? on : !on);
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
                digitalWrite(p, !on);
                vTaskDelete(NULL);
            }, "identify", 1024, args, 1, NULL);
        }
    );

    // POST /api/system/ledpin  { "pin": 8 }
    apiServer.http().on("/api/system/ledpin", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                ApiServer::sendError(req, 400, "Invalid JSON");
                return;
            }
            int pin = doc["pin"] | -2;
            if (pin < -1 || pin > boardGpioMax()) {
                ApiServer::sendError(req, 400, "Invalid pin");
                return;
            }
            int old = effectiveLedPin();
            if (old >= 0) pinMode(old, INPUT);

            Preferences prefs;
            prefs.begin("esp32lab", false);
            if (pin == -1) prefs.remove("ledpin");
            else           prefs.putInt("ledpin", pin);
            prefs.end();

            initLedPin(pin);
            Serial.printf("[System] LED pin set to GPIO%d\n", pin);
            req->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // POST /api/system/name  { "name": "mydevice" }
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
