/*
*  Copyright (c) 2026  IoT2-Werkstatt <Klaus-Uwe Gollmer>
*  SPDX-License-Identifier: MIT
*  Diese Datei ist Teil der Bibliothek IoT-Werkstatt
*  nutzt FTServo_Arduino (SCServo.h, Klasse SMS_STS), tested with STS3215
*  https://github.com/ftservo/FTServo_Arduino
*  MIT license
*  max. 20 servo (ID 0 to 20) im Scan
*
*  Unterschiede zu IoTW_Feetech_SCL.cpp (SC09 / SCSCL):
*    - Klasse SMS_STS, 4096 Schritte = 360 Grad, Little-Endian (macht die Lib)
*    - Betriebsart (Position / Wheel) per Register 33 (EEPROM!), nicht per Winkelgrenzen
*    - Torque-Limit liegt im RAM-Register 48 (0..1000 = 0..100 %)
*    - Last (Reg 60/61): 0..1000 = 0..100 % PWM-Ansteuerung, Vorzeichenbit macht die Lib
*    - WritePosEx() schreibt bei jeder Fahrt auch die Geschwindigkeit -> "Max Speed"
*      wird hier pro ID gemerkt (set mode 3) und bei jeder Positionsfahrt mitgesendet
*
*  set(id, mode, wert):
*    0 = Zielposition in Grad (0..360)      4 = Zielposition in Rohwerten (0..4095)
*    1 = Torque an/aus (0/1)                5 = Wheel-Mode an/aus (1/0), schreibt EEPROM
*    2 = Torque-Limit in % (0..100)         6 = Wheel-Geschwindigkeit in % (-100..100)
*    3 = Max. Speed fuer Positionsfahrten in % (0..100, 0 = Servo-Maximum)
*    7 = neue ID (0..253), schreibt EEPROM
*  get(id, mode):
*    0 = Winkel in Grad     1 = Torque an/aus   2 = Last in % (mit Vorzeichen)
*    3 = faehrt gerade      4 = Temperatur C    5 = Position in Rohwerten
*    6 = Ping (1/0)         7 = Strom (Rohwert) 8 = Spannung in V
*/

#ifdef ESP32

#include <SCServo.h>      // enthaelt SMS_STS inkl. Registerdefinitionen SMS_STS_*
#include <IoTW_config.h>
#include <math.h>
#include <Arduino.h>

extern int IOTW_debug_level;

#define MAX_FT_ID_SCAN   20
#define STS_RANGE_DEG    360.0f
#define STS_STEPS_REV    4096.0f   // 4096 Schritte pro Umdrehung (Rohwerte 0..4095)
#define STS_MAX_RAW      4095
#define STS_MAX_SPEED    3000      // Schritte/s fuer 100 % (Naeherung, bei Bedarf anpassen)
#define STS_ACC          0         // Beschleunigung 0 = Servo-Standard

static SMS_STS  st;
static uint16_t sts_speed[256];    // Max Speed pro ID in Schritten/s, 0 = Servo-Maximum


// --------------------------------------------------
// interne Helfer
// --------------------------------------------------

// Aktuelle Position als Ziel setzen (verhindert Sprung beim Torque-Einschalten).
// Nur im Positionsmodus (Reg 33 = 0) sinnvoll.
static void sts_hold_position(uint8_t id) {
  if (st.readByte(id, SMS_STS_MODE) != 0) return;   // -1 (Fehler) oder Wheel-Mode -> nichts tun
  int p = st.ReadPos(id);
  if (p >= 0) st.WritePosEx(id, (s16)p, sts_speed[id], STS_ACC);
}

// Torque ein und Zielposition (Rohwert) senden
static void sts_goto(uint8_t id, int pos) {
  pos = constrain(pos, 0, STS_MAX_RAW);
  st.EnableTorque(id, 1);
  st.WritePosEx(id, (s16)pos, sts_speed[id], STS_ACC);
}


// --------------------------------------------------
// Begin
// --------------------------------------------------
void feetech_sts_begin(int ser, int rx, int tx, int baud) {
  switch (ser) {
    case 0:
      Serial.begin(baud, SERIAL_8N1, rx, tx);
      st.pSerial = &Serial;
    break;

    case 1:
      Serial1.begin(baud, SERIAL_8N1, rx, tx);
      st.pSerial = &Serial1;
    break;

    case 2:
      Serial2.begin(baud, SERIAL_8N1, rx, tx);
      st.pSerial = &Serial2;
    break;

    case 3:
      st.pSerial = nullptr;
      IOTW_PRINT(F("sorry, no softwareserial on feetech "));
      return;
    break;

    default:
      st.pSerial = nullptr;
      IOTW_PRINT(F("unknown serial in feetech "));
      return;
  }
  IOTW_PRINT(F("feetech STS ready on ser=")); IOTW_PRINT(ser); IOTW_PRINT(F(" baud=")); IOTW_PRINTLN(baud);
}


// --------------------------------------------------
// Scan
// --------------------------------------------------
int feetech_sts_scan() {
  int count = 0;
  if (st.pSerial == nullptr) return 0;
  IOTW_PRINT(F("scan id 0-20, torque off: "));

  for (uint8_t id = 0; id <= MAX_FT_ID_SCAN; id++) {
    if (st.Ping(id) >= 0) {
      IOTW_PRINT(id); IOTW_PRINT(F(" ")); count++;
      st.EnableTorque(id, 0);     // gefundene Servos frei geben
    }
  }
  IOTW_PRINT(F("\n")); IOTW_PRINT(count); IOTW_PRINTLN(F(" servos detected"));
  return count;
}


// --------------------------------------------------
// Set
// --------------------------------------------------
void feetech_sts_set(uint8_t id, int mode, float wert) {
  if (st.pSerial == nullptr) return;
  if (IOTW_debug_level > 1) { IOTW_PRINT(F("\n servo id ")); IOTW_PRINT(id); }

  switch (mode) {

    case 0: {  // Zielposition in Grad (0..360)
      wert = constrain(wert, 0.0f, STS_RANGE_DEG);
      int pos_raw = (int)(wert * STS_STEPS_REV / STS_RANGE_DEG + 0.5f);
      sts_goto(id, pos_raw);     // begrenzt auf 4095
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" angle ")); IOTW_PRINT(wert); IOTW_PRINT(F(" raw ")); IOTW_PRINT(pos_raw); }
      break;
    }

    case 1: {  // Torque an/aus
      uint8_t en = (wert != 0.0f) ? 1 : 0;
      if (en) sts_hold_position(id);   // kein Sprung zu einem alten Ziel
      st.EnableTorque(id, en);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" torque enable ")); IOTW_PRINT(en); }
      break;
    }

    case 2: {  // Torque-Limit 0..100 % (RAM Reg 48, 0..1000)
      float percent = constrain(wert, 0.0f, 100.0f);
      uint16_t torque_raw = (uint16_t)(percent * 10.0f + 0.5f);
      st.writeWord(id, SMS_STS_TORQUE_LIMIT_L, torque_raw);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" torque limit raw ")); IOTW_PRINT(torque_raw); }
      break;
    }

    case 3: {  // Max Speed fuer Positionsfahrten in % (0 = Servo-Maximum)
      float percent = constrain(fabsf(wert), 0.0f, 100.0f);
      sts_speed[id] = (uint16_t)(percent / 100.0f * STS_MAX_SPEED + 0.5f);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" max speed % ")); IOTW_PRINT(percent); IOTW_PRINT(F(" (steps/s ")); IOTW_PRINT(sts_speed[id]); IOTW_PRINTLN(F(")")); }
      break;
    }

    case 4: {  // Zielposition in Rohwerten (0..4095)
      sts_goto(id, (int)wert);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" position ")); IOTW_PRINT((int)wert); }
      break;
    }

    case 5: {  // Wheel-Mode an/aus (Reg 33 liegt im EEPROM -> Lock oeffnen, danach schliessen)
      bool wheel = (wert > 0.5f);
      st.EnableTorque(id, 0);
      delay(10);
      st.unLockEprom(id);
      delay(5);
      st.writeByte(id, SMS_STS_MODE, wheel ? 1 : 0);   // 0 = Position, 1 = Wheel (geschlossene Regelung)
      delay(10);
      st.LockEprom(id);
      delay(10);
      if (wheel) st.WriteSpe(id, 0, STS_ACC);          // sicher mit Stillstand starten
      else       sts_hold_position(id);                // Position halten statt zu springen
      st.EnableTorque(id, 1);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" wheel mode ")); IOTW_PRINT(wheel); }
      break;
    }

    case 6: {  // Wheel-Geschwindigkeit -100..+100 % (nur im Wheel-Mode wirksam)
      float percent = constrain(wert, -100.0f, 100.0f);
      int16_t spd = (int16_t)roundf(percent / 100.0f * STS_MAX_SPEED);
      st.WriteSpe(id, spd, STS_ACC);                   // Vorzeichenbit 15 setzt die Lib
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" wheel speed % ")); IOTW_PRINT(percent); IOTW_PRINT(F(" (steps/s ")); IOTW_PRINT(spd); IOTW_PRINTLN(F(")")); }
      break;
    }

    case 7: {  // neue ID (EEPROM)
      int id_w = constrain((int)wert, 0, 253);
      st.unLockEprom(id);
      st.writeByte(id, SMS_STS_ID, (u8)id_w);
      delay(10);
      st.LockEprom((u8)id_w);
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" id change to ")); IOTW_PRINT(id_w); }
      break;
    }

    default:
      if (IOTW_debug_level > 0) { IOTW_PRINT(F(" unknown set mode ")); IOTW_PRINTLN(mode); }
      break;
  } // switch
} // function


// --------------------------------------------------
// Get
// --------------------------------------------------
float feetech_sts_get(uint8_t id, int mode) {
  if (st.pSerial == nullptr) return NAN;
  float out = NAN;
  if (IOTW_debug_level > 1) { IOTW_PRINT(F("\n servo id ")); IOTW_PRINT(id); }

  switch (mode) {

    case 0: {  // Winkel in Grad (0..360)
      int raw = st.ReadPos(id);
      if (raw >= 0) out = raw * STS_RANGE_DEG / STS_STEPS_REV;   // -1 = Fehler
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. angle ")); IOTW_PRINTLN(out); }
      break;
    }

    case 1: {  // Torque an/aus
      int v = st.readByte(id, SMS_STS_TORQUE_ENABLE);
      if (v >= 0) out = (float)v;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. torque enable ")); IOTW_PRINTLN(out); }
      break;
    }

    case 2: {  // Last in % mit Vorzeichen (Lib liefert -1000..+1000 = +-100.0 %)
      int raw = st.ReadLoad(id);
      if (raw != -1) out = raw / 10.0f;   // -1 = Fehler (Last -0.1 % geht dabei verloren)
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" load raw ")); IOTW_PRINT(raw); IOTW_PRINT(F(" -> % ")); IOTW_PRINTLN(out); }
      break;
    }

    case 3: {  // faehrt gerade
      int v = st.ReadMove(id);
      if (v >= 0) out = (float)v;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" is moving ")); IOTW_PRINTLN(out); }
      break;
    }

    case 4: {  // Temperatur in Grad C
      int v = st.ReadTemper(id);
      if (v >= 0) out = (float)v;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. temperature ")); IOTW_PRINTLN(out); }
      break;
    }

    case 5: {  // Position in Rohwerten
      int raw = st.ReadPos(id);
      if (raw >= 0) out = (float)raw;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. position ")); IOTW_PRINTLN(out); }
      break;
    }

    case 6: {  // Ping
      out = (st.Ping(id) >= 0) ? 1.0f : 0.0f;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" ping ")); IOTW_PRINTLN(out); }
      break;
    }

    case 7: {  // Strom, Rohwert mit Vorzeichen (Einheit je LSB laut Feetech-Datenblatt pruefen)
      int raw = st.ReadCurrent(id);
      if (raw != -1) out = (float)raw;   // -1 = Fehler
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" current raw ")); IOTW_PRINTLN(out); }
      break;
    }

    case 8: {  // Versorgungsspannung in V (Servo liefert 0.1 V pro Schritt)
      int v = st.ReadVoltage(id);
      if (v >= 0) out = v / 10.0f;
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" voltage V ")); IOTW_PRINTLN(out); }
      break;
    }

    default:
      if (IOTW_debug_level > 0) { IOTW_PRINT(F(" unknown get mode ")); IOTW_PRINTLN(mode); }
      break;
  }

  return out;
}

#endif
