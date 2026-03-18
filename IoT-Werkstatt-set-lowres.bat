cd arduino

@echo off
:: Zielverzeichnis und Dateiname
set "INI_PATH=arduino.l4j.ini"

:: Inhalt der Datei schreiben
echo -Xms128M > "%INI_PATH%"
echo -Xmx1G >> "%INI_PATH%"
echo -Dfile.encoding=UTF8 >> "%INI_PATH%"
echo -Dsun.java2d.uiScale=1.2 >> "%INI_PATH%"

:: Bestätigung ausgeben
::echo Die IoT-Werkstatt wurde fuer LowRes-Monitore optimiert
::pause

