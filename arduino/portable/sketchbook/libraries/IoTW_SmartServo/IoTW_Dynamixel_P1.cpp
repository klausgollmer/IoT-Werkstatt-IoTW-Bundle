/*
*  Copyright (c) 2026  IoT2-Werkstatt <Klaus-Uwe Gollmer>
*  SPDX-License-Identifier: MIT
*  Diese Datei ist Teil der Bibliothek IoT-Werkstatt
*  nutzt Robotis dynamixel2arduino lib, tested with AX 12+ (Protocol 1.0)
*   https://github.com/ROBOTIS-GIT/Dynamixel2Arduino
*  Apache-2.0 license Ver 2.0 http://www.apache.org/licenses/
*  max. 20 servo (ID 0 to 20) im Scan
*/
#ifdef ESP32 

#include <Dynamixel2Arduino.h>
#include <IoTW_config.h>
#include <math.h>
#include <Arduino.h>

extern int IOTW_debug_level;
#define MAX_DXL_ID_SCAN 20   
#define DXL_RANGE_DEG 300.0
#define BROADCAST_ID 0xFE

HardwareSerial* activeSerial = nullptr;
Dynamixel2Arduino dxl(Serial2, -1);  // Default Serial2
extern void robotis_set(uint8_t, int , float );

using namespace ControlTableItem;
//------------------------------------- Robotis Begin

void robotis_begin(int ser, int rx, int tx, int baud) {
  switch (ser) {
    case 0: activeSerial = &Serial; break;
    case 1: activeSerial = &Serial1; break;
    case 2: activeSerial = &Serial2; break;
	case 3: activeSerial = nullptr;
       IOTW_PRINT(F("sorry, no softwareserial on dynamixel"));
	   return;
    break;
	 
    default:
      IOTW_PRINT(F("unknown serial in dynamixel "));
      return;
  }

  activeSerial->end();
  activeSerial->begin(baud, SERIAL_8N1, rx, tx);

  dxl = Dynamixel2Arduino(*activeSerial, -1);
  dxl.begin(baud);
  dxl.setPortProtocolVersion(1.0);  // AX-12A+
  IOTW_PRINT(F("Dynamixel ready on ser=")); IOTW_PRINT(ser);IOTW_PRINT(F(" baud=")); IOTW_PRINTLN(baud);
}

int robotis_scan() { 
  if (activeSerial == nullptr) return 0;
  int cnt_p1=0;
  IOTW_PRINT(F("scan id 1-20, position mode, torque off: "));
  dxl.torqueOff(BROADCAST_ID);
  robotis_set(BROADCAST_ID, 3, 100.0f);  // Broadcast: Alle Speed 100% → raw 1023 (MAX!) 
  robotis_set(BROADCAST_ID, 2, 100.0f);  // Broadcast: Torque limit max 

  for (int id = 0; id<=MAX_DXL_ID_SCAN;id++) {
    if (dxl.ping(id)) {
      dxl.setOperatingMode(id, OP_POSITION);
      dxl.torqueOff(id);
      IOTW_PRINT(id);IOTW_PRINT(F(" "));
	  cnt_p1++;
    } 
  }

  IOTW_PRINT(F("\n"));IOTW_PRINT(cnt_p1);IOTW_PRINTLN(F(" servos detected"));
  return cnt_p1;
}

//------------------------------------- Robotis set 
void robotis_set(uint8_t id, int mode, float wert){
  if (activeSerial == nullptr) return;
  int rawpos, speed_raw;
  
  if (IOTW_debug_level > 1) {IOTW_PRINT(F("\n servo id "));IOTW_PRINT(id);}
	
  switch (mode) {
    case 0: { // Goal Angle
      rawpos = (wert * 1023. / 300.0f + 0.5f);
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" position "));IOTW_PRINT(wert);IOTW_PRINT(" raw ");IOTW_PRINT(rawpos);}
      dxl.setGoalPosition(id, rawpos);
	  dxl.torqueOn(id);
	  delay(2);
    break;
	}
    case 1: {// Torque enable
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" torque enable "));IOTW_PRINT(wert);}
      if (wert>=0.5) dxl.torqueOff(id);else dxl.torqueOn(id);
    break;
	}
    case 2: {// Torque Limit in %
	  uint16_t torque_raw = (uint16_t)(wert * 10.23f + 0.5f);  // % → 0-1023
	  torque_raw = constrain(torque_raw, 0, 1023);
	  dxl.writeControlTableItem(TORQUE_LIMIT, id, torque_raw);
  	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" torque limit %"));IOTW_PRINT(wert);}
    break;
	}
    case 3:{ // Moving Speed ±%
	  float percent = constrain(wert, -100.0f, 100.0f);
	  if (percent >= 0) {
		  speed_raw = (uint16_t)(percent * 10.23f + 0.5f);  // 0-1023
	  } else {
		  speed_raw = 1024 + (uint16_t)(-percent * 10.23f + 0.5f);  // 1024-2047 CW
	  }
      if (speed_raw == 1023) speed_raw = 0;  // 100% → MAX!
	  dxl.writeControlTableItem(MOVING_SPEED, id, speed_raw);
	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" speed ±%"));IOTW_PRINT(percent);IOTW_PRINT(F(" raw "));IOTW_PRINT(speed_raw);}
    break;
	}
	case 4: {// Goal Position
      dxl.setGoalPosition(id, (int) wert);
	  dxl.torqueOn(id);
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" position "));IOTW_PRINT(wert);}
    break;
    }
	case 5: {// Wheel Mode ON (Continuous)
	   dxl.torqueOff(id);
	   if (wert > 0.5) {
		   dxl.setOperatingMode(id, OP_VELOCITY);
	   } else {
  	       dxl.setOperatingMode(id, OP_POSITION);
	   }
       dxl.torqueOn(id);
 	   if (IOTW_debug_level > 1) IOTW_PRINTLN(F(" wheel mode ON"));  // Clean
    break;
	}
	case 6: {  // Moving Speed ±% (AX-kompatibel)
     float percent = constrain(wert, -100.0f, 100.0f);
     uint16_t speed_raw;
     if (percent >= 0) {
       speed_raw = (uint16_t)(percent * 10.23f);  // 0-1023 CCW
      } else {
       speed_raw = 1024 + (uint16_t)(-percent * 10.23f);  // 1024-2047 CW (Wheel)
     }
     dxl.writeControlTableItem(MOVING_SPEED, id, speed_raw);  // AX Addr 32!
     IOTW_PRINT(F(" speed ±% ")); IOTW_PRINT(percent); IOTW_PRINT(F(" raw ")); IOTW_PRINTLN(speed_raw);
    break;
    }
	case 7: {// ID Change (1-253)
	  int new_id = constrain(wert, 0,254);
      if(dxl.setID(id, new_id) == true){
		if (IOTW_debug_level > 1) {IOTW_PRINT(F(" ID change to "));IOTW_PRINT((int)wert);}
	  } else {
		if (IOTW_debug_level > 1) {IOTW_PRINT(F(" ID change error "));}
	  }
 	break;
	}
   }
  }


//------------------------------------- Robotis get 
float robotis_get(uint8_t id, int mode){  
  if (activeSerial == nullptr) return NAN;
  float out = NAN;
  int raw;
  if (IOTW_debug_level > 1) {IOTW_PRINT(F("\n servo id "));IOTW_PRINT(id);}

  switch (mode) {
    case 0:{ // current Angle
      raw = dxl.getPresentPosition(id);
	  if (raw == 0) raw = dxl.getPresentPosition(id); // qnd bugfix erste Abfrage 0
      out = raw * DXL_RANGE_DEG / 1023.; 
      if (IOTW_debug_level > 1){IOTW_PRINT(F(" current pos "));IOTW_PRINT(out);}
	break;
	}
    case 1: {// Torque enable
      out = dxl.readControlTableItem(TORQUE_ENABLE, id);
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" current torque enable "));IOTW_PRINT(out);}
    break;
	}
    case 2: { // Signed Load ±%
	  int16_t load_signed;
   	  uint16_t raw = dxl.readControlTableItem(PRESENT_LOAD, id);
	  load_signed = (raw > 1023) ? (int16_t)(raw - 2048) : (int16_t)raw;  // -1023..+1023
	  out = load_signed / 10.23f;  // ±100%
	  if (IOTW_debug_level > 1) {IOTW_PRINT(F(" signed load %"));IOTW_PRINT(out);}
      break;
    }

    case 3:{ // is Moving
      out =  dxl.readControlTableItem(MOVING, id);
      if (IOTW_debug_level > 1) {IOTW_PRINT(F(" is moving "));IOTW_PRINT(out);}
    break;
	}
    case 4: {// Temperature
     out =  dxl.readControlTableItem(PRESENT_TEMPERATURE, id);
     if (IOTW_debug_level > 1) {IOTW_PRINT(F(" temperature "));IOTW_PRINT(out);}
    break;
	}
	case 5:{ // current Position
      raw = dxl.getPresentPosition(id);
	  if (raw == 0) raw = dxl.getPresentPosition(id); // qnd bugfix erste Abfrage 0
      out = raw; 
      if (IOTW_debug_level > 1){IOTW_PRINT(F(" current pos "));IOTW_PRINT(out);}
	break;
	}
	case 6:{ // ping
      out = dxl.ping(id);
      if (IOTW_debug_level > 1){IOTW_PRINT(F(" ping "));IOTW_PRINT(out);}
	break;
	}


	
  } 
  return out;
}
#endif