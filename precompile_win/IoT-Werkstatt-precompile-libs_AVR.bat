@echo off
setlocal

REM Ordner dieser BAT (IoTW)
for %%A in ("%~dp0..") do set "ROOT=%%~fA\"

echo ROOT ist: %ROOT%

REM Arbeitsverzeichnis auf arduino setzen (wichtig für JNA/IDE)
cd /d "%ROOT%arduino"

REM Portables Temp anlegen und JNA/Java drauf lenken (gegen Rechte-/Temp-Probleme)
if not exist "%ROOT%temp" mkdir "%ROOT%temp"
set "JNA_TMPDIR=%ROOT%temp"
set "TEMP=%ROOT%temp"
set "TMP=%ROOT%temp"
set "JAVA_TOOL_OPTIONS=-Djna.nosys=true -Djna.tmpdir=%ROOT%temp -Djava.io.tmpdir=%ROOT%temp"

REM Portable Java bevorzugen (falls vorhanden)
if exist "%ROOT%arduino\java\bin\java.exe" (
  set "PATH=%ROOT%arduino\java\bin;%PATH%"
)

echo.
echo ================= Zielplattform waehlen =================
echo [1] ESP32  Makey V1   (FQBN: esp32:esp32:makey_v1)
echo [2] ESP8266 Octopus   (FQBN: esp8266:esp8266:octopus)
echo [3] AVR    Arduino UNO (FQBN: arduino:avr:uno)
echo ========================================================
choice /C 123 /N /M "Auswahl (1/2/3): "
echo.

REM CHOICE setzt ERRORLEVEL: 1 fuer '1', 2 fuer '2', 3 fuer '3'
set "FQBN="
if errorlevel 3 set "FQBN=arduino:avr:uno"
if errorlevel 2 if not defined FQBN set "FQBN=esp8266:esp8266:octopus"
if errorlevel 1 if not defined FQBN set "FQBN=esp32:esp32:makey_v1"

echo Starte Precompile mit:
echo   FQBN = %FQBN%
echo.

echo libs in: "%ROOT%precompile_win\IoT-Werkstatt-precompile-libs_AVR.ps1"
echo arduino in: "%ROOT%arduino"

REM PowerShell-Skript mit PortableRoot starten
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass ^
  -File "%ROOT%precompile_win\IoT-Werkstatt-precompile-libs_AVR.ps1" ^
  -PortableRoot "%ROOT%arduino" ^
  -FQBN "%FQBN%"

set RC=%ERRORLEVEL%
echo.
echo === Fertig. Exitcode: %RC% ===
pause
endlocal & exit /b %RC%
