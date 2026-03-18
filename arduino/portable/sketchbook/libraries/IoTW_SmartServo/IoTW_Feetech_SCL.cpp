/*
*  Copyright (c) 2026  IoT2-Werkstatt <Klaus-Uwe Gollmer>
*  SPDX-License-Identifier: MIT
*  Diese Datei ist Teil der Bibliothek IoT-Werkstatt
*  nutzt FTServ_Arduino, tested with SC09 Waveshare
*  https://github.com/ftservo/FTServo_Arduino
*  MIT license 
*  max. 20 servo (ID 0 to 20) im Scan
*/

#ifdef ESP32

#include <SCServo.h>      // enthält konkrete Implementierung
#include <IoTW_config.h>
#include <math.h>
#include <Arduino.h>

extern int IOTW_debug_level;
#define MAX_FT_ID_SCAN   20
#define FT_RANGE_DEG     300.0

// Register (SCS Serie Standardwerte)
#define SCS_TORQUE_ENABLE        40
#define SCS_MAX_TORQUE_L         14
#define SCS_GOAL_POSITION_L      42
#define SCS_GOAL_SPEED_L 46
#define SCS_PRESENT_POSITION_L   56
#define SCS_PRESENT_SPEED_L      58
#define SCS_PRESENT_LOAD_L       60
#define SCS_PRESENT_VOLTAGE      62
#define SCS_PRESENT_TEMPERATURE  63
#define SCS_MOVING               66
#define SCS_PRESENT_CURRENT_L    69   // falls vorhanden
#define BROADCAST_ID 0xFE

SCSCL sc;
  
void feetech_scs_begin(int ser,int rx, int tx, int baud) {
	switch (ser) {
	 case 0: 
       Serial.begin(baud,SERIAL_8N1, rx, tx);
       sc.pSerial = &Serial;
	 break;
	 
	 case 1: 
       Serial1.begin(baud,SERIAL_8N1, rx, tx);
       sc.pSerial = &Serial1;
	 break;
	 
	 case 2: 
       Serial2.begin(baud,SERIAL_8N1, rx, tx);
       sc.pSerial = &Serial2;
	 break;
	 
     case 3: 
       sc.pSerial = nullptr;
       IOTW_PRINT(F("sorry, no softwareserial on feetech "));
	   return;
	 break;
	 
	 default:
       sc.pSerial = nullptr;
       IOTW_PRINT(F("unknown serial in feetech "));
	   return;
  }
   IOTW_PRINT(F("feetech ready on ser=")); IOTW_PRINT(ser);IOTW_PRINT(F(" baud=")); IOTW_PRINTLN(baud);
}



// --------------------------------------------------
// Scan
// --------------------------------------------------
int feetech_scs_scan() {
  int count = 0;
  if (sc.pSerial == nullptr) return 0;
  IOTW_PRINT(F("scan id 1-20, torque off: "));
  sc.writeByte(BROADCAST_ID, SCS_TORQUE_ENABLE, 0);  // Alle OFF!
  delay(10);  // Settle
  
  for (uint8_t id = 0; id <= MAX_FT_ID_SCAN; id++){
    if (sc.Ping(id) >= 0) {
	 IOTW_PRINT(id);IOTW_PRINT(F(" "));count++;
	}
  }
  IOTW_PRINT(F("\n"));IOTW_PRINT(count);IOTW_PRINTLN(F(" servos detected"));
  return count;
}


// --------------------------------------------------
// Set
// --------------------------------------------------
void feetech_scs_set(uint8_t id, int mode, float wert)
{
  if (sc.pSerial == nullptr) return;
  if (IOTW_debug_level > 1) {IOTW_PRINT(F("\n servo id "));IOTW_PRINT(id);}
  switch (mode) {
    case 0: {  // Angle → Pos
	  wert = constrain(wert, 0, 300);
	  uint16_t pos_raw = (uint16_t)(wert * 1023.0 / 300.0 + 0.5);
      sc.EnableTorque(id, 1);
	  sc.writeWord(id, SCS_GOAL_POSITION_L , pos_raw);  // Fährt mit gespeichertem Speed! 
      if (IOTW_debug_level > 1){IOTW_PRINT(F(" angle "));IOTW_PRINT(wert);IOTW_PRINT(F(" raw "));IOTW_PRINT(pos_raw);}
      break;
    }

    case 1: {   // Torque enable/disable
	  sc.writeByte(id, SCS_TORQUE_ENABLE, (uint8_t)(wert != 0));
	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" torque enable " ));IOTW_PRINT(wert);}
	  break;
    }

    case 2: {  // Torque Limit (0–100%)
	  float percent = constrain(wert, 0, 100);
      uint16_t torque_raw = (uint16_t)(percent / 100.0 * 1023.0 + 0.5);
	  sc.writeWord(id, SCS_MAX_TORQUE_L, torque_raw);  // 
	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" torque limit: ")); IOTW_PRINTLN(torque_raw); }
	  break;
    }
   
    case 3: {  // Speed % bidirectional (-100 CCW .. +100 CW)
	  float percent = constrain(wert, -100, 100);
	  // Safe Range: ±90% statt ±100 (Deadzone vermeiden)
	  percent = constrain(percent, -90, 90);
	  int16_t vel = (int16_t)(percent / 100.0 * 1023);
	  uint16_t raw = 1024 + vel;  // 112..1936 (safe!)
	  sc.writeWord(id, SCS_GOAL_SPEED_L, raw);
	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" speed ±%: ")); IOTW_PRINT(percent);IOTW_PRINT(F(" (raw ")); IOTW_PRINT(raw); IOTW_PRINTLN(F(")"));}
	  break;
	}

	case 4: {  // Goal Position (unit)
      uint16_t pos_raw = (uint16_t)(wert);
	  sc.EnableTorque(id, 1);
	  sc.writeWord(id, SCS_GOAL_POSITION_L , pos_raw);  // Fährt mit gespeichertem Speed! 
      if (IOTW_debug_level > 1) { IOTW_PRINT(F(" position ")); IOTW_PRINT(pos_raw);}
      break;
    }
	
	case 5: {  // continous on/off
       sc.writeByte(id, SCS_TORQUE_ENABLE, 0);
   	  if (wert > 0.5) {
		    sc.PWMMode(id);  
		  } else {
              sc.writeWord(id, 9, 0);
			  sc.writeWord(id, 11, 1023);
		      sc.writeByte(id, 19, 0);
		  }
	   delay(50);
       sc.writeByte(id, SCS_TORQUE_ENABLE, 1);
      if (IOTW_debug_level > 1){IOTW_PRINT(F(" wheel mode "));IOTW_PRINT(wert);}
      break;
    }
   
    case 6: {  // Velocity speed wheel mode
	  wert = constrain(wert, -100.0f, 100.0f);  // float -100 ... 100 Percent
      sc.WritePWM(id, wert*10);
	  if (IOTW_debug_level > 1) {
		IOTW_PRINT(F(" wheel speed in % =")); IOTW_PRINTLN(wert);
	  }
    break;
    }

   	case 7: {   // id
	 int id_w = wert;
     sc.unLockEprom(id); //Unlock EPROM-SAFE
     sc.writeByte(id, SCSCL_ID, id_w);//Change ID
     sc.LockEprom(id_w); // Lock EPROM-SAFE 
     if (IOTW_debug_level > 1){IOTW_PRINT(F(" id change to "));IOTW_PRINT(id_w);}
    break;
   }
 } // switch
} // function



// --------------------------------------------------
// Get
// --------------------------------------------------
float feetech_scs_get(uint8_t id, int mode) {
  if (sc.pSerial == nullptr) return NAN;
  float out = NAN;
  if (IOTW_debug_level > 1) {IOTW_PRINT(F("\n servo id "));IOTW_PRINT(id);}
  switch (mode){
    case 0: {   // Angle
	  int raw = sc.readWord(id, SCS_PRESENT_POSITION_L);  // Addr 56 ✅
	  out = raw * FT_RANGE_DEG / 1023.0;                 // 0-1023 → 0-300° ✅
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" curr. angle "));IOTW_PRINTLN(out);}
	  break;
	}


    case 1: {   // Torque enable
      out = sc.readByte(id, SCS_TORQUE_ENABLE);
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" curr. torque enable "));IOTW_PRINT(out);}
      break;
    }

    case 2: {   // Load (unit)
      int16_t raw_load = sc.ReadLoad(id);  // Signed! -32768..32767
      if (raw_load == -1) {
        out = NAN;  // Error
      } else {
        out = raw_load * 100.0 / 1023.0;  // ±100%
      }
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" load signed %: ")); IOTW_PRINTLN(out);}
      break;
    }

    case 3: {   // is Moving
      out = sc.readByte(id, SCS_MOVING);
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" is moving "));IOTW_PRINTLN(out);}
      break;
    }

    case 4: {  // Temperature
      out = sc.readByte(id, SCS_PRESENT_TEMPERATURE);
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" curr. temperature "));IOTW_PRINTLN(out);}
	  break;
    }

    case 5: { // Position
      out = sc.readWord(id, SCS_PRESENT_POSITION_L);
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" curr. position "));IOTW_PRINTLN(out);}
      break;
    }

   case 6: { // Ping
      out = (sc.Ping(id) >= 0);
	  if (IOTW_debug_level > 1){IOTW_PRINT(F(" ping "));IOTW_PRINTLN(out);}
      break;
    }

  }

  return out;
}

#else
//void feetech_scs_begin(int ser,int rx, int tx, int baud) {}
//int   feetech_scs_scan() { return 0; }
//void  feetech_scs_set(uint8_t id, int mode, float wert) {}
//float feetech_scs_get(uint8_t id, int mode) { return NAN; }
#endif
