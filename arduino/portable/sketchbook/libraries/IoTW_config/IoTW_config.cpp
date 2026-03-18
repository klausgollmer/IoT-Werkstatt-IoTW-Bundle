// ========================= IOTW_config.cpp =========================
#include "IoTW_config.h"

#if defined(ESP8266) || defined(ESP32) || defined(AVR)
  #include <Wire.h>
#endif

#ifdef ESP32
  #include <rom/rtc.h>        // esp_reset_reason
#endif

#if defined(IOTW_BOARD_MAKEY)
  #include <Adafruit_GFX.h>
  #include <Adafruit_SH110X.h>
  #include <Adafruit_NeoPixel.h>
  // ---- Logo selection: provide one of these headers with a PROGMEM bitmap 
  // Each header should: 1) #define IOTW_HAVE_LOGO_BITMAP 1, 2) define
  //   extern const unsigned char logoBitmap[] PROGMEM; (size 128x64)
  // If you don't supply any header, the logo drawing is simply skipped.
  #if   (IOTW_LOGO_PARTNER == 1)
    #include "logos/logo_vdi.h"
  #elif (IOTW_LOGO_PARTNER == 2)
    #include "logos/logo_zeit.h"
  #elif (IOTW_LOGO_PARTNER == 3)
    #include "logos/logo_gi.h"
  #elif (IOTW_LOGO_PARTNER == 4)
    #include "logos/logo_heise.h"
  #elif (IOTW_LOGO_PARTNER == 5)
    #include "logos/logo_h96.h"
  #else
    #include "logos/logo_iotw.h"  // create this from your existing default logo
  #endif
#endif

// --------------------------- Internal helpers ---------------------------
static inline bool i2cDevicePresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

#if defined(IOTW_BOARD_MAKEY)
  static void Makey_on_Terminal() {
    IOTW_PRINTLN("\n");
    IOTW_PRINTLN(F("              MMMMMMMMMM"));
    IOTW_PRINTLN(F("            M  MMMMMMMM  M"));
    IOTW_PRINTLN(F("           M    MMMMMM    M"));
    IOTW_PRINTLN(F("          MMMMMMMMMMMMMMMMMM"));
    IOTW_PRINTLN(F(" "));
    IOTW_PRINTLN(F("         MMMMMMMMMMMMMMMMMMMM"));
    IOTW_PRINTLN(F("   MMMMM MMMM  MM    MM  MMMM MMMMMM"));
    IOTW_PRINTLN(F("  MMMMMM MMMM  MMM  MMM  MMMM MMMMMM"));
    IOTW_PRINTLN(F(" MMMMMMM MMM   M M  M M   MMM MMMMMMM"));
    IOTW_PRINTLN(F(" MMMMMMM MMMM  M  MM  M  MMMM MMMMMMM"));
    IOTW_PRINTLN(F(" MMMMMM  MMMMMMMM    MMMMMMMM  MMMMMM"));
    IOTW_PRINTLN(F(" MMMMMM   MMMMMMMMMMMMMMMMMM   MMMMMM"));
    IOTW_PRINTLN(F(" MMMMMM    MMMMM      MMMMM    MMMMMM"));
    IOTW_PRINTLN(F("  MMMMMM   MMMMM      MMMMM   MMMMMM"));
    IOTW_PRINTLN(F("  MMMMMM   MMMMMM    MMMMMM   MMMMMM"));
    IOTW_PRINTLN(F("    MMMM  MMMMMMM    MMMMMMM  MMMM"));
    IOTW_PRINTLN(F("          MMMMMMM    MMMMMMM"));
    IOTW_PRINTLN(F("          MMMMMMM    MMMMMMM"));
    IOTW_PRINTLN(F("         MMMMMMMM    MMMMMMMM"));
    IOTW_PRINT(F(" ------- Frequency: "));
    #ifdef ESP32
      IOTW_PRINT(getCpuFrequencyMhz());
    #else
      IOTW_PRINT("n/a");
    #endif
    IOTW_PRINTLN(F(" MHz ----------"));
  }

  static void Task_ClearDisplay(void *pvParameters) {
    Adafruit_SH1107* disp = (Adafruit_SH1107*)pvParameters;
    vTaskDelay(pdMS_TO_TICKS(10000));
    if (!preventDisplayClear) {
      disp->clearDisplay();
      disp->display();
    }
    vTaskDelete(NULL);
  }
#endif

#if defined(IOTW_BOARD_OCTOPUS)
  static void Octi_on_Terminal(){
    IOTW_PRINTLN(F("\n\n      @@@@@@@@"));
    IOTW_PRINTLN(F("   @@@@@@@@@@@@@@@"));
    IOTW_PRINTLN(F("  @@@@@@@@@@@@@@@@@"));
    IOTW_PRINTLN(F("  @@@@  @@@@@  @@@@@"));
    IOTW_PRINTLN(F(" @@@@@@@@@@@@@@@@@@"));
    IOTW_PRINTLN(F("  @@@@@@@@@@@@@@@@@"));
    IOTW_PRINTLN(F("    @@@@@  @@@@@@"));
    IOTW_PRINTLN(F("     @@@@@@@@@@@"));
    IOTW_PRINTLN(F("    @@@@@@@@@@@@@@@@@@@@@@"));
    IOTW_PRINTLN(F("  @@@@@@@@@@@@@@@@@@     @@"));
    IOTW_PRINTLN(F(" @@@@@@@@@@@@@@@@ @@@@@"));
    IOTW_PRINTLN(F(" @@@ @@@@@@@@@@@@@@  @@@@@@@"));
    IOTW_PRINTLN(F(" @@@  @@ @@@@ @@ @@    @@@@"));
    IOTW_PRINTLN(F("  @@@  @@ @@@  @@@ @@@"));
    IOTW_PRINTLN(F("    @@@  @@@@@@  @@@ @@@@@"));
    IOTW_PRINTLN(F("     @@@  @ @@@  @@@   @@@@"));
    IOTW_PRINTLN(F("      @@  @ @@@   @@@    @@@"));
    IOTW_PRINTLN(F("      @@  @ @@@   @@@"));
    IOTW_PRINTLN(F("      @@    @@    @@@"));
    IOTW_PRINTLN(F("      @    @@   @@"));
  }
#endif

// --------------------------- Public state ---------------------------
bool preventDisplayClear = false;   // single definition

// --------------------------- Public API ---------------------------
void IoT_WerkstattInit() {
  // ESP8266 I2C bring-up (matches your original logic)
  #if defined(ESP8266)
    Wire.begin(SDA, SCL);
    if (Wire.status() != I2C_OK) {
      IOTW_PRINTLN(F("Something wrong with I2C"));
    }
  #endif

  #if defined(IOTW_BOARD_OCTOPUS)
    Octi_on_Terminal();
  #endif

  #if defined(ESP32) && defined(IOTW_BOARD_MAKEY)
    int reason = esp_reset_reason();
    if (reason == ESP_RST_POWERON) {
      Wire.begin(SDA, SCL);

      // If no OLED present, keep running without display
      if (!i2cDevicePresent(0x3C)) {
        IOTW_PRINTLN(F("OLED-Display nicht gefunden, I2C überspringen."));
        return;
      }

      // Display
      Adafruit_SH1107* display = new Adafruit_SH1107(IOTW_SCREEN_HEIGHT, IOTW_SCREEN_WIDTH, &Wire);
      if (display->begin(0x3C, true)) {
        display->setRotation(1);
        display->clearDisplay();
        #ifdef IOTW_HAVE_LOGO_BITMAP
          display->drawBitmap(0, 0, logoBitmap, IOTW_SCREEN_WIDTH, IOTW_SCREEN_HEIGHT, SH110X_BLACK, SH110X_WHITE);
        #endif
        display->display();

        // Terminal banner
        Makey_on_Terminal();

        // NeoPixel init (2 pixels on IOTW_GPIO_NEO)
        Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, IOTW_GPIO_NEO, NEO_GRBW + NEO_KHZ800);
        pixels.begin();
        IOTW_DELAY(10);
        pixels.clear();
        pixels.show();
        IOTW_DELAY(1000);

        // Schedule clear task (unless user prevents it)
        xTaskCreate(Task_ClearDisplay, "ClearDisplayTask", 2048, display, 1, NULL);
      }
    }
  #endif
}

#ifdef ESP32
void IoT_WerkstattPreventDisplayClear() {
  preventDisplayClear = true;
}
#endif
