/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Modified for ESP8266 platform by Ivan Grokhotkov, 2014-2015.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

/*
Board: Octopus
Platform: ESP8266
CPU: ESP8266EX, 80/160 MHz, 4 MB Flash
Sensors: BME680 (Temp/Humidity/Pressure/Gas, I2C) 
Actuators: NeoPixel (NEO_GRBW + NEO_KHZ800, right = index 0, left = index 1, GPIO13)
Grove: Analog-Grove (A0), I2C-Grove (SCL, SDA)
Feather-Wing: Socket-Headers
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#include <stdint.h>

#include "../generic/common.h"


#define PIN_WIRE_SDA (4)
#define PIN_WIRE_SCL (5)


static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
#define LED_BUILTIN_AUX 16

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;




// IoT-Werkstatt #kgo
#define IOTW_BOARD_OCTOPUS

#define IOTW_GPIO_ROTARY_A 12
#define IOTW_GPIO_ROTARY_B 14
#define IOTW_GPIO_ROTARY_BUTTON 2
#define IOTW_GPIO_NEO 13
#define IOTW_GPIO_NEOWING 15
#define IOTW_GPIO_A0 A0
#define IOTW_AI_SCALE 4

// Number of maximal possible MQTT Subscriptions
#define IOTW_MAX_MQTT_SUB 10 

// Arraylänge beim Datenfeld (z.B. OLED Array display) 
#define IOTW_ARRAYLEN 64
// Feldlänge bei EdgeImpulse (maximal mögliche Einträge für features)
#define IOTW_EI_MAXPOINTS 50


// LoRaWAN radio MCCI LMIC
#define CFG_sx1262_radio 1
#define IOTW_GPIO_LMIC_NSS 2
#define IOTW_GPIO_LMIC_DIO0 15
#define IOTW_GPIO_LMIC_DIO1 15
#define IOTW_GPIO_LMIC_RST -1


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
