# Third Party Notices - IoT² Workshop Platform

### Code Generator
The **IoT² Workshop Code Generator** extends the Ardublock project (**GNU General Public License v3.0**). Ardublock uses OpenBlocks (refactoring branch) for graphical display (**MIT**). Source code of our extended version: [https://github.com/klausgollmer/IoT-Werkstatt-Ardublock](https://github.com/klausgollmer/IoT-Werkstatt-Ardublock). **Generated C code**: **MIT License** unless additional libraries are included. License notices from integrated libraries appear in the generated code.

### Portable Installation
ZIP/Installer contains Arduino IDE 1.8.19, ESP32/ESP8266 packages, and libraries with individual licenses. Details in `resources/` or below.  
Arduino uses GNU avr-gcc, avr-libc, avrdude etc. (see Arduino license).  
**Arduino IDE 1.8.19**: GPL [https://github.com/arduino/Arduino](https://github.com/arduino/Arduino)  
**Java Runtime**: Eclipse Temurin (EPL-2.0) [https://adoptium.net/de/](https://adoptium.net/de/)  
**ESP8266 Package**: LGPL 2.1 [https://github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)  
**ESP32 Package**: LGPL 2.1 [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)  
**USB Driver**: CP210x Silicon Labs VCP Driver (**proprietary EULA**) [https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)  
**Installer**: NSIS (Nullsoft Scriptable Install System)

## Bundle Components
| Interface | Library | Lizenz | Autor/Quelle |
|-----------|---------|--------|-------------|
| Umweltdaten BME280 | SparkFun_BME280_Arduino_Library | **MIT** | Marshall Taylor @ SparkFun, https://github.com/sparkfun/SparkFun_BME280_Arduino_Library |
| Umweltdaten BME680 | AdafruitBME680 | **BSD** | Limor Fried/Ladyada, https://github.com/adafruit/Adafruit_BME680 |
| | Adafruit_Unified_Sensor | **Apache 2.0** | Adafruit, https://github.com/adafruit/Adafruit_Sensor |
| Umweltdaten BSEC | Bosch BSEC2/BME68x | **BSD-3-Clause** | Bosch Sensortec, https://github.com/boschsensortec/Bosch-BSEC2-Library |
| Beschleunigung | AdafruitBNO055 | **MIT** | Kevin (KTOWN), https://github.com/adafruit/Adafruit_BNO055 |
| Beschleunigung | Accelerometer_MMA7660 | **MIT** | Frankie Chu, Seeed Studio, https://github.com/Seeed-Studio/Accelerometer_MMA7660 |
| RGB-LED | Adafruit NeoPixel | **LGPL-3** | Phil Burgess, Adafruit https://github.com/adafruit/Adafruit_NeoPixel |
| RGB-LED | Adafruit WS2801 | **MIT** | Limor Fried/Ladyada, https://github.com/adafruit/Adafruit-WS2801-Library |
| NFC/RFID | Seeed PN532 | **BSD-3-Clause** | Seeed Studio, https://github.com/Seeed-Studio/PN532 |
| Gassensor Multichannel | Seeed Multichannel Gas | **GNU** | Jacky Zhang, Seeed, https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor|
| Dreh/Drück Rotary-Encoder | Paul Stoffregen Encoder | **MIT** | Paul Stoffregen, https://github.com/PaulStoffregen/Encoder |
| | ESP32Encoder | **MIT** | Kevin Harrington, https://github.com/madhephaestus/ESP32Encoder/ |
| LED-Matrix Charlieplex | Adafruit IS31FL3731 | **MIT** | Adafruit, https://github.com/adafruit/Adafruit_IS31FL3731 |
| Textausgabe | Adafruit GFX V1.1.5 | **BSD** | Adafruit, https://github.com/adafruit/Adafruit-GFX-Library |
| MQTT | PubSubClient v2.6 | **MIT** | Nick O'Leary, github.com/knolleary/pubsubclient/releases/tag/v2.6 |
| GSM Mobilfunk | TinyGSM 0.1.7 | **LGPL-3** | Volodymyr Shymanskyy, https://github.com/vshymanskyy/TinyGSM |
| OTA | ArduinoOTA | **LGPL 2.1** | ESP8266 Arduino Core, https://github.com/esp8266/Arduino |
| LoRaWAN LMIC | MCCI arduino-lmic | **MIT** | MCCI Corporation (kgo mods), https://github.com/mcci-catena/arduino-lmic |
| Digital I/O Extender | SparkFun SX1509 | **Open Source** | Jim Lindblom, https://github.com/sparkfun/SparkFun_SX1509_Arduino_Library |
| Motoren | Adafruit Motor Shield V2 | **BSD** | Adafruit, https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library |
| 7-Segment Display | Adafruit LED Backpack | **MIT** | Adafruit, https://github.com/adafruit/Adafruit_LED_Backpack |
| LCD Grove | Seeedstudio LCD RGB | **MIT** | Loovee, https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight/ |
| RGB/Gesten/Licht | Adafruit APDS9960 | **BSD** | Adafruit, https://github.com/adafruit/Adafruit_APDS9960 |
| CO2 | SparkFun SCD30 | **MIT** | Nathan Seidle, https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library  |
| CO2 | SparkFun SCD4x | **MIT** | Nathan Seidle, https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library  |
| CO2 | MH-Z19 | **LGPL 3.0** | Jonathan Dempsey, https://github.com/WifWaf/MH-Z19 |
| Feinstaub | Seeed HM3301 | **MIT** | Seeed Studio, https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301 |
| Feinstaub | Honeywell HPMA115S0 | **LGPLv3** | dfrobot, Zuyang @ HUST, https://wiki.dfrobot.com/sen0177/ |
| Touch-Sensor | Adafruit MPR121 | **MIT** | Adafruit, Limor / Fried, https://github.com/adafruit/Adafruit_MPR121 |
| InfluxDB | InfluxDB-Client-for-Arduino | **MIT** | Tobias Schürg, https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino |
| WiFi-Sniffer PaxCounter | SensorIot Wi-Fi Sniffer | **MIT OpenSource** | Spiess, Burnette https://github.com/SensorsIot/Wi-Fi-Sniffer-as-a-Human-detector |
| CurveFit Polynomfit | arduinoCurveFitting | **MIT** | Rotario, https://github.com/Rotario/arduinoCurveFitting |
| ESP8266Audio | ESP8266Audio | **GPL-3** | Earle F. Philhower, https://github.com/earlephilhower/ESP8266Audio |
| RF24 | RF24 | **GPL-2** | Brendan Doherty, https://github.com/nRF24/RF24/  |
| Hoymiles | Hoymiles DTU | **GPL-3** | hmsoft, https://github.com/hm-soft/Hoymiles-DTU-Simulation |
| Useful Sensor | Camera | **Apache-2** | https://github.com/moonshine-ai/person_sensor_arduino |
| ToF VL53L0X | Time of Flight | **OpenSource** | https://github.com/pololu/vl53l0x-arduino |
| OLED | Display | **BSD** | Ladyada, adafruit, https://github.com/adafruit/Adafruit_SH110x |
| GPS | Position | **BSD** | Limor/Fried/Ladyada, adafruit, https://github.com/adafruit/Adafruit_GPS |
| TCS34725 | RGB | **BSD** | adafruit, https://github.com/adafruit/Adafruit_TCS34725 |
| INA219 | Current | **BSD** | adafruit, https://github.com/adafruit/Adafruit_INA219 |
| ChatGPT | genAI | **MIT** | Eric Nam, https://github.com/0015/ChatGPT_Client_For_Arduino |
| JSON | ArduinoJSON | **MIT** | Benoît Blanchon, https://github.com/bblanchon/ArduinoJson |
| Serial | espsoftwareserial | **LGPL-2.1** | Dirk Kaar, Peter Lerup, https://github.com/plerup/espsoftwareserial |
| Licht | BH1750 | **MIT** | claws, https://github.com/claws/BH1750 |
| Licht | TSL2561 | **OpenSource** | Kevin (KTOWN) Townsend Adafruit, https://github.com/adafruit/Adafruit_TSL2561 |
| Gewicht | HX711 | **MIT** | Rob Tillaart, https://github.com/RobTillaart/HX711 |
| EDGEImpulse| EDGEImpulse | **Clear BSD License and other** | https://docs.edgeimpulse.com/docs/run-inference/arduino-library |
| Webserver SensorPlot| SensorPlot_WebInterface | **MIT** |Jason Rietzke, https://github.com/jason-rietzke/SensorPlot_WebInterface  |
| WiFi| WiFiManager | **MIT** | Tzapu, https://github.com/tzapu/WiFiManager |
| SAM| ESP8266SAM | **  ** | https://github.com/earlephilhower/ESP8266SAM |
| SMTP mail| ESP-mail-client | **MIT** | bobizt, https://github.com/mobizt/ESP-Mail-Client |
| Whatsapp| urlencode | **MIT** | Masayuki Sugahara, https://github.com/plageoj/urlencode |
| Temperature | Arduino-Temperature-Control-Library | **MIT** | Miles Burton, https://github.com/milesburton/Arduino-Temperature-Control-Library|
| Temperature | Arduino-Temperature-Control-Library | **MIT** | Miles Burton, https://github.com/milesburton/Arduino-Temperature-Control-Library|
| Phyphox Arduino | phyphox-arduino | **LGPL-3.0/GPL-3.0** | phyphox.org, https://github.com/phyphox/phyphox-arduino |
| Dynamixel Servos | Dynamixel2Arduino | **Apache 2.0** | ROBOTIS, https://github.com/ROBOTIS-GIT/Dynamixel2Arduino |
| Feetech Servos | FTServo_Arduino | **MIT** | ftservo, https://github.com/ftservo/FTServo_Arduino |

**Hinweis**: Vollständige Tabelle mit Links auf die im Bundle verwendeten Repros unter `resources/THIRD-PARTY-NOTICES.pdf`.   
Lizenztexte in `resources/licences/`bzw. unter den angegebenen Links. 

## Important Notes
- **Repository code** (configs, boards.txt, MIT libs): **MIT License** ([LICENSE](LICENSE))
- **Modified third-party components** retain their **original licenses** (documented above)
- **GPL/LGPL source availability**: Via linked repositories and IoT² Workshop forks
- **Full library list with repo links**: `resources/THIRD-PARTY-NOTICES.pdf`
- **Complete license texts**: `resources/licences/`

**Generated C code**: MIT unless libraries impose additional restrictions (notices in code).
