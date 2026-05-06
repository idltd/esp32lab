#pragma once
bool wifiManagerSetup();   // try saved credentials → AP fallback; returns true if STA
void setupWifiApi();        // register /api/wifi/* endpoints
