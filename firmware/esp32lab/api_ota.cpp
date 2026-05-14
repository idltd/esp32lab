#include "config.h"
#include "api_server.h"
#include "api_ota.h"
#include <Update.h>

static unsigned long gRestartAt = 0;

void otaLoop() {
    if (gRestartAt && millis() >= gRestartAt) {
        Serial.println("[OTA] Restarting now.");
        ESP.restart();
    }
}

void setupOtaApi() {
    apiServer.http().on("/api/system/update", HTTP_POST,
        // Called when the full upload is complete
        [](AsyncWebServerRequest* req) {
            bool ok = !Update.hasError();
            req->send(ok ? 200 : 500, "application/json",
                ok ? "{\"status\":\"ok\"}"
                   : "{\"error\":\"Flash write failed\"}");
            if (ok) gRestartAt = millis() + 1500;  // restart from loop() after TCP flushes
        },
        // Called for each chunk of the uploaded binary
        [](AsyncWebServerRequest* req, const String& filename,
           size_t index, uint8_t* data, size_t len, bool final) {
            if (!index) {
                Serial.printf("[OTA] Receiving: %s\n", filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            }
            if (len && Update.write(data, len) != len) {
                Update.printError(Serial);
            }
            if (final) {
                if (!Update.end(true)) {
                    Update.printError(Serial);
                } else {
                    Serial.printf("[OTA] Done — %u bytes written\n", index + len);
                }
            }
        }
    );

    Serial.println("[API] OTA endpoint registered");
}
