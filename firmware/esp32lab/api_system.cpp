#include "config.h"
#include "api_server.h"
#include "api_system.h"
#include <WiFi.h>

extern bool gStaMode;

void setupSystemApi() {
    apiServer.http().on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest* req) {
        ApiServer::restRequestCount++;
        JsonDocument doc;

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

    Serial.println("[API] System endpoints registered");
}
