#pragma once

// ============================================================================
// ESP32 Lab — Configuration
// ============================================================================

// --- WiFi Access Point ---
#define WIFI_AP_SSID       "ESP32Lab"
#define WIFI_AP_PASSWORD   "esp32lab"    // min 8 chars
#define WIFI_AP_CHANNEL    1
#define WIFI_AP_MAX_CONN   4

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
#define FIRMWARE_VERSION   "1.0.0"

// --- mDNS ---
#define MDNS_HOSTNAME      "esp32lab"
