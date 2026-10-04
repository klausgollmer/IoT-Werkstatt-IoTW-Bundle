/*
*  Copyright (c) 2026  IoT2-Werkstatt <Klaus-Uwe Gollmer>
*  SPDX-License-Identifier: MIT
*  Diese Datei ist Teil der Bibliothek IoT-Werkstatt
*  nutzt FTServ_Arduino, tested with STS3215
*  https://github.com/ftservo/FTServo_Arduino
*  MIT license
*  max. 20 servo (ID 0 to 20) im Scan
*
*  Unterschiede zu IoTW_Feetech_SCL.cpp (SC09/SCSCL):
*    - Klasse: SMS_STS statt SCSCL
*    - Auflösung: 4096 Steps / 360° statt 1023 / 300°
*    - Byte-Reihenfolge: Little-Endian (automatisch per SMS_STS)
*    - Speed: Bit15=Richtung statt Offset 1024
*    - Register: teilweise andere Adressen
*/

#ifdef ESP32

#include <SCServo.h>      // enthält SCSCL und SMS_STS
#include <IoTW_config.h>
#include <math.h>
#include <Arduino.h>

extern int IOTW_debug_level;
#define MAX_FT_ID_SCAN    20
#define STS_RANGE_DEG     360.0
#define STS_STEPS         4095.0   // 0..4095

// Register (STS-Serie)
#define STS_TORQUE_ENABLE       40
#define STS_MAX_TORQUE_L        16
#define STS_GOAL_POSITION_L     42
#define STS_GOAL_SPEED_L        46
#define STS_PRESENT_POSITION_L  56
#define STS_PRESENT_SPEED_L     58
#define STS_PRESENT_LOAD_L      60
#define STS_PRESENT_VOLTAGE     62
#define STS_PRESENT_TEMPERATURE 63
#define STS_MOVING              66
#define BROADCAST_ID 0xFE

SMS_STS st;

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
    IOTW_PRINT(F("scan id 1-20, torque off: "));
    st.writeByte(BROADCAST_ID, STS_TORQUE_ENABLE, 0);  // Alle OFF!
    delay(10);

    for (uint8_t id = 0; id <= MAX_FT_ID_SCAN; id++) {
        if (st.Ping(id) >= 0) {
            IOTW_PRINT(id); IOTW_PRINT(F(" ")); count++;
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
        case 0: {  // Angle → Pos  (0..360°)
            wert = constrain(wert, 0, 360);
            uint16_t pos_raw = (uint16_t)(wert * STS_STEPS / STS_RANGE_DEG + 0.5);
            st.EnableTorque(id, 1);
            st.WritePosEx(id, pos_raw, 0, 0);  // pos, time, speed  (0=max speed)
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" angle ")); IOTW_PRINT(wert); IOTW_PRINT(F(" raw ")); IOTW_PRINT(pos_raw); }
            break;
        }

        case 1: {  // Torque enable/disable
            st.writeByte(id, STS_TORQUE_ENABLE, (uint8_t)(wert != 0));
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" torque enable ")); IOTW_PRINT(wert); }
            break;
        }

        case 2: {  // Torque Limit (0–100%)
            float percent = constrain(wert, 0, 100);
            uint16_t torque_raw = (uint16_t)(percent / 100.0 * STS_STEPS + 0.5);
            st.writeWord(id, STS_MAX_TORQUE_L, torque_raw);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" torque limit: ")); IOTW_PRINTLN(torque_raw); }
            break;
        }

        case 3: {  // Speed % bidirectional (-100 CCW .. +100 CW)
            // STS: Bit15=Richtung (0=CW, 1=CCW), Bits 0-14 = Betrag
            float percent = constrain(wert, -100, 100);
            uint16_t speed_abs = (uint16_t)(fabs(percent) / 100.0 * 2047.0 + 0.5);
            uint16_t raw = (percent < 0) ? (0x8000 | speed_abs) : speed_abs;
            st.writeWord(id, STS_GOAL_SPEED_L, raw);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" speed ±%: ")); IOTW_PRINT(percent); IOTW_PRINT(F(" (raw ")); IOTW_PRINT(raw); IOTW_PRINTLN(F(")")); }
            break;
        }

        case 4: {  // Goal Position (raw units 0..4095)
            uint16_t pos_raw = (uint16_t)(wert);
            st.EnableTorque(id, 1);
            st.WritePosEx(id, pos_raw, 0, 0);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" position ")); IOTW_PRINT(pos_raw); }
            break;
        }

        case 5: {  // Wheel mode on/off
            st.writeByte(id, STS_TORQUE_ENABLE, 0);
            if (wert > 0.5) {
                st.WheelMode(id);
            } else {
                // Servo-Modus wiederherstellen: Min/Max-Pos auf Vollbereich
                st.unLockEprom(id);
                st.writeWord(id, 9,  0);
                st.writeWord(id, 11, 4095);
                st.LockEprom(id);
            }
            delay(50);
            st.writeByte(id, STS_TORQUE_ENABLE, 1);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" wheel mode ")); IOTW_PRINT(wert); }
            break;
        }

        case 6: {  // Wheel speed (-100..+100%)
            wert = constrain(wert, -100.0f, 100.0f);
            uint16_t speed_abs = (uint16_t)(fabs(wert) / 100.0 * 2047.0 + 0.5);
            uint16_t raw = (wert < 0) ? (0x8000 | speed_abs) : speed_abs;
            st.writeWord(id, STS_GOAL_SPEED_L, raw);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" wheel speed in % =")); IOTW_PRINTLN(wert); }
            break;
        }

        case 7: {  // ID ändern
            int id_w = (int)wert;
            st.unLockEprom(id);
            st.writeByte(id, 5, id_w);  // Addr 5 = Servo ID Register
            st.LockEprom(id_w);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" id change to ")); IOTW_PRINT(id_w); }
            break;
        }
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
        case 0: {  // Angle (0..360°)
            int raw = st.ReadPos(id);
            if (raw < 0) { out = NAN; break; }
            out = raw * STS_RANGE_DEG / STS_STEPS;
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. angle ")); IOTW_PRINTLN(out); }
            break;
        }

        case 1: {  // Torque enable
            out = st.readByte(id, STS_TORQUE_ENABLE);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. torque enable ")); IOTW_PRINT(out); }
            break;
        }

        case 2: {  // Load (±100%)
            int16_t raw_load = st.ReadLoad(id);
            if (raw_load == -1) {
                out = NAN;
            } else {
                // STS: Bit15=Richtung, Bits 0-9 = Betrag (0..1000)
                int16_t sign = (raw_load & 0x8000) ? -1 : 1;
                out = sign * (raw_load & 0x3FF) * 100.0 / 1000.0;
            }
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" load signed %: ")); IOTW_PRINTLN(out); }
            break;
        }

        case 3: {  // is Moving
            out = st.readByte(id, STS_MOVING);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" is moving ")); IOTW_PRINTLN(out); }
            break;
        }

        case 4: {  // Temperature
            out = st.readByte(id, STS_PRESENT_TEMPERATURE);
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. temperature ")); IOTW_PRINTLN(out); }
            break;
        }

        case 5: {  // Position (raw 0..4095)
            int raw = st.ReadPos(id);
            out = (raw < 0) ? NAN : (float)raw;
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" curr. position ")); IOTW_PRINTLN(out); }
            break;
        }

        case 6: {  // Ping
            out = (st.Ping(id) >= 0) ? 1.0f : 0.0f;
            if (IOTW_debug_level > 1) { IOTW_PRINT(F(" ping ")); IOTW_PRINTLN(out); }
            break;
        }
    }

    return out;
}

#endif
