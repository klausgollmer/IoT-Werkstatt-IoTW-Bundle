/*
 *  Copyright (c) 2025  IoT2-Werkstatt <Klaus-Uwe Gollmer>
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Diese Datei ist Teil der Bibliothek IoT-Werkstatt
 *  Weitere Informationen und den vollständigen Lizenztext finden Sie
 *  in der Datei LICENSE im Wurzelverzeichnis dieser Bibliothek.
 *
 *  Die Lib nutzt die Arduino-LMIC https://github.com/mcci-catena/arduino-lmic
 * Copyright (C) 2014-2016 IBM Corporation
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2016-2024 MCCI Corporation
*/

#ifndef _IoTW_LMIC_h
#define _IoTW_LMIC_h

#include <Arduino.h>
#include <arduino_lmic_hal_boards.h>
#include <lmic.h>
#include <hal/hal.h>
#include <IoTW_config.h>

extern int IOTW_debug_level; // Print debug information
extern void onEvent (ev_t ev) ;
extern uint32_t RTCcalculateCRC32(const uint8_t *data, size_t length); 
extern int LoRaWAN_Rx_Payload;
extern int LoRaWAN_Rx_Port;
extern String LoRaWAN_Rx_Payload_Raw;
extern volatile int LoRaWAN_Event_TxComplete;
extern volatile int LoRaWAN_Event_No_Join;  

extern void os_runloop_once_sleep();

typedef void (*Callback)();
// Funktionspointer initialisieren
extern Callback LoRaWANCallbackPointer;



#ifdef ESP8266
  extern void LoadLMICFromRTC_ESP8266();
  extern void SaveLMICToRTC_ESP8266(int); 
#endif
#ifdef ESP32
  #include <rom/rtc.h>
  #include <esp_attr.h> // Für RTC_DATA_ATTR
  extern RTC_DATA_ATTR lmic_t RTC_LMIC;
  extern void LoadLMICFromRTC_ESP32() ;
  extern void SaveLMICToRTC_ESP32(int) ;
#endif

#endif // _IoTW_LMIC_h




