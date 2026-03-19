
<p align="center">
  <img src="Logo.png" alt="IoT-Werkstatt Logo" width="350">
</p>

<div align="center">

[![MIT License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)
[![Arduino](https://img.shields.io/badge/platform-Arduino-blue.svg)](https://www.arduino.cc)
[![ESP32](https://img.shields.io/badge/ESP32-3.3.2-orange.svg)](https://github.com/espressif/arduino-esp32)
[![ESP8266](https://img.shields.io/badge/ESP8266-3.1.2-teal.svg)](https://github.com/esp8266/Arduino)
[![Education](https://img.shields.io/badge/education-maker-blueviolet.svg)](https://www.umwelt-campus.de/forschung/projekte/iot-werkstatt)

</div>


# IoT²-Werkstatt Plattform Bundle

**Lizenz**: 
- Eigene Libraries (portable/sketchbook/libraries/): **MIT**
- Third-Party-Anteile behalten ihre eigene Lizenzen: [THIRD-PARTY-NOTICES.md](https://github.com/klausgollmer/IoT-Werkstatt-IoTW-Bundle/blob/main/THIRD-PARTY-NOTICES.MD)

Dieses Repository ist Teil der **IoT²-Werkstatt**, einer **offenen Bildungsinitiative für Maker, Schulen, Lernende und Lehrkräfte**. Ziel ist es, kreative, experimentelle und alltagsnahe Zugänge zu **Elektronik, Programmierung** und dem **Internet of Things and Thinking (IoT²)** zu ermöglichen.  

Weitere **Informationen, Unterrichtsmaterialien und Beispielprojekte** auf der Homepage am Umwelt-Campus Birkenfeld der Hochschule Trier https://www.iot-werkstatt.de  

Quellcode für die **aktuelle Version des grafischen Codegenerators** findet sich in einem eigenen Github: https://github.com/klausgollmer/IoT-Werkstatt-Ardublock/tree/master

**Dieses Repository enthält die für eine Nutzung der Plattform notwendigen modifizierten Konfigurationsdateien und eigene Bibliotheken.**  
Die Basis bildet die portable Version der Arduino IDE 1.8.19 mit ESP-Packages, verschiedenen, auf den Codegenerator abgestimmten Bibliotheken und unserer Erweiterung der grafischen Programmieroberfläche Ardublock als Codegenerator. Eine **"Core-Injection"** beschleunigt den Übersetzungsvorgang auf leistungschwächerer Hardware (wie Raspberry Pi oder ältere Notebooks). Dabei wird ein vorcompilierter Core der ESP32 bzw. ESP8266 Packages genutzt. Anleitung zur Erstellung eigener .a -Files dazu im Ordner 'precompile_win'.   

**Die Releases enthalten "ready to play" Installationsfiles für Windows, Linux_x64 und Linux_ARM64 (Raspberry Pi).** Es wird empfohlen, die vorhandenen Bibliotheken und die ESP-Packages nicht manuell upzudaten, denn nur so kann "plug & play" des Codegenerators und der "Core-Injection" garantiert werden. Unten sind die dabei verwendeten Third-Party Komponenten aufgeführt.

## Installation
Die portable Version besteht aus einem /IoTW/ mit entsprechenden Unterverzeichnissen und ist "ready to play".  
**Schnellstartanleitung:** https://www.umwelt-campus.de/iot-werkstatt/tutorials/schnellstart-makey   
Die neuste Version des Code-Generators kann separat installiert werden: https://github.com/klausgollmer/IoT-Werkstatt-Ardublock/releases

## Verzeichnisstruktur IoT²-Werkstatt /IoTW/
|Verzeichnis       | Inhalt                                      |
|-------------------|---------------------------------------------|
| `arduino/`        | portable Arduino IDE 1.8.19                          |
| `arduino/java/`   | Eclipse Adoptium Temurin 19.0.1             |
| `arduino/portable/` | Board-Packages, Bibliotheks-Quellcodes   |
| `resources/`      | Quellen und Lizenzen                        |
| `tutor/`          | Links und Beispiele AI-Tutor (IoT-Werkstatt)|
| `user/`           | Anwenderverzeichnis, Home für Ardublock-Dateien |
| `precompile_win/` | Powershell zum Precompiling der Libs/Cores  |
| `examples/`       | Beispiele                                   |
| `usb_driver/`     | USB virtueller COM-Port, Silicon Labs       |
| `nsis/`           | NSIS-Skripte zur Erzeugung des Windows-Installer (.exe)   |

## Build - vom Repro zum Bundle
- Arduino IDE 1.8.19 Portable Version für die jeweilige Plattform installieren https://www.arduino.cc/en/software/  
- arduino-1.8.19 umbenennen in arduino und in Verzeichnis /IoTW/ verschieben (Wichtig: Windows hat Probleme mit langen Pfaden, IoTW möglichst im root)  
- Java Verzeichnis in IoTW/arduino/java/ durch Version 19.0.1 ersetzen https://adoptium.net/de/temurin/releases?version=19&os=any&mode=filter  
- Arduino starten, Boardverwalter, ESP32 Espressif Systems 3.3.2 und ESP8266 Community 3.1.2 installieren  
- Das Verzeichnis IoTW anschließend um die Einstellungsfile in diesem Repro ergänzen (Für Win oder Linux, Java Einstellungen, Boards.txt, Plattform.local.txt, Preferences werden überschrieben)  
- Anwendungslibraries von Hand nstallieren (Liste in \resources\THIRD-PARTY-NOTICES.pdf), ggf. Core precompilieren 

## Third Party Notices
**Complete details**: [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)  
**License files**: `resources/licences/`

## Autor
Klaus-Uwe Gollmer  
- Zusammenstellung portable Installation
- HighRes Monitore JRE Temurin 
- Modification LMIC LoRa für energiesparenden Betrieb (light/deep sleep)
- Core-Injection und precompiled Libraries
- Boardkonfiguration Octopus (ESP8266) und MakeyLab (ESP32) 



# English Version
**License**:
- Own Libraries (`portable/sketchbook/libraries/`): **MIT**
- Third-Party components retain their original licenses: [THIRD-PARTY-NOTICES.md](https://github.com/klausgollmer/IoT-Werkstatt-IoTW-Bundle/blob/main/THIRD-PARTY-NOTICES.MD)

This repository is part of the **IoT² Workshop**, an **open educational initiative for makers, schools, learners, and teachers**. The goal is to enable creative, experimental, and everyday approaches to **electronics, programming**, and **Internet of Things and Thinking (IoT²)**.

More **information, teaching materials, and projects** on the homepage at Umwelt-Campus Birkenfeld, Trier University of Applied Sciences: [https://www.iot-werkstatt.de](https://www.iot-werkstatt.de).  
Source code for the **current version of the graphical code generator** can be found in a dedicated GitHub repository: https://github.com/klausgollmer/IoT-Werkstatt-Ardublock/tree/master

**This repository contains the modified configuration files, settings, and custom libraries required for platform usage.**  
The base is the portable Arduino IDE 1.8.19 with ESP packages, various libraries tuned to the code generator, and our extension of the graphical Ardublock programming interface as code generator. A **"Core-Injection"** accelerates compilation on lower-performance hardware (Raspberry Pi or older notebooks) using precompiled ESP32/ESP8266 cores. Instructions for creating .a files are in the `precompile_win` folder.

**Releases provide "ready to play" installation files for Windows, Linux_x64, and Linux_ARM64 (Raspberry Pi).** Do not manually update libraries or ESP packages to ensure "plug & play" functionality of the code generator and Core-Injection. Third-party components used are listed below.

## Installation
The portable version consists of `/IoTW/` with subdirectories.  
**Quick start guide**: [https://www.umwelt-campus.de/iot-werkstatt/tutorials/schnellstart-makey](https://www.umwelt-campus.de/iot-werkstatt/tutorials/schnellstart-makey)  
The latest version of the code generator can be installed separately: https://github.com/klausgollmer/IoT-Werkstatt-Ardublock/releases  
For language settings use Arduino IDE preferences (english/german only)

## Directory Structure IoT² Workshop /IoTW/
| Directory          | Content                                      |
|--------------------|----------------------------------------------|
| `arduino/`         | Portable Arduino IDE 1.8.19                  |
| `arduino/java/`    | Eclipse Adoptium Temurin 19.0.1              |
| `arduino/portable/`| Board packages, library source codes         |
| `resources/`       | Sources and licenses                         |
| `tutor/`           | AI Tutor links and examples (IoT Workshop)   |
| `user/`            | User directory, home for Ardublock files     |
| `precompile_win/`  | PowerShell for precompiling libs/cores       |
| `examples/`        | Examples                                     |
| `usb_driver/`      | USB virtual COM port, Silicon Labs           |
| `nsis/`            | NSIS scripts for Windows installer (.exe)    |

## Build - From Repo to Bundle
- Install Arduino IDE 1.8.19 portable version for your platform [https://www.arduino.cc/en/software/](https://www.arduino.cc/en/software/)  
- Rename `arduino-1.8.19` to `arduino` and move to `/IoTW/` (Windows: avoid long paths, place IoTW near root)  
- Replace `arduino/java/` with Temurin 19.0.1 [https://adoptium.net/de/temurin/releases?version=19](https://adoptium.net/de/temurin/releases?version=19)  
- Start Arduino, install ESP32 (Espressif 3.3.2) and ESP8266 (Community 3.1.2) via Board Manager  
- Add config files from this repo (Win/Linux settings, boards.txt, platform.local.txt, preferences get overwritten)  
- Manually install application libraries (list in `resources/THIRD-PARTY-NOTICES.pdf`), optionally precompile cores

## Third Party Notices
**Complete details**: [THIRD-PARTY-NOTICES.MD](THIRD-PARTY-NOTICES.MD)  
**License files**: `resources/licences/`

## Author
Klaus-Uwe Gollmer  
- Portable installation assembly  
- HighRes monitor JRE Temurin  
- LMIC LoRa modifications for energy-saving operation (light/deep sleep)  
- Core-Injection and precompiled libraries  
- Board configuration Octopus (ESP8266) and MakeyLab (ESP32)
