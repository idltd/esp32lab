@echo off
:: ESP32 Lab — Install Arduino libraries
:: Run this once before building for the first time.

echo Installing ESP32 Lab libraries...
echo.

:: Core libraries (available in Library Manager)
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "DHTesp"
arduino-cli lib install "OneWire"
arduino-cli lib install "DallasTemperature"

:: ESPAsyncWebServer and AsyncTCP must be installed from git
:: (not available as versioned releases in Library Manager)
arduino-cli lib install --git-url https://github.com/mathieucarbou/ESPAsyncWebServer
arduino-cli lib install --git-url https://github.com/mathieucarbou/AsyncTCP

echo.
echo Done. You can now build with:
echo   arduino-cli compile --profile default firmware/esp32lab
echo.
