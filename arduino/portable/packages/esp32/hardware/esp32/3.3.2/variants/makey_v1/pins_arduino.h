/*
Board: Makey_V1
Platform: ESP32
CPU: WROOM 
Sensors: BME 680 (Temp/Humidity/Pressure/Gas, I2C), APDS-9999 (Proximity/RGB, I2C), Micro (I2S, BCK=GPIO12, WS=GPIO2, SD=GPIO27), Rotary-Encoder (A=GPIO32, B=GPIO35, Press=GPIO15)
Actuators: NeoPixel (NEO_GRBW + NEO_KHZ800, right = index 0, left = index 1, GPIO13), Buzzer (DAC1)
Display: SH1107 (OLED 128x64, I2C)
Grove: Ain-Grove (GPIO34, GPIO39), I2C-Grove (GPIO21, GPIO22), Serial-Grove (GPIO16, GPIO16), Aout-Grove (GIOP26, GPIO26), Dio-Grove (GPIO33,GPIO14)
Feather-Wing: Socket-Headers
*/


#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

static const uint8_t TX = 1;
static const uint8_t RX = 3;

static const uint8_t SDA = 21;
static const uint8_t SCL = 22;

static const uint8_t SS = 5;
static const uint8_t MOSI = 23;
static const uint8_t MISO = 19;
static const uint8_t SCK = 18;


static const uint8_t A0 = 26;
static const uint8_t A1 = 25;
static const uint8_t A2 = 34;
static const uint8_t A3 = 39;
static const uint8_t A4 = 36;
static const uint8_t A5 = 4; //33;
static const uint8_t A6 = 35;
static const uint8_t A7 = 32;

static const uint8_t T0 = 4;
static const uint8_t T1 = 0;
static const uint8_t T2 = 2;
static const uint8_t T3 = 15;
static const uint8_t T4 = 13;
static const uint8_t T5 = 12;
static const uint8_t T6 = 14;
static const uint8_t T7 = 27;
static const uint8_t T8 = 33;
static const uint8_t T9 = 32;

static const uint8_t DAC1 = 25;
static const uint8_t DAC2 = 26;

#define PIN_SPI_SS   SS    // backward compatibility
#define PIN_SPI_MOSI MOSI  // backward compatibility
#define PIN_SPI_MISO MISO  // backward compatibility
#define PIN_SPI_SCK  SCK   // backward compatibility
#define PIN_A0 A0          // backward compatibility
#define PIN_WIRE_SDA SDA   // backward compatibility
#define PIN_WIRE_SCL SCL   // backward compatibility

// IoT-Werkstatt #kgo
#define IOTW_BOARD_MAKEY
#define IOTW_BOARD_MAKEY_V1


#define IOTW_GPIO_ROTARY_B 35
#define IOTW_GPIO_ROTARY_A 32
#define IOTW_GPIO_ROTARY_BUTTON 15
#define IOTW_GPIO_NEO 13
#define IOTW_GPIO_NEOWING 33
#define IOTW_GPIO_A0 A5
#define IOTW_AI_SCALE 1
#define IOTW_GPIO_AUDIO_OUT 25

// Number of maximal possible MQTT Subscriptions
#define IOTW_MAX_MQTT_SUB 10 

// Arraylänge beim Datenfeld (z.B. OLED Array display) 
  #define IOTW_ARRAYLEN 64
// Feldlänge bei EdgeImpulse (maximal mögliche Einträge für features-Vector)
  #define IOTW_EI_MAXPOINTS 50


// 
#define IOTW_GPIO_MIC_BCK      GPIO_NUM_12   // Bit-Clock
#define IOTW_GPIO_MIC_WS       GPIO_NUM_2    // LR-Clock
#define IOTW_GPIO_MIC_SD       GPIO_NUM_27   // Daten-IN




// LoRaWAN radio MCCI LMIC
// #define CFG_sx1262_radio 1
#define CFG_sx1276_radio 1
#define IOTW_GPIO_LMIC_NSS 14
#define IOTW_GPIO_LMIC_DIO0 33
#define IOTW_GPIO_LMIC_DIO1 33
#define IOTW_GPIO_LMIC_RST 255


// Pegel DigiSelfTrans
#define IOTW_BATTERY_LIMIT_25 3.6  // 3xAA NiMH, Spannung bei 25 % Restkapazität
#define IOTW_BATTERY_LIMIT_10 3.3  // 3xAA NiMH, Spannung bei 10 % Restkapazität
#define IOTW_US_WARM_UP_MS    10000    // 10 s Warmup für US-Sensor
#define IOTW_WAIT_6H_IN_MS    21600000 // Wartezeit 6 h für deepsleep Akkubetrieb
#define IOTW_WAIT_1H_IN_MS    3600000  // Wartezeit 1 h für deepsleep Akkubetrieb
#define IOTW_WAIT_30MIN_IN_MS 1800000  // Wartezeit 30 Min für deepsleep Akkubetrieb

#define IOTW_GPIO_US_POWER 4        // GPIO 4 steuert Abschaltung US-Sensor
#define IOTW_GPIO_US_RX 16          // GPIO 16 RX für US-Sensor (Hardware UART)
#define IOTW_GPIO_ONE_WIRE 25       // GPIO 25 als Data für 1-Wire Temperaturmessung Akku DS
#define IOTW_GPIO_ONE_WIRE_EXT 13       // GPIO 25 als Data für 1-Wire Temperaturmessung Akku DS
#define IOTW_GPIO_CHARGE_BATTERY 26 // GPIO 26 Charge Enable Akku

#endif /* Pins_Arduino_h */
