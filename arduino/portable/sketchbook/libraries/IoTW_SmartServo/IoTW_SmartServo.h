#ifndef _IOTW_SMARTSERVO_H_
#define _IOTW_SMARTSERVO_H_

#include <Arduino.h>
#include <math.h>

// =====================================================
// Servo-Typen
// =====================================================

#define SMARTSERVO_ROBOTIS        0
#define SMARTSERVO_FEETECH_SCS    1

// =====================================================
// Debug-Level kommt vom Sketch
// =====================================================

extern int IOTW_debug_level;
static int Aktueller_Typ;

// =====================================================
// Externe Implementierungen (liegen in getrennten .cpp)
// =====================================================

#ifdef ESP32

// --- Robotis ---
void  robotis_begin(int serial, int rx, int tx, int baud);
int   robotis_scan();
void  robotis_set(uint8_t id, int mode, float wert);
float robotis_get(uint8_t id, int mode);

// --- Feetech SCS ---
void  feetech_scs_begin(int serial, int rx, int tx, int baud);
int   feetech_scs_scan();
void  feetech_scs_set(uint8_t id, int mode, float wert);
float feetech_scs_get(uint8_t id, int mode);
#endif

// =====================================================
// Inline Verteiler (Sprungtabelle per Switch)
// =====================================================

inline void SmartServo_start(int type, int serial, int rx, int tx, int baud) {
	int dum;
#ifdef ESP32	
    switch (type) {
        case SMARTSERVO_ROBOTIS:
			robotis_begin(serial,rx,tx, baud);
            Aktueller_Typ = SMARTSERVO_ROBOTIS;
			dum = robotis_scan();
	    break;

        case SMARTSERVO_FEETECH_SCS:
  		    feetech_scs_begin(serial,rx,tx, baud);
            Aktueller_Typ = SMARTSERVO_FEETECH_SCS;
			dum = feetech_scs_scan();
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

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
            return 0;
    }
#else
	return NAN;
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

        default:
            if (IOTW_debug_level > 0) IOTW_PRINT(F("unknown Servo type"));
            return NAN;
    }
#else
	return NAN;
#endif
}

#endif
