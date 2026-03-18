/*
 *  Copyright (c) 2025  IoT2-Werkstatt <Klaus-Uwe Gollmer>
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Diese Datei ist Teil der Bibliothek »MySensor«.
 *  Weitere Informationen und den vollständigen Lizenztext finden Sie
 *  in der Datei LICENSE im Wurzelverzeichnis dieser Bibliothek.
 */


#ifndef _APDS9999_H_
#define _APDS9999_H_

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

#define APDS9999_ADDRESS (0x52) /**< I2C-Adresse laut I2C-Scanner */

/** I2C Register-Adressen */
enum {
  APDS9999_MAIN_CTRL       = 0x00, /**< Betriebsmodus und Software-Reset */
  APDS9999_PS_VCSEL        = 0x01, /**< Enthält Proximity Gain (Bits[1:0]) und LED Einstellungen (Bits[3:2]) */
  APDS9999_PS_PULSES       = 0x02, /**< Anzahl der VCSEL-Pulse */
  APDS9999_PS_MEAS_RATE    = 0x03, /**< Proximity Messrate */
  APDS9999_LS_MEAS_RATE    = 0x04, /**< Licht-Sensor Messrate & Auflösung */
  APDS9999_LS_GAIN         = 0x05, /**< Analog Gain-Wert für den Lichtsensor */
  APDS9999_PART_ID         = 0x06, /**< Sensor-ID */
  APDS9999_MAIN_STATUS     = 0x07, /**< Interrupt- und Daten-Status */
  APDS9999_PS_DATA_0       = 0x08, /**< LSB des Proximity-Messwerts */
  APDS9999_PS_DATA_1       = 0x09, /**< MSB des Proximity-Messwerts */
  APDS9999_LS_DATA_IR_0    = 0x0A, /**< LSB des IR-Werts */
  APDS9999_LS_DATA_IR_1    = 0x0B, /**< Mittleres Byte des IR-Werts */
  APDS9999_LS_DATA_IR_2    = 0x0C, /**< MSB des IR-Werts */
  APDS9999_LS_DATA_GREEN_0 = 0x0D, /**< LSB des Grün-Werts */
  APDS9999_LS_DATA_GREEN_1 = 0x0E, /**< Mittleres Byte des Grün-Werts */
  APDS9999_LS_DATA_GREEN_2 = 0x0F, /**< MSB des Grün-Werts */
  APDS9999_LS_DATA_BLUE_0  = 0x10, /**< LSB des Blau-Werts */
  APDS9999_LS_DATA_BLUE_1  = 0x11, /**< Mittleres Byte des Blau-Werts */
  APDS9999_LS_DATA_BLUE_2  = 0x12, /**< MSB des Blau-Werts */
  APDS9999_LS_DATA_RED_0   = 0x13, /**< LSB des Rot-Werts */
  APDS9999_LS_DATA_RED_1   = 0x14, /**< Mittleres Byte des Rot-Werts */
  APDS9999_LS_DATA_RED_2   = 0x15, /**< MSB des Rot-Werts */
  APDS9999_INT_CFG         = 0x19, /**< Interrupt-Konfiguration */
  APDS9999_INT_PST         = 0x1A, /**< Interrupt-Persistenz */
  APDS9999_PS_THRES_UP_0   = 0x1B, /**< Proximity Interrupt-Obergrenze LSB */
  APDS9999_PS_THRES_UP_1   = 0x1C, /**< Proximity Interrupt-Obergrenze MSB */
  APDS9999_PS_THRES_LOW_0  = 0x1D, /**< Proximity Interrupt-Untergrenze LSB */
  APDS9999_PS_THRES_LOW_1  = 0x1E, /**< Proximity Interrupt-Untergrenze MSB */
  APDS9999_LS_THRES_UP_0   = 0x21, /**< Licht-Interrupt Obergrenze LSB */
  APDS9999_LS_THRES_UP_1   = 0x22, /**< Licht-Interrupt Obergrenze */
  APDS9999_LS_THRES_UP_2   = 0x23, /**< Licht-Interrupt Obergrenze MSB */
  APDS9999_LS_THRES_LOW_0  = 0x24, /**< Licht-Interrupt Untergrenze LSB */
  APDS9999_LS_THRES_LOW_1  = 0x25, /**< Licht-Interrupt Untergrenze */
  APDS9999_LS_THRES_LOW_2  = 0x26  /**< Licht-Interrupt Untergrenze MSB */
};

/** LS_MEAS_RAT Resolution */
enum {
  APDS9999_LS_RESOLUTION_400MS  = 0x00,
  APDS9999_LS_RESOLUTION_200MS  = 0x01,
  APDS9999_LS_RESOLUTION_100MS  = 0x02,
  APDS9999_LS_RESOLUTION_50MS   = 0x03,
  APDS9999_LS_RESOLUTION_25MS   = 0x04,
  APDS9999_LS_RESOLUTION_3MS    = 0x05
};

enum {
  APDS9999_LS_RATE_25MS   = 0x00,
  APDS9999_LS_RATE_50MS   = 0x01,
  APDS9999_LS_RATE_100MS  = 0x02,
  APDS9999_LS_RATE_200MS  = 0x03,
  APDS9999_LS_RATE_500MS  = 0x04,
  APDS9999_LS_RATE_1000MS = 0x05,
  APDS9999_LS_RATE_2000MS = 0x06
};



/** LED Drive Einstellungen */
typedef enum {
  APDS9999_LEDDRIVE_10MA = 0x02,
  APDS9999_LEDDRIVE_25MA = 0x03
};

/** PS Resolution */
typedef enum {
  APDS9999_PS_RESOLUTION_8BIT = 0x00,
  APDS9999_PS_RESOLUTION_9BIT = 0x01,
  APDS9999_PS_RESOLUTION_10BIT = 0x02,
  APDS9999_PS_RESOLUTION_11BIT = 0x03
}; 


/** ADC Gain Einstellungen */
enum {
  APDS9999_AGAIN_1X  = 0x00,
  APDS9999_AGAIN_3X  = 0x01,
  APDS9999_AGAIN_6X  = 0x02,
  APDS9999_AGAIN_9X  = 0x03,
  APDS9999_AGAIN_18X = 0x04
};


class IoTW_APDS9999 {
public:
  IoTW_APDS9999(){};
  ~IoTW_APDS9999();

  boolean begin(uint8_t addr = APDS9999_ADDRESS, TwoWire *theWire = &Wire);
  void setLSRes(uint8_t);
  uint8_t getLSRes();
  void setLSRate(uint8_t);
  uint8_t getLSRate();
  void setLSGain(uint8_t);
  uint8_t getLSGain();

  // Proximity
  void enableProximity(boolean en = true);
  uint16_t readProximity();
  float calcDistCM(float);
  void setProxRes(uint8_t);
  void setProxPulse(uint8_t);  
  uint8_t getProxRes();

  // LED Steuerung
  void setLED(uint8_t);
  
  // Licht & Farbe
  void enableColor(boolean en = true);
  void getColorData(uint32_t *r, uint32_t *g, uint32_t *b, uint32_t *c);
  bool colorDataReady();
  bool proxDataReady();
  uint32_t calculateLux();
  
private:
  Adafruit_I2CDevice *i2c_dev = NULL;
  void write8(byte reg, byte value);
  uint8_t read8(byte reg);
  uint16_t read16(uint8_t reg);
  uint32_t read20(uint8_t reg);
};

#endif
