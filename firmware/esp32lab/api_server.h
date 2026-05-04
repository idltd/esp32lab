#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>

struct WsEndpoint {
    AsyncWebSocket* ws;
    const char* path;
};

class ApiServer {
public:
    void begin();
    void loop();

    AsyncWebSocket* addWebSocket(const char* path);
    static void broadcast(AsyncWebSocket* ws, const String& json);
    AsyncWebServer& http() { return _server; }
    int wsClientCount();

    static void sendError(AsyncWebServerRequest* req, int code, const char* msg);
    static void sendOk(AsyncWebServerRequest* req, const char* msg = "ok");

    static unsigned long restRequestCount;
    static unsigned long wsMessageCount;

private:
    AsyncWebServer _server{HTTP_PORT};
    std::vector<WsEndpoint> _wsEndpoints;

    static void _onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                           AwsEventType type, void* arg, uint8_t* data, size_t len);
};

extern ApiServer apiServer;
