#ifndef _IOTW_SMARTSERVO_H_
#define _IOTW_SMARTSERVO_H_

#include <Arduino.h>
#include <math.h>

// =====================================================
// Servo-Typen
// =====================================================

#define SMARTSERVO_ROBOTIS        0
#define SMARTSERVO_FEETECH_SCS    1   // SC09, FT01016 etc. (Potentiometer, Big-Endian, 1024 Steps)
#define SMARTSERVO_FEETECH_STS    2   // STS3215 etc.       (Magnetencoder, Little-Endian, 4096 Steps)

// =====================================================
// Debug-Level kommt vom Sketch
// =====================================================

extern int IOTW_debug_level;
static int Aktueller_Typ = -1;   // zuletzt gestarteter Typ (type < 0 nutzt diesen)

// =====================================================
// Externe Implementierungen (liegen in getrennten .cpp)
// =====================================================

#ifdef ESP32

// --- Robotis ---
void  robotis_begin(int serial, int rx, int tx, int baud);
int   robotis_scan();
void  robotis_set(uint8_t id, int mode, float wert);
float robotis_get(uint8_t id, int mode);

// --- Feetech SCS (SC09, Big-Endian, 300°/1024) ---
void  feetech_scs_begin(int serial, int rx, int tx, int baud);
int   feetech_scs_scan();
void  feetech_scs_set(uint8_t id, int mode, float wert);
float feetech_scs_get(uint8_t id, int mode);

// --- Feetech STS (STS3215, Little-Endian, 360°/4096) ---
void  feetech_sts_begin(int serial, int rx, int tx, int baud);
int   feetech_sts_scan();
void  feetech_sts_set(uint8_t id, int mode, float wert);
float feetech_sts_get(uint8_t id, int mode);

#endif

// =====================================================
// Inline Verteiler (Sprungtabelle per Switch)
// =====================================================

inline void SmartServo_start(int type, int serial, int rx, int tx, int baud) {
    int dum;
#ifdef ESP32
    switch (type) {
        case SMARTSERVO_ROBOTIS:
            robotis_begin(serial, rx, tx, baud);
            Aktueller_Typ = SMARTSERVO_ROBOTIS;
            dum = robotis_scan();
        break;

        case SMARTSERVO_FEETECH_SCS:
            feetech_scs_begin(serial, rx, tx, baud);
            Aktueller_Typ = SMARTSERVO_FEETECH_SCS;
            dum = feetech_scs_scan();
        break;

        case SMARTSERVO_FEETECH_STS:
            feetech_sts_begin(serial, rx, tx, baud);
            Aktueller_Typ = SMARTSERVO_FEETECH_STS;
            dum = feetech_sts_scan();
        break;

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
        break;
    }
#endif
}


inline int SmartServo_scan(int type) {
#ifdef ESP32
    if (type < 0) type = Aktueller_Typ;
    switch (type)
    {
        case SMARTSERVO_ROBOTIS:
            return robotis_scan();

        case SMARTSERVO_FEETECH_SCS:
            return feetech_scs_scan();

        case SMARTSERVO_FEETECH_STS:
            return feetech_sts_scan();

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
            return 0;
    }
#else
    return 0;
#endif
}


inline void SmartServo_set(int type, uint8_t id, int mode, float wert) {
#ifdef ESP32
    if (type < 0) type = Aktueller_Typ;
    switch (type)
    {
        case SMARTSERVO_ROBOTIS:
            robotis_set(id, mode, wert);
            break;

        case SMARTSERVO_FEETECH_SCS:
            feetech_scs_set(id, mode, wert);
            break;

        case SMARTSERVO_FEETECH_STS:
            feetech_sts_set(id, mode, wert);
            break;

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
            break;
    }
#endif
}


inline float SmartServo_get(int type, uint8_t id, int mode) {
#ifdef ESP32
    if (type < 0) type = Aktueller_Typ;
    switch (type)
    {
        case SMARTSERVO_ROBOTIS:
            return robotis_get(id, mode);

        case SMARTSERVO_FEETECH_SCS:
            return feetech_scs_get(id, mode);

        case SMARTSERVO_FEETECH_STS:
            return feetech_sts_get(id, mode);

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
            return NAN;
    }
#else
    return NAN;
#endif
}

#endif
