REM Setzt den relativen Pfad zur .ino Datei
SET RELATIVE_INO_FILE_PATH=arduino\portable\sketchbook\IoT-Werkstatt\IoT-Werkstatt.ino

REM Setzt den relativen Pfad zur .jar Datei
SET RELATIVE_JAR_FILE_PATH=arduino\portable\sketchbook\tools\ArduBlockTool\tool\ardublock-MakeyLab.jar

REM Setzt den relativen Pfad zur java runtime
SET RELATIVE_JAVA_FILE_PATH=arduino\java\bin

REM Absoluten Pfad zur Arduino IDE 2 berechnen
SET ARDUINO_IDE_PATH="%~dp0%RELATIVE_ARDUINO_IDE_PATH%"

REM Absoluten Pfad zur .ino Datei berechnen
SET INO_FILE_PATH="%~dp0%RELATIVE_INO_FILE_PATH%"

REM Absoluten Pfad zur .jar Datei berechnen
SET JAR_FILE_PATH="%~dp0%RELATIVE_JAR_FILE_PATH%"

REM Überprüfen, ob die .ino Datei existiert
REM IF NOT EXIST "%INO_FILE_PATH%" (
REM     echo Die .ino Datei wurde unter dem angegebenen Pfad nicht gefunden: "%INO_FILE_PATH%"
REM     exit /b 1
REM )

REM Arduino IDE mit der .ino Datei öffnen
rem echo Starte Arduino IDE mit der Datei: "%INO_FILE_PATH%"
rem start "" "%ARDUINO_IDE_PATH%" "%eINO_FILE_PATH%"


echo Starte Werkstatt .jar: "%INO_FILE_PATH%"
set JAVA_HOME=%~dp0%RELATIVE_JAVA_FILE_PATH%
set PATH=%JAVA_HOME%\bin;%PATH%
echo java home "%JAVA_HOME%"
echo java home "%JAR_FILE_PATH%"

start /B javaw -jar %JAR_FILE_PATH% %INO_FILE_PATH% 
rem start /B javaw -Dsun.java2d.uiScale=1.2 -jar %JAR_FILE_PATH% %INO_FILE_PATH%

