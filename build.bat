@echo off
setlocal

rem ── ESP32 Lab — unified build script ─────────────────────────────────────────
rem
rem   build.bat                        Build both boards
rem   build.bat c3                     Build ESP32-C3 Mini only
rem   build.bat devkit                 Build ESP32 DevKit only
rem   build.bat c3 COM3                Build C3 + upload via USB
rem   build.bat c3 192.168.0.117       Build C3 + upload via OTA
rem   build.bat devkit COM3            Build DevKit + upload via USB
rem   build.bat devkit 192.168.0.117   Build DevKit + upload via OTA

set SKETCH=%~dp0firmware\esp32lab
set BOARD=%~1
set TARGET=%~2

if /i "%BOARD%"=="c3"     goto :only_c3
if /i "%BOARD%"=="devkit" goto :only_devkit
if    "%BOARD%"==""       goto :both

echo [ERROR] Unknown board "%BOARD%". Use: c3  devkit  (or omit for both)
exit /b 1

:both
call :build_board esp32:esp32:esp32c3 esp32c3 ""
if %errorlevel% neq 0 exit /b 1
call :build_board esp32:esp32:esp32   esp32dev ""
exit /b %errorlevel%

:only_c3
call :build_board esp32:esp32:esp32c3 esp32c3 "%TARGET%"
exit /b %errorlevel%

:only_devkit
call :build_board esp32:esp32:esp32 esp32dev "%TARGET%"
exit /b %errorlevel%

rem ── :build_board <fqbn> <board-dir> <upload-target> ──────────────────────────
:build_board
set _FQBN=%~1
set _DIR=%~dp0firmware\build\%~2
set _TARGET=%~3

echo.
echo [ESP32 Lab] Building for %_FQBN%...
arduino-cli compile --fqbn %_FQBN% --output-dir "%_DIR%" "%SKETCH%"
if %errorlevel% neq 0 ( echo [ERROR] Build failed. & exit /b 1 )
echo [ESP32 Lab] OK: %_DIR%\esp32lab.ino.bin

if "%_TARGET%"=="" goto :eof

echo %_TARGET% | findstr /r "^COM" >nul
if %errorlevel%==0 (
    echo [ESP32 Lab] Uploading via USB on %_TARGET%...
    arduino-cli upload --fqbn %_FQBN% --port %_TARGET% --input-dir "%_DIR%"
    if %errorlevel% neq 0 ( echo [ERROR] Upload failed. & exit /b 1 )
    echo [ESP32 Lab] Upload complete.
    goto :eof
)

echo [ESP32 Lab] Uploading via OTA to http://%_TARGET% ...
curl -s -X POST "http://%_TARGET%/api/system/update" -F "firmware=@%_DIR%\esp32lab.ino.bin"
echo.
echo [ESP32 Lab] OTA sent. Device is rebooting.

goto :eof
endlocal
