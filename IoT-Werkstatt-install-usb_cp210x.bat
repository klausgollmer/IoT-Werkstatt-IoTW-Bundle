@echo off
setlocal

REM Pfad zur 64-Bit Treiber-Installationsdatei
set "DRIVER_INSTALLER_X64=usb_driver/CP210x_VCP_Windows/CP210xVCPInstaller_x64.exe"
REM Pfad zur 32-Bit Treiber-Installationsdatei
set "DRIVER_INSTALLER_X86=usb_driver/CP210x_VCP_Windows/CP210xVCPInstaller_x86.exe"


REM Überprüfen, ob das Betriebssystem 64-Bit oder 32-Bit ist und entsprechenden Treiber installieren
if defined ProgramFiles(x86) (
    echo 64-Bit Betriebssystem erkannt.
    set "DRIVER_PATH=%~dp0%DRIVER_INSTALLER_X64%"
) else (
    echo 32-Bit Betriebssystem erkannt.
    set "DRIVER_PATH=%~dp0%DRIVER_INSTALLER_X86%"
)

echo Überprüfe die Existenz der Datei: "%DRIVER_PATH%"

if exist "%DRIVER_PATH%" (
    echo Datei existiert: "%DRIVER_PATH%"
) else (
    echo Datei existiert nicht: "%DRIVER_PATH%"
    goto :eof
)

REM Überprüfen, ob der Treiber installiert ist
echo Überprüfen, ob der CP210x Treiber installiert ist...
pnputil /enum-drivers | findstr /i "silicon labs" >nul 2>&1

if %errorlevel% equ 0 (
    echo Der CP210x Treiber ist bereits installiert.
) else (
    echo Der CP210x Treiber ist nicht installiert. Installation wird gestartet...
    REM Installation des Treibers
    echo Starte Treiberinstallation...
    "%DRIVER_PATH%" /q
    if %errorlevel% equ 0 (
        echo Die Treiberinstallation war erfolgreich.
    ) else (
        echo Fehler bei der Treiberinstallation.
    )
)

endlocal
pause
