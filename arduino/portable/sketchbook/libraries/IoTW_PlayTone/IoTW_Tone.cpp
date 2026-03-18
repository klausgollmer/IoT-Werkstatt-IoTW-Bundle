#include "IoTW_Tone.h"
#include "Arduino.h"
int PlayTimeSEC  = 10;  // song duration
int PlayStopMS   = 0;   // stop playing
#define LEDC_RESOLUTION 10  // Set resolution to 10 bits

void beep(int f, int dur) {
	#if !defined(ESP32)
   	    Serial.println(F("ESP32 only"));
		return;
    #endif	
	
  // https://garrysblog.com/2022/12/16/experimenting-with-audio-tones-using-the-esp32-for-use-in-projects/
  if (millis() < PlayStopMS) {
    ledcAttach(IOTW_GPIO_AUDIO_OUT, 50, LEDC_RESOLUTION); // Attach pin before starting tone
    ledcWriteTone(IOTW_GPIO_AUDIO_OUT, f);
    delay(dur); 
    ledcDetach(IOTW_GPIO_AUDIO_OUT); // Detatch pin to stop the tone   
    delay(dur*0.25);
  }
}


void BuzzerTone(int f, int dur) {
	#if !defined(ESP32)
   	    Serial.println(F("ESP32 only"));
		return;
    #endif	

	
   if (f > 0) {
    ledcAttach(IOTW_GPIO_AUDIO_OUT, 50, LEDC_RESOLUTION); // Attach pin before starting tone
    ledcWriteTone(IOTW_GPIO_AUDIO_OUT, f);
    if  (dur>0) { 
		delay(dur); 
		ledcDetach(IOTW_GPIO_AUDIO_OUT); // Detatch pin to stop the tone   
	}
   } else {
    ledcDetach(IOTW_GPIO_AUDIO_OUT); // Detatch pin to stop the tone   
  }
}


//******* Happy Birthday ****************
void HappyBirthday(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_G3, 200);
  beep(NOTE_G3, 200);
  beep(NOTE_A3, 500);
  beep(NOTE_G3, 500);
  beep(NOTE_C4, 500);
  beep(NOTE_B3, 1000);
  beep(NOTE_G3, 200);
  beep(NOTE_G3, 200);
  beep(NOTE_A3, 500);
  beep(NOTE_G3, 500);
  beep(NOTE_D4, 500);
  beep(NOTE_C4, 1000);
  beep(NOTE_G3, 200);
  beep(NOTE_G3, 200);
  beep(NOTE_G4, 500);
  beep(NOTE_E4, 500);
  beep(NOTE_C4, 500);
  beep(NOTE_B3, 500);
  beep(NOTE_A3, 750);
  beep(NOTE_F4, 200);
  beep(NOTE_F4, 200);
  beep(NOTE_E4, 500);
  beep(NOTE_C4, 500);
  beep(NOTE_D4, 500);
  beep(NOTE_C4, 1000); 
}
  
  
//******* Song1 ****************
void Song1(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_B4, 333);  
  beep(NOTE_E5, 1666);    
}
  
void Song2(int PlayTimeSEC) {  
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_E4, 125);
  beep(NOTE_A4, 500);
}
  
void Song3(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_F5, 166);
  beep(NOTE_C6, 666);  
}
  
void Song4(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_D5, 214);
  beep(NOTE_D6, 856);  
}
  
void Song5(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_GS5, 240);
  beep(NOTE_FS5, 480);
}
  
  


