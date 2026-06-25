@echo off
set CLI="%USERPROFILE%\bin\arduino-cli"
set PORT=%~1
if not "%PORT%"=="" goto :flash

echo [ESP32 Lab] Detecting port...
%CLI% board list > "%TEMP%\esp32lab_ports.txt"
for /f "tokens=1" %%p in ('findstr /i "USB" "%TEMP%\esp32lab_ports.txt"') do set PORT=%%p
if "%PORT%"=="" (
    echo [ERROR] No USB serial device found. Is the chip connected on a data cable?
    type "%TEMP%\esp32lab_ports.txt"
    exit /b 1
)

:flash
echo [ESP32 Lab] Flashing C3 on %PORT%...
%CLI% upload --fqbn esp32:esp32:esp32c3 --port %PORT% --input-dir "%~dp0firmware\build\esp32c3" "%~dp0firmware\esp32lab"
