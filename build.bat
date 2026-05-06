@echo off
setlocal
set FQBN=esp32:esp32:esp32dev
set BOARD=esp32dev
set BUILD_DIR=%~dp0firmware\build\%BOARD%
set SKETCH=%~dp0firmware\esp32lab

echo [ESP32 Lab] Building for %FQBN%...
arduino-cli compile --fqbn %FQBN% --output-dir "%BUILD_DIR%" "%SKETCH%"
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    exit /b 1
)
echo [ESP32 Lab] OK: %BUILD_DIR%\esp32lab.ino.bin

rem ── Optional upload ───────────────────────────────────────────────────────
rem   build.bat COM3              upload via USB
rem   build.bat 192.168.0.x       upload via OTA (WiFi)

if "%~1"=="" goto :done

echo %~1 | findstr /r "^COM" >nul
if %errorlevel%==0 (
    echo [ESP32 Lab] Uploading via USB on %~1...
    arduino-cli upload --fqbn %FQBN% --port %~1 --input-dir "%BUILD_DIR%"
    if %errorlevel% neq 0 ( echo [ERROR] Upload failed. & exit /b 1 )
    echo [ESP32 Lab] Upload complete.
    goto :done
)

echo [ESP32 Lab] Uploading via OTA to http://%~1 ...
curl -s -X POST "http://%~1/api/system/update" -F "firmware=@%BUILD_DIR%\esp32lab.ino.bin"
echo.
echo [ESP32 Lab] OTA sent. Device is rebooting.

:done
endlocal
