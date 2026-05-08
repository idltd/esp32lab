#pragma once

// ============================================================================
// ESP32 Lab — Configuration
// Board-specific settings (GPIO limits, LED pin) are auto-detected at boot
// by board.cpp using ESP.getChipModel(). No per-board editing needed here.
// ============================================================================

// --- Hotspot (AP mode) ---
// Used when no WiFi credentials are saved, or saved credentials fail.
#define WIFI_AP_SSID       "ESP32Lab"
#define WIFI_AP_PASSWORD   "esp32lab"    // min 8 chars
#define WIFI_AP_CHANNEL    1
#define WIFI_AP_MAX_CONN   4

// --- Station mode ---
#define WIFI_STA_TIMEOUT_S 20

// --- HTTP Server ---
#define HTTP_PORT          80

// --- Grove / Sensor Port ---
#define GROVE_D_PIN    4    // Primary data / signal pin
#define GROVE_D2_PIN   5    // Secondary data pin (HC-SR04 echo, rotary DT)

// --- Firmware Version ---
#define FIRMWARE_VERSION   "1.6.1"
