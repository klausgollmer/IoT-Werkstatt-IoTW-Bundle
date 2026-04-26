// ========================= IOTW_config.h =========================
#pragma once

// Slim public API for the IoT‑Werkstatt helpers
// Drop this file into your library's include folder and include it from sketches.

#include <Arduino.h>   // keep lightweight, ensures Serial/vTaskDelay are declared
#include <stdint.h>

#define SH110X_DISPLAYOFF   0xAE
#define SH110X_DISPLAYON    0xAF


// --------------------------- Public constants ---------------------------
#ifndef IOTW_SCREEN_WIDTH
#  define IOTW_SCREEN_WIDTH 128
#endif
#ifndef IOTW_SCREEN_HEIGHT
#  define IOTW_SCREEN_HEIGHT 64
#endif

// Board defaults if neither Makey nor Octopus is defined
#if !defined(IOTW_BOARD_OCTOPUS) && !defined(IOTW_BOARD_MAKEY)
  #define IOTW_MAX_MQTT_SUB    10
  #define IOTW_ARRAYLEN        64
  #define IOTW_EI_MAXPOINTS    50
  #define IOTW_GPIO_ROTARY_B   255
  #define IOTW_GPIO_ROTARY_A   255
  #define IOTW_GPIO_ROTARY_BUTTON 255
  #define IOTW_GPIO_NEO        255
  #define IOTW_GPIO_NEOWING    33
  #define IOTW_GPIO_A0         A5
  #define IOTW_AI_SCALE        1
  #define IOTW_GPIO_AUDIO_OUT  25
  #define IOTW_GPIO_MIC_BCK    I2S_PIN_NO_CHANGE
  #define IOTW_GPIO_MIC_WS     I2S_PIN_NO_CHANGE
  #define IOTW_GPIO_MIC_SD     I2S_PIN_NO_CHANGE
#endif

// Default partner logo selector (only used on Makey)
#ifdef ESP32
#  if defined(IOTW_BOARD_MAKEY)
#    ifndef IOTW_LOGO_PARTNER
#      define IOTW_LOGO_PARTNER 0
#    endif
#  endif
#endif

// --------------------------- Debug macros ---------------------------
#ifndef IOTW_DEBUG_LEVEL
#  define IOTW_DEBUG_LEVEL 1
#endif

#if (IOTW_DEBUG_LEVEL == 0)
  #define IOTW_PRINT(...)   do{}while(0)
  #define IOTW_PRINTLN(...) do{}while(0)
  #define IOTW_PRINTF(...)  do{}while(0)
#else
  #define IOTW_PRINT(...)   do{ Serial.print(__VA_ARGS__); }while(0)
  #define IOTW_PRINTLN(...) do{ Serial.println(__VA_ARGS__); }while(0)
  #define IOTW_PRINTF(...)  do{ Serial.printf(__VA_ARGS__); }while(0)
#endif

// --------------------------- Power-friendly delay ---------------------------
#ifdef ESP32
  #ifdef IOTW_GREEN_CODE_LEVEL
    // Use light sleep for long delays; stays a macro-wrapper to avoid redefining core delay()
    #define IOTW_DELAY(ms) do { \
      if ((ms) > 10) { \
        esp_sleep_enable_timer_wakeup((uint64_t)(ms) * 1000ULL); \
        Serial.flush(); \
        esp_light_sleep_start(); \
      } else { \
        vTaskDelay((ms)/portTICK_PERIOD_MS); \
      } \
    } while(0)
  #else
    #define IOTW_DELAY(ms) delay(ms)
  #endif
#else
  #define IOTW_DELAY(ms) delay(ms)
#endif

// --------------------------- Public state & API ---------------------------
extern bool preventDisplayClear;               // single definition lives in IOTW_config.cpp
// Sleep OLED
void IoTW_sleepOLED(int);

// Initialize peripherals / banner / display depending on board
void IoTW_init();

#ifdef ESP32
// Prevent the clear-display FreeRTOS task from wiping the boot logo
void IoTW_preventDisplayClear();
#endif
