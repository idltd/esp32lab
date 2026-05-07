#include "board.h"
#include "config.h"

static String _name       = "ESP32";
static int    _gpioMax    = 39;
static int    _defaultLed = -1;
static int    _ledOn      = HIGH;
static bool   _reserved[50] = {};

static void mark(int pin) {
    if (pin >= 0 && pin < 50) _reserved[pin] = true;
}

static void markAll(const int* pins, int n) {
    for (int i = 0; i < n; i++) mark(pins[i]);
}

void boardSetup() {
    _name = ESP.getChipModel();

    mark(GROVE_D_PIN);
    mark(GROVE_D2_PIN);

    if (_name == "ESP32-C3") {
        _gpioMax = 21; _defaultLed = 8; _ledOn = LOW;
        const int r[] = {2,8,9,11,12,13,14,15,16,17,18,19};
        markAll(r, sizeof(r)/sizeof(r[0]));

    } else if (_name == "ESP32-C6") {
        _gpioMax = 30; _defaultLed = 8; _ledOn = LOW;
        const int r[] = {8,9,12,13,14,15,16,17};
        markAll(r, sizeof(r)/sizeof(r[0]));

    } else if (_name == "ESP32-S2") {
        _gpioMax = 46; _defaultLed = 2; _ledOn = HIGH;
        const int r[] = {0,19,20,26,27,28,29,30,31,32,45,46};
        markAll(r, sizeof(r)/sizeof(r[0]));

    } else if (_name == "ESP32-S3") {
        _gpioMax = 48; _defaultLed = 2; _ledOn = HIGH;
        const int r[] = {0,3,19,20,26,27,28,29,30,31,32,45,46};
        markAll(r, sizeof(r)/sizeof(r[0]));

    } else {
        // Classic ESP32 (and safe fallback for unknown chips)
        _gpioMax = 39; _defaultLed = 2; _ledOn = HIGH;
        const int r[] = {0,1,2,3,6,7,8,9,10,11,12,15};
        markAll(r, sizeof(r)/sizeof(r[0]));
    }

    Serial.printf("[Board] %s — GPIO 0-%d, default LED GPIO%d\n",
                  _name.c_str(), _gpioMax, _defaultLed);
}

String boardName()             { return _name; }
int    boardGpioMax()          { return _gpioMax; }
bool   boardPinReserved(int p) { return p < 0 || p >= 50 || _reserved[p]; }
int    boardDefaultLedPin()    { return _defaultLed; }
int    boardLedOn()            { return _ledOn; }
