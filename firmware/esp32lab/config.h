#pragma once

// ============================================================================
// ESP32 Lab — Configuration
// ============================================================================

// --- WiFi Mode ---
// WIFI_MODE_AP  : ESP32 creates its own hotspot. Connect your device to it.
//                 Works anywhere — no router needed.
// WIFI_MODE_STA : ESP32 joins your existing WiFi network.
//                 Accessible from any device on that network.
//                 Falls back to AP mode automatically if connection fails.
#define WIFI_MODE_AP   0
#define WIFI_MODE_STA  1

#define WIFI_MODE      WIFI_MODE_AP   // <-- change to WIFI_MODE_STA to join your network

// --- Hotspot (AP mode) ---
#define WIFI_AP_SSID       "ESP32Lab"
#define WIFI_AP_PASSWORD   "esp32lab"    // min 8 chars
#define WIFI_AP_CHANNEL    1
#define WIFI_AP_MAX_CONN   4

// --- Home/School Network (STA mode) ---
// Fill these in before switching to WIFI_MODE_STA
#define WIFI_STA_SSID      "YourNetworkName"
#define WIFI_STA_PASSWORD  "YourPassword"
#define WIFI_STA_TIMEOUT_S 20              // seconds to wait before giving up

// --- HTTP Server ---
#define HTTP_PORT          80

// --- Grove / Sensor Port ---
// Default pins for a generic ESP32 DevKit. Change these to match your wiring.
#define GROVE_D_PIN    4    // Primary data / signal pin   (yellow wire on Grove)
#define GROVE_D2_PIN   5    // Secondary data pin           (white wire on Grove)
                            // Used by: HC-SR04 echo, rotary encoder DT

// --- GPIO Safety ---
// The GPIO API will refuse to touch these pins.
//   0, 2, 12, 15 — boot-mode strapping pins
//   1, 3         — UART0 TX/RX (USB serial / programming)
//   6–11         — internal flash SPI — NEVER touch, will crash the board
//   4, 5         — reserved for Grove sensor port above
#define GPIO_MAX_PIN       39
static const int RESERVED_PINS[] = {
    0, 2, 12, 15,
    1, 3,
    6, 7, 8, 9, 10, 11,
    GROVE_D_PIN, GROVE_D2_PIN
};
#define RESERVED_PIN_COUNT (sizeof(RESERVED_PINS) / sizeof(RESERVED_PINS[0]))

// --- Firmware Version ---
#define FIRMWARE_VERSION   "1.1.0"

// --- mDNS ---
#define MDNS_HOSTNAME      "esp32lab"
