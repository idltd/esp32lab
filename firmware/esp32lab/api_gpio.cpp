#include "config.h"
#include "api_server.h"
#include "api_gpio.h"
#include <Arduino.h>

static bool pinConfigured[GPIO_MAX_PIN + 1] = {false};

static bool isReservedPin(int pin) {
    for (int i = 0; i < (int)RESERVED_PIN_COUNT; i++) {
        if (RESERVED_PINS[i] == pin) return true;
    }
    return false;
}

static bool validatePin(AsyncWebServerRequest* req, int pin) {
    if (pin < 0 || pin > GPIO_MAX_PIN) {
        ApiServer::sendError(req, 400, "Invalid pin number (0-39)");
        return false;
    }
    if (isReservedPin(pin)) {
        ApiServer::sendError(req, 400, "Pin is reserved (flash/UART/strapping/Grove)");
        return false;
    }
    return true;
}

void setupGpioApi() {
    // GET /api/gpio/{pin}
    apiServer.http().on("^\\/api\\/gpio\\/(\\d+)$", HTTP_GET,
        [](AsyncWebServerRequest* req) {
            int pin = req->pathArg(0).toInt();
            if (!validatePin(req, pin)) return;
            JsonDocument doc;
            doc["pin"] = pin;
            doc["value"] = digitalRead(pin);
            doc["configured"] = pinConfigured[pin];
            String json; serializeJson(doc, json);
            ApiServer::restRequestCount++;
            req->send(200, "application/json", json);
        }
    );

    // POST /api/gpio/{pin}
    apiServer.http().on("^\\/api\\/gpio\\/(\\d+)$", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            int pin = req->pathArg(0).toInt();
            if (!validatePin(req, pin)) return;
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) { ApiServer::sendError(req, 400, "Invalid JSON"); return; }
            int value = doc["value"] | 0;
            digitalWrite(pin, value ? HIGH : LOW);
            JsonDocument resp;
            resp["pin"] = pin; resp["value"] = value; resp["status"] = "written";
            String json; serializeJson(resp, json);
            ApiServer::restRequestCount++;
            req->send(200, "application/json", json);
        }
    );

    // POST /api/gpio/{pin}/mode
    apiServer.http().on("^\\/api\\/gpio\\/(\\d+)\\/mode$", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        NULL,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            int pin = req->pathArg(0).toInt();
            if (!validatePin(req, pin)) return;
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) { ApiServer::sendError(req, 400, "Invalid JSON"); return; }
            const char* mode = doc["mode"] | "input";
            if      (strcasecmp(mode, "input")          == 0) pinMode(pin, INPUT);
            else if (strcasecmp(mode, "output")         == 0) pinMode(pin, OUTPUT);
            else if (strcasecmp(mode, "input_pullup")   == 0) pinMode(pin, INPUT_PULLUP);
            else if (strcasecmp(mode, "input_pulldown") == 0) pinMode(pin, INPUT_PULLDOWN);
            else { ApiServer::sendError(req, 400, "Invalid mode"); return; }
            pinConfigured[pin] = true;
            JsonDocument resp;
            resp["pin"] = pin; resp["mode"] = mode; resp["status"] = "configured";
            String json; serializeJson(resp, json);
            ApiServer::restRequestCount++;
            req->send(200, "application/json", json);
        }
    );

    Serial.println("[API] GPIO endpoints registered");
}
