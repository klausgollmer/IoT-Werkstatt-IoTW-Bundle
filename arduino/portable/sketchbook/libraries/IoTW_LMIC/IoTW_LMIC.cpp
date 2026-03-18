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


#include <arduino_lmic_hal_boards.h>
#include <lmic.h>
#include <hal/hal.h>
#include <IoTW_LMIC.h>
#include <Arduino.h>

#ifdef ESP32
#include <esp_attr.h> // Für RTC_DATA_ATTR
RTC_DATA_ATTR  lmic_t RTC_LMIC;
#endif
const char* dr2str[] = {
    "SF12","SF11","SF10","SF9","SF8","SF7","SF7B","FSK"  };
volatile int LoRaWAN_Event_TxComplete = 0 ;
volatile int LoRaWAN_Event_No_Join    = 1;  


// ==== Snapshots für das aktuelle Funk-Zyklus ====
static ostime_t s_rx1_time = 0;
static ostime_t s_rx2_time = 0;
static bool     s_in_cycle = false;  // wir sind in TX/RX-Sequenz



typedef void (*Callback)();
// Funktionspointer initialisieren
Callback LoRaWANCallbackPointer = nullptr;

int LoRaWAN_Rx_Payload = 0 ;
int LoRaWAN_Rx_Port = 0 ;
String LoRaWAN_Rx_Payload_Raw = "" ;


void onEvent (ev_t ev) {
  if (IOTW_debug_level>1) IOTW_PRINT(os_getTime());
  if (IOTW_debug_level>1) IOTW_PRINT(": ");
  switch(ev) {
  case EV_SCAN_TIMEOUT:
    if (IOTW_debug_level) IOTW_PRINTLN(F("❌ EV_SCAN_TIMEOUT"));
    break;
  case EV_BEACON_FOUND:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_BEACON_FOUND"));
    break;
  case EV_BEACON_MISSED:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_BEACON_MISSED"));
    break;
  case EV_BEACON_TRACKED:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_BEACON_TRACKED"));
    break;
  case EV_JOINING:
    if (IOTW_debug_level) {
		IOTW_PRINT(F("EV_JOINING "));
		IOTW_PRINTLN(dr2str[LMICbandplan_getInitialDrJoin()]);
	}
    break;
  case EV_JOINED:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_JOINED"));
    // LoRaWAN_Tx_Ready =  !(LMIC.opmode & OP_TXDATA); // otherwise joined without TX blocks queue
    break;
    /*
        || This event is defined but not used in the code. No
     || point in wasting codespace on it.
     ||
     || case EV_RFU1:
     ||     if (IOTW_debug_level) IOTW_PRINTLN(F("EV_RFU1"));
     ||     break;
     */
  case EV_JOIN_FAILED:
    LoRaWAN_Event_No_Join = 1;  
    if (IOTW_debug_level) IOTW_PRINTLN(F("❌ EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    LoRaWAN_Event_No_Join = 1;  
    if (IOTW_debug_level) IOTW_PRINTLN(F("❌ EV_REJOIN_FAILED"));
    break;
  case EV_TXCOMPLETE:
    s_in_cycle = false;
    s_rx1_time = s_rx2_time = 0;
  
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_TXCOMPLETE (incl. RX)"));
    if (LMIC.txrxFlags & TXRX_ACK)
      if (IOTW_debug_level) IOTW_PRINTLN(F("Received ack"));
    if (LMIC.dataLen) {
      if (IOTW_debug_level) IOTW_PRINTLN(F("Received "));
      if (IOTW_debug_level) IOTW_PRINTLN(LMIC.dataLen);
      if (IOTW_debug_level) IOTW_PRINTLN(F(" bytes of payload"));
      LoRaWAN_Rx_Payload = 0; // Payload IoT-Werkstatt
      LoRaWAN_Rx_Payload_Raw = ""; // Payload IoT-Werkstatt
      LoRaWAN_Rx_Port    = LMIC.frame[LMIC.dataBeg-1];              
      String Zeichen = "";              
      for (int i = 0;i<LMIC.dataLen;i++) { 
        if (IOTW_debug_level) IOTW_PRINTLN(LMIC.frame[i+ LMIC.dataBeg],HEX);
        LoRaWAN_Rx_Payload = 256*LoRaWAN_Rx_Payload+LMIC.frame[i+ LMIC.dataBeg];
        Zeichen = String(LMIC.frame[i+ LMIC.dataBeg],HEX);
        if (Zeichen.length() == 1) Zeichen = "0"+Zeichen;
        LoRaWAN_Rx_Payload_Raw += Zeichen;
      }

      //#kgo
      //if (IOTW_debug_level) IOTW_PRINTLN("Downlink");
      if (LoRaWANCallbackPointer) {
        LoRaWANCallbackPointer();
      }
    }
    LoRaWAN_Event_TxComplete = 1;
    // Schedule next transmission
    // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    break;
  case EV_LOST_TSYNC:
    if (IOTW_debug_level) IOTW_PRINTLN(F("❌ EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_RESET"));
    s_in_cycle = false;
    s_rx1_time = s_rx2_time = 0;
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_LINK_ALIVE"));
    break;
    /*
        || This event is defined but not used in the code. No
     || point in wasting codespace on it.
     ||
     || case EV_SCAN_FOUND:
     ||    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_SCAN_FOUND"));
     ||    break;
     */
  case EV_TXSTART:
   s_in_cycle  = true;
   s_rx1_time  = 0;
   s_rx2_time  = 0;
  
//   s_rx1_time = LMIC.rxtime;             // RX1 Startzeit
//   s_rx2_time = s_rx1_time + sec2osticks(1); // RX2 = RX1 + 1s
  
  
    if (IOTW_debug_level){
		IOTW_PRINT(F("EV_TXSTART "));
		IOTW_PRINTLN(dr2str[LMIC.datarate]);
		
	}	
    break;
  case EV_TXCANCELED:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_TXCANCELED"));
	s_in_cycle = false;
    s_rx1_time = s_rx2_time = 0;
    break;
  case EV_RXSTART:
    /* do not print anything -- it wrecks timing */
    break;
  case EV_JOIN_TXCOMPLETE:
    if (IOTW_debug_level) IOTW_PRINTLN(F("EV_JOIN_TXCOMPLETE: ❌ no JoinAccept"));
    LoRaWAN_Event_No_Join = 1;  
	s_in_cycle = false;
    s_rx1_time = s_rx2_time = 0;
	
	break; 
  default:
    if (IOTW_debug_level) IOTW_PRINT(F("Unknown event: "));
    if (IOTW_debug_level) IOTW_PRINTLN((unsigned) ev);
    break;
  }
}





// ==== Einstellungen für Sleep ====
#define HEADROOM_MS         20UL     // früher aufwachen, Fenster sicher treffen
#define MAX_LIGHTSLEEP_MS   10000UL  // sonst nie > 10 s am Stück schlafen
#define MIN_SLEEP_MS        30UL     // Micro-Naps vermeiden

// Hilfsfunktion: Millisekunden bis zu einem Zeitpunkt in osticks
static inline uint32_t ms_until(ostime_t t) {
    if (t == 0) return 0;
    ostime_t now = os_getTime();
    if (t <= now) return 0;
    return (uint32_t)osticks2ms(t - now);
}
// Helfer: harte vs. weiche Jobs
static inline bool lmic_has_hard_job() {
  return (LMIC.opmode & (OP_TXRXPEND | OP_TXDATA | OP_JOINING)) != 0;
}





// Millisekunden bis zum nächsten Funkereignis (TX-Start, RX1, RX2)
// Rückgabe: 0 = nichts geplant
static uint32_t ms_until_next_radio_event() {
    ostime_t now = os_getTime();
    uint32_t best = UINT32_MAX;

    // --- A) Vor dem Uplink: warten bis TX starten darf ---
    bool txQueued = (LMIC.opmode & (OP_TXDATA | OP_JOINING)) || (LMIC.pendTxLen > 0);
    if (!s_in_cycle && txQueued) {
        ostime_t a = (LMIC.txend           > now) ? (LMIC.txend           - now) : 0;
        ostime_t b = (LMIC.globalDutyAvail > now) ? (LMIC.globalDutyAvail - now) : 0;
        ostime_t need = (a > b) ? a : b;
        if (need > 0) {
            uint32_t ms_need = (uint32_t)osticks2ms(need);
            if (ms_need < best) best = ms_need;
        }
    }

    // --- B) Während TX/RX-Zyklus: RX1 / RX2 Fenster ---
    if (s_in_cycle) {

        // Lazy Snapshot der Fensterzeiten
//        if (s_rx1_time == 0 && (LMIC.txend != 0)) {
        if (LMIC.txend != 0) { // tx vorüber, RX genau bekannt
            s_rx1_time = LMIC.txend+sec2osticks(5);
            s_rx2_time = s_rx1_time + sec2osticks(1);
			if (IOTW_debug_level>1) {
		       Serial.print("RX1 calculation based on txend");Serial.println(s_rx1_time);
			}
        } else if (LMIC.rxtime > now) { // RX geschätzt
		    if (s_rx1_time == 0) {
               s_rx1_time = LMIC.rxtime;
	           s_rx2_time = s_rx1_time + sec2osticks(1);
			   if (IOTW_debug_level>1) {
			      Serial.print("RX1 estimation on calculated rxtime");Serial.println(s_rx1_time);
			   }
			}
        }
	
        if (s_rx1_time && now < s_rx1_time) {
            uint32_t ms_rx1 = (uint32_t)osticks2ms(s_rx1_time - now);
			if (ms_rx1 >= HEADROOM_MS) ms_rx1 -= HEADROOM_MS;
			if (ms_rx1 < best) best = ms_rx1;
        }
        else if (s_rx2_time && now < s_rx2_time) {
            uint32_t ms_rx2 = (uint32_t)osticks2ms(s_rx2_time - now);
			if (ms_rx2 >= HEADROOM_MS) ms_rx2 -= HEADROOM_MS;
            if (ms_rx2 < best) best =ms_rx2;
        }
		
        else {
            // Beide Fenster vorbei
            return 0;
        }
    }
    if (best > MAX_LIGHTSLEEP_MS) best = MAX_LIGHTSLEEP_MS; // max Wake 
    return (best == UINT32_MAX) ? 0 : best;
}

void os_runloop_once_sleep() {
    os_runloop_once();

#ifdef ESP32
    uint32_t sleep_ms = ms_until_next_radio_event();
    if ((sleep_ms > MIN_SLEEP_MS) && lmic_has_hard_job()) {

        // Timer-Wakeup immer setzen
        esp_sleep_enable_timer_wakeup((uint64_t)sleep_ms * 1000ULL);

        // Phase anhand der gesetzten RX-Zeiten bestimmen
        bool waiting_rx1 = s_in_cycle && (s_rx1_time != 0) && (os_getTime() < s_rx1_time);
        bool waiting_rx2 = s_in_cycle && (s_rx2_time != 0) && (os_getTime() < s_rx2_time);

        if (IOTW_debug_level) {
            Serial.print("lightsleep bis ");
            if (waiting_rx1) Serial.print("RX1 in ");
            else if (waiting_rx2) Serial.print("RX2 in ");
            else Serial.print("Job in ");
            Serial.print(sleep_ms);
            Serial.print(" ms, ...");
		    Serial.flush();
         }

        // Nur während RX1 auch GPIO-Wakeup erlauben
        if (waiting_rx1) {
            uint64_t mask = (1ULL << IOTW_GPIO_LMIC_DIO0);
            esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_HIGH);
        }

        // Sleep starten
        esp_light_sleep_start();

        // Debug: Wakeup-Ursache ausgeben
        if (IOTW_debug_level) {
            esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
            Serial.print(" wakeup cause: ");
            switch (cause) {
                case ESP_SLEEP_WAKEUP_EXT0: Serial.println("EXT0 GPIO"); break;
                case ESP_SLEEP_WAKEUP_EXT1: Serial.println("EXT1 GPIO (DIO0/DIO1)"); break;
                case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Timer"); break;
                default: Serial.println((int)cause); break;
            }
        }
    }
#endif

    os_runloop_once();
}


// ESP8266:  Load/Store LoRa LMIC to RTC-Mem 
#if defined(ESP8266)  
//----------------------------------------------- load store LMIC structure in TRC-RAM
// Berechne CRC-Prüfsumme für RTC-RAM 
uint32_t RTCcalculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

void LoadLMICFromRTC_ESP8266() {
  lmic_t RTC_LMIC;
  uint32_t crcOfData;
  if (sizeof(lmic_t) <= 512) {
    ESP.rtcUserMemoryRead(1, (uint32_t*) &RTC_LMIC, sizeof(RTC_LMIC));
    ESP.rtcUserMemoryRead(0, (uint32_t*) &crcOfData, sizeof(crcOfData));
    uint32_t crcOfData_RTC = RTCcalculateCRC32((uint8_t*) &RTC_LMIC, sizeof(RTC_LMIC));
    if (crcOfData != crcOfData_RTC) {
      if (IOTW_debug_level) IOTW_PRINTLN("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!");
    } 
    else {
      if (IOTW_debug_level) IOTW_PRINT(F("load LMIC from RTC, FrameCounter =  "));
      LMIC = RTC_LMIC;
	  LMIC.opmode &= ~(OP_TXRXPEND | OP_TXDATA | OP_POLL | OP_JOINING | OP_NEXTCHNL | OP_RNDTX);
      os_clearCallback(&LMIC.osjob);
      // *** WICHTIG: Zeitfelder auf die neue Taktbasis setzen ***
      LMIC.rxtime = 0;
      LMIC.txend  = os_getTime();   // <<< verhindert „Geister-Wartezeit“
      if (IOTW_debug_level) IOTW_PRINTLN(LMIC.seqnoUp);     
    }  
  } 
  else {
    if (IOTW_debug_level) IOTW_PRINTLN(F("sizelimit RTC-Mem, #define LMIC_ENABLE_long_messages in config.h"));
  };
}; 

void SaveLMICToRTC_ESP8266(int deepsleep_sec) {
	uint32_t t_start = millis();
	while (os_queryTimeCriticalJobs(ms2osticksRound(deepsleep_sec * 1000))) {
    if (!(LMIC.opmode & (OP_TXRXPEND | OP_JOINING))) {
        if (IOTW_debug_level) {
			IOTW_PRINTLN(F("Soft job only (e.g. POLL), skip waiting"));
		    Serial.println(LMIC.txend); 
		}
        break;
    }
    os_runloop_once_sleep();

    if (millis() - t_start > 120000) { // 120 s hartes Limit
        if (IOTW_debug_level) IOTW_PRINTLN(F("Hard timeout waiting for TX/Join -> abort"));
        LMIC_reset();  // sichergehen, Neuanfang
        break;
    }
}  

	
  if (sizeof(lmic_t) <= 512) {
    if (IOTW_debug_level) IOTW_PRINTLN(F("Save LMIC to RTC"));
	
	  // 1) Duty-Cycle nur um Schlafdauer vorspulen
	#if defined(CFG_LMIC_EU_like)
	  const ostime_t sleep_ticks = sec2osticks(deepsleep_sec);
	  for (int i = 0; i < MAX_BANDS; i++) {
		LMIC.bands[i].avail = (LMIC.bands[i].avail > sleep_ticks) ? (LMIC.bands[i].avail - sleep_ticks) : 0;
	  }
	  LMIC.globalDutyAvail = (LMIC.globalDutyAvail > sleep_ticks) ? (LMIC.globalDutyAvail - sleep_ticks) : 0;
	#endif

	  // 2) Flüchtige Opmode-Bits löschen (nichts „offen“ konservieren)
	  LMIC.opmode &= ~(OP_TXRXPEND | OP_TXDATA | OP_POLL | OP_JOINING | OP_NEXTCHNL | OP_RNDTX);
	// keine neuen Starts
	  os_clearCallback(&LMIC.osjob);
		// weiche Flags raus
	  LMIC.txrxFlags = 0;
	  LMIC.pendTxLen  = 0;
	  LMIC.pendTxConf = 0;
	  LMIC.pendTxPort = 0;
	  LMIC.rxtime = 0;                 // keine erwarteten RX-Zeiten mitschleppen
	  LMIC.txend  = os_getTime();      // „ab jetzt“ (oder =0); wichtig: keine alte Tick-Basis

		// Write to RTC
	  uint32_t crcOfData = RTCcalculateCRC32((uint8_t*) &LMIC, sizeof(LMIC));
	  ESP.rtcUserMemoryWrite(1, (uint32_t*) &LMIC, sizeof(LMIC));
	  ESP.rtcUserMemoryWrite(0, (uint32_t*) &crcOfData, sizeof(crcOfData));
  } else {
    if (IOTW_debug_level) IOTW_PRINTLN(F("sizelimit RTC-Mem, #define LMIC_ENABLE_long_messages in config.h"));
  }
};
#endif 



// ESP32 Load/Store LoRa LMIC to RTC-Mem 
// more Info https://github.com/JackGruber/ESP32-LMIC-DeepSleep-example/blob/master/src/main.cpp

#if defined(ESP32)  
void LoadLMICFromRTC_ESP32() {
  if ((RTC_LMIC.seqnoUp != 0)) {
    if (IOTW_debug_level) IOTW_PRINT(F("load LMIC from RTC, Frame = "));
    if (IOTW_debug_level) IOTW_PRINTLN(RTC_LMIC.seqnoUp);
    LMIC = RTC_LMIC;
	
	// Sicherheit: flüchtige Bits & osjob bereinigen
	LMIC.opmode &= ~(OP_TXRXPEND | OP_TXDATA | OP_POLL | OP_JOINING | OP_NEXTCHNL | OP_RNDTX);
    os_clearCallback(&LMIC.osjob);
    // *** WICHTIG: Zeitfelder auf die neue Taktbasis setzen ***
    LMIC.rxtime = 0;
    LMIC.txend  = os_getTime();   // <<< verhindert „Geister-Wartezeit“
	
  } 
  else {
    if (IOTW_debug_level) IOTW_PRINTLN(F("no valid LMIC Data, start from scratch ----------------- "));
  }; 
}; 

void SaveLMICToRTC_ESP32(int deepsleep_sec) {
	
uint32_t t_start = millis();
while (os_queryTimeCriticalJobs(ms2osticksRound(deepsleep_sec * 1000))) {
    if (!(LMIC.opmode & (OP_TXRXPEND | OP_JOINING))) {
        if (IOTW_debug_level) {
			IOTW_PRINTLN(F("Soft job only (e.g. POLL), skip waiting"));
		    Serial.println(LMIC.txend); 
		}
        break;
    }
    os_runloop_once_sleep();

    if (millis() - t_start > 120000) { // 120 s hartes Limit
        if (IOTW_debug_level) IOTW_PRINTLN(F("Hard timeout waiting for TX/Join -> abort"));
        LMIC_reset();  // sichergehen, Neuanfang
        break;
    }
}  
  
  
 if (IOTW_debug_level) IOTW_PRINTLN(F("Save LMIC to RTC"));

  // 1) Duty-Cycle nur um Schlafdauer vorspulen
#if defined(CFG_LMIC_EU_like)
  const ostime_t sleep_ticks = sec2osticks(deepsleep_sec);
  for (int i = 0; i < MAX_BANDS; i++) {
    LMIC.bands[i].avail = (LMIC.bands[i].avail > sleep_ticks) ? (LMIC.bands[i].avail - sleep_ticks) : 0;
  }
  LMIC.globalDutyAvail = (LMIC.globalDutyAvail > sleep_ticks) ? (LMIC.globalDutyAvail - sleep_ticks) : 0;
#endif

  // 2) Flüchtige Opmode-Bits löschen (nichts „offen“ konservieren)
  LMIC.opmode &= ~(OP_TXRXPEND | OP_TXDATA | OP_POLL | OP_JOINING | OP_NEXTCHNL | OP_RNDTX);
// keine neuen Starts
  os_clearCallback(&LMIC.osjob);
	// weiche Flags raus
  LMIC.txrxFlags = 0;
  LMIC.pendTxLen  = 0;
  LMIC.pendTxConf = 0;
  LMIC.pendTxPort = 0;
  LMIC.rxtime = 0;                 // keine erwarteten RX-Zeiten mitschleppen
  LMIC.txend  = os_getTime();      // „ab jetzt“ (oder =0); wichtig: keine alte Tick-Basis

  // 4) Persistieren
  RTC_LMIC = LMIC;
}; 
#endif 







/* 
#if defined(ESP32)  
void LoadLMICFromRTC_ESP32() {
  if ((RTC_LMIC.seqnoUp != 0)) {
    if (IOTW_debug_level) IOTW_PRINT(F("load LMIC from RTC, Frame = "));
    if (IOTW_debug_level) IOTW_PRINTLN(RTC_LMIC.seqnoUp);
    LMIC = RTC_LMIC;
  } 
  else {
    if (IOTW_debug_level) IOTW_PRINTLN(F("no valid LMIC Data, start from scratch ----------------- "));
  }; 
}; 

void SaveLMICToRTC_ESP32(int deepsleep_sec) {
	
  // sicherstellen, dass es keine weiteren Jobs, außer Joining gibt   	
  if(os_queryTimeCriticalJobs(ms2osticksRound( (deepsleep_sec*1000) ))&&!LoRaWAN_Event_No_Join)
    {
        if (IOTW_debug_level) IOTW_PRINTLN(F("-------------------------------------  Can't  sleep"));
		while (os_queryTimeCriticalJobs(ms2osticksRound( (deepsleep_sec*1000) ))) {
			os_runloop_once_sleep();
		}
    }
  if (IOTW_debug_level) IOTW_PRINTLN(F("Save LMIC to RTC"));

  unsigned long now = millis();
  // EU Like Bands
#if defined(CFG_LMIC_EU_like)
  // if (IOTW_debug_level) IOTW_PRINTLN(F("Reset CFG_LMIC_EU_like band avail"));
  for (int i = 0; i < MAX_BANDS; i++)
  {
   ostime_t correctedAvail = LMIC.bands[i].avail - ((now / 1000.0 + deepsleep_sec) * OSTICKS_PER_SEC);
    
   if (correctedAvail < 0)
    {
      correctedAvail = 0;
    }
    LMIC.bands[i].avail = correctedAvail;
    //if (IOTW_debug_level) IOTW_PRINT(LMIC.bands[i].avail);
  }

   LMIC.globalDutyAvail = LMIC.globalDutyAvail - ((now / 1000.0 + deepsleep_sec) * OSTICKS_PER_SEC);
   if (LMIC.globalDutyAvail < 0)
  {
    LMIC.globalDutyAvail = 0;
  }
  //if (IOTW_debug_level) IOTW_PRINT(LMIC.globalDutyAvail);

#else
  //if (IOTW_debug_level) IOTW_PRINTLN(F("No DutyCycle recalculation function!"));
#endif
  // Write to RTC

  RTC_LMIC = LMIC;
}; 
#endif 

*/

