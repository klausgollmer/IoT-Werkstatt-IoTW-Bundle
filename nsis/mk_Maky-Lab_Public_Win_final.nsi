;--------------------------------------------------------------
; MakeyLab-Installer
;--------------------------------------------------------------
!include "MUI2.nsh"
!include "x64.nsh"  ; Hilfs-Makros für RunningX64
Unicode true


; RequestExecutionLevel admin   ; Installer läuft immer mit UAC
RequestExecutionLevel user

; --------------------------------------------------------------
; Modern UI Einstellungen für Bilder (Installer-Design)
; --------------------------------------------------------------
!define MUI_ABORTWARNING   ; Warnung beim Abbrechen der Installation
!define MUI_FINISHPAGE_NOAUTOCLOSE  ; Stellt sicher, dass sich die Installationsseite nicht zu früh schließt
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "MakeyHeader.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "MakeyWelcome.bmp"
!define MUI_WELCOMEPAGE_TITLE "Willkommen zu Makey:Lab"
!define MUI_WELCOMEPAGE_TEXT "Willkommen im Club der Zukunftsgestalter! $\r$\n $\r$\nEine Initiative des Umwelt-Campus Birkenfeld der Hochschule Trier und des Make-Magazins aus dem Heise-Verlag. $\r$\n$\r$\nDieses Installationsprogramm hilft dabei, Makey:Lab auf dem PC zu installieren."
Caption "Makey:Lab - Installation ..."
!define MUI_FINISHPAGE_TITLE "Installation erfolgreich!"
!define MUI_FINISHPAGE_TEXT "Lernen wir nun gemeinsam das Internet der Dinge kennen und starten wir endlich durch, unsere Ideen nicht nur zu denken, sondern auch zu realisieren. $\r$\n$\r$\nAb sofort begleiten wir Dich von der Schule bis in den Beruf oder gar ein Studium.$\r$\n$\r$\nFalls Du nach der Schule ein Studium am Umwelt-Campus der Hochschule Trier beginnst, kannst Du Deine Makey:Lab Projekte als Vorleistung anerkennen lassen. $\r$\n$\r$\nWeitere Informationen findest Du unter www.iot-werkstatt.de"
!define MUI_TEXT_INSTALLING "Installiere Dateien..."
!define MUI_TEXT_FINISH_TITLE "Installation abgeschlossen!"

!define MUI_PAGE_HEADER_TEXT "Installationsverzeichnis"
!define MUI_PAGE_HEADER_SUBTEXT "Wählen Sie ein Verzeichnis für Makey:Lab."

Name "MakeyLab-Installer"
Icon "MakeyLab.ico"
OutFile "Win_MakeyLab-InstallerV1_0.exe"


; Installationsverzeichnis: C:\Users\<Name>\IoTW
; InstallDir "$PROFILE\IoTW"

; Installationsverzeichnis: C:\IoTW
; InstallDir "c:\IoTWPub0123456789"

; Installationsverzeichnis: C:\Users\Public
InstallDir "C:\Users\Public\IoTW"

; --------------------------------------------------------------
; Seiten (Installer UI Struktur)
; --------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
Page directory     ; Verzeichniswahl (zeigt standardmäßig z. B. "C:\IoTW")
Page instfiles     ; Installationsfortschritt
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"
!macro RemovePathIfExists PATH
  ; eindeutige ID pro Makro-Expansion
  !define _RPIE_ID ${__LINE__}

  ; 1) nur Kopfzeile
  SetDetailsPrint textonly
  DetailPrint "Clean: ${PATH}"

  ; 2) während der Arbeit nichts anzeigen
  SetDetailsPrint none

  ; Ordner?
  IfFileExists "${PATH}\*.*" 0 +5
    nsExec::ExecToStack 'cmd /c attrib -R /S /D "${PATH}\*.*"'
    RMDir /r "${PATH}"
    Goto done_${_RPIE_ID}

  ; Datei?
  IfFileExists "${PATH}" 0 +3
    SetFileAttributes "${PATH}" NORMAL
    Delete "${PATH}"
    Goto done_${_RPIE_ID}

  ; nichts zu tun

done_${_RPIE_ID}:
  ; 3) Abschlusszeile
  SetDetailsPrint textonly
  DetailPrint "✓ Fertig: ${PATH}"

  ; 4) vorherige Einstellung wiederherstellen
  ;SetDetailsPrint lastused

  !undef _RPIE_ID
!macroend

;--------------------------------------------------------------
; Section: Installation
;--------------------------------------------------------------
Section "Install IoTWPub Files" SEC01

  ; Verzeichnis setzen und Files installieren
SetOutPath "$INSTDIR"
DetailPrint "lösche alte Files "

!insertmacro RemovePathIfExists "$INSTDIR\arduino"
!insertmacro RemovePathIfExists "$INSTDIR\resources"
!insertmacro RemovePathIfExists "$INSTDIR\examples"
!insertmacro RemovePathIfExists "$INSTDIR\tutor"
!insertmacro RemovePathIfExists "$INSTDIR\usb_driver"

DetailPrint "kopiere neue Files "
SetDetailsPrint none
 File /r "E:\IoTWPub0123456789\*.*"



; 2. Bildschirmauflösung ermitteln (VERTIKALE Auflösung)

ReadRegDWORD $R0 HKCU "Control Panel\\Desktop\\WindowMetrics" "AppliedDPI"
DetailPrint "AppliedDPI: $R0"

; Default = 96
IntCmp $R0 0 setDPI setDPI skip_setDPI

setDPI:
  StrCpy $R0 96
  Goto skip_setDPI

skip_setDPI:

System::Call 'user32::GetSystemMetrics(i 1) i .r1'
DetailPrint "Vor der Berechnung - Logische Höhe: $1, DPI-Wert: $R0"
; physische Höhe = logische Höhe * (AppliedDPI/96)
; Man macht es in einer Rechenoperation. In NSIS kann man nicht so easy float, also Trick:
;   $phys = $1 * $R0 / 96
IntOp $3 $1 * $R0  ; $3 = 1440*168 = 241920
IntOp $1 $3 / 96   ; $1 = 241920/96 = 2520
IntOp $1 $1 + 1    ; aufrunden
DetailPrint "Endergebnis: $1"

  ; Low-Resolution: Weniger als 1080px Höhe -> lowres
  IntCmp $1 1080 check_midres run_lowres check_midres

  check_midres:
  ; Mid-Resolution: Zwischen 1080px und 1919px -> midres, darüber high_res
  IntCmp $1 1919 run_highres run_midres run_highres

  run_lowres:
    DetailPrint "Low-Resolution erkannt. Starte IoT-Werkstatt-set-lowres.bat..."
    ExecWait '"$INSTDIR\IoT-Werkstatt-set-lowres.bat"'
    ;Exec '"$WINDIR\System32\cmd.exe" /C ""$INSTDIR\IoT-Werkstatt-set-lowres.bat"" & exit"'
    Goto after_resolution

  run_midres:
    DetailPrint "Mid-Resolution erkannt. Starte IoT-Werkstatt-set-midres.bat..."
    ;Exec '"$WINDIR\System32\cmd.exe" /C ""$INSTDIR\IoT-Werkstatt-set-midres.bat"" & exit"'
    ExecWait '"$INSTDIR\IoT-Werkstatt-set-midres.bat"'
    Goto after_resolution

  run_highres:
    DetailPrint "High-Resolution erkannt. Starte IoT-Werkstatt-set-highres.bat..."
    ExecWait '"$INSTDIR\IoT-Werkstatt-set-highres.bat"'
    ;Exec '"$WINDIR\System32\cmd.exe" /C ""$INSTDIR\IoT-Werkstatt-set-highres.bat"" & exit"'
    Goto after_resolution

  after_resolution:
    DetailPrint "Auflösungsabhängige Konfiguration abgeschlossen."


  ; 3. Buildcache-Skript ausführen
  ; DetailPrint "Starte Buildcache-Skript..."
  ; ExecWait '"$INSTDIR\IoT-Werkstatt-set-public-buildcache.bat"'


  
  ; 4. Desktop-Verknüpfung anlegen:
  ;    - Ziel: "$INSTDIR\arduino\arduino.exe"
  ;    - Arbeitsverzeichnis: "$INSTDIR\arduino"
  ;    - Icon: "$INSTDIR\Makey.ico"
  DetailPrint "Erstelle Desktop-Verknüpfung MakeyLab..."

SetOutPath "$INSTDIR\arduino"
CreateShortCut "$DESKTOP\MakeyLab.lnk" "$INSTDIR\arduino\arduino.exe" "" "$INSTDIR\MakeyLab.ico"

; Erstellt eine zweite Verknüpfung im Installationsverzeichnis
CreateShortCut "$INSTDIR\MakeyLab.lnk" "$INSTDIR\arduino\arduino.exe" "" "$INSTDIR\MakeyLab.ico" 

SectionEnd

Section "Treiberinstallation"
 ; suche ob es treiber

  ; Erstmal den Standardpfad (System32) testen
  ;  DetailPrint "Teste Zugriff auf pnputil.exe in System32..."
    IfFileExists "$WINDIR\System32\pnputil.exe" 0 try_sysnative

    DetailPrint "PnPUtil wurde in System32 gefunden!"
    StrCpy $R1 "$WINDIR\System32\pnputil.exe"
    Goto run_pnputil

try_sysnative:
    ; Falls System32 nicht gefunden wurde, versuche SysNative (für 32-Bit-NSIS auf 64-Bit-Windows)
   ; DetailPrint "Teste Zugriff auf pnputil.exe in SysNative..."
    IfFileExists "$WINDIR\SysNative\pnputil.exe" 0 pnputil_not_found

    DetailPrint "PnPUtil wurde in SysNative gefunden!"
    StrCpy $R1 "$WINDIR\SysNative\pnputil.exe"
    Goto run_pnputil

pnputil_not_found:
   ; DetailPrint "Fehler: pnputil.exe wurde nicht gefunden!"
    MessageBox MB_OK "pnputil.exe wurde nicht gefunden! Stelle sicher, dass du auf einem Windows 10/11 oder Server 2016+ System bist."
    Goto done

run_pnputil:
   ; DetailPrint "PnPUtil wird ausgeführt: $R1"
done:

;DetailPrint "🔍 Suche mit `findstr`..."
ExecWait '"$WINDIR\System32\cmd.exe" /C ""$R1" /enum-drivers | findstr /Ri "\<Silicon Laboratories\> \<silicon labs\> \<slabvcp.inf\>""' $R0
DetailPrint "Exitcode von findstr: $R0"

; Wenn Exitcode == 0 -> Treiber ist installiert
IntCmp $R0 0 driver_found driver_not_found driver_not_found

driver_found:
  DetailPrint "Treiber ist bereits vorhanden. Keine Installation nötig."
  Goto doneDriver

driver_not_found:
  DetailPrint "Treiber fehlt, Installation wird gestartet..."

; 2) Prüfen, ob 64-Bit- oder 32-Bit-Windows
  ${If} ${RunningX64}
    DetailPrint "64-Bit-OS erkannt. Starte x64-Installer..."
;    ExecWait '"$INSTDIR\usb_driver\CP210x_VCP_Windows\CP210xVCPInstaller_x64.exe" /q' $R0
    ExecShell "runas" "$INSTDIR\usb_driver\CP210x_VCP_Windows\CP210xVCPInstaller_x64.exe"

  ${Else}
    DetailPrint "32-Bit-OS erkannt. Starte x86-Installer..."
;    ExecWait '"$INSTDIR\usb_driver\CP210x_VCP_Windows\CP210xVCPInstaller_x86.exe" /q' $R0
    ExecShell "runas" "$INSTDIR\usb_driver\CP210x_VCP_Windows\CP210xVCPInstaller_x64.exe"
  ${EndIf}



  ; Exitcode auswerten
  DetailPrint "Exitcode: $R0"
  ; Falls nötig, kannst du hier checken, ob $R0 == 0

doneDriver:
  DetailPrint "Treiberinstallation abgeschlossen (falls nötig)."

SectionEnd


Function .onInstSuccess
    MessageBox MB_YESNO|MB_ICONQUESTION "Makey:Lab jetzt starten?" IDNO no_start
    ExecShell "open" "$INSTDIR\arduino\arduino.exe"

no_start:
FunctionEnd

