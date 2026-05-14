#pragma once
bool wifiManagerSetup();        // try saved credentials → AP fallback; returns true if STA
void wifiManagerLoop();         // must be called from loop() — processes DNS in AP mode
const String& getApSsid();      // returns the runtime AP SSID (ESP32Lab_XXXX)
void setupWifiApi();            // register /api/wifi/* endpoints
