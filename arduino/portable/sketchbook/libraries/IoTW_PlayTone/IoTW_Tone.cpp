// Lib for tone output on DAC 1 (GPIO25)
// DAC-Audio from https://github.com/bhall66/DacTone/blob/main/DacTone.cpp
// Bruce E. Hall , MIT License
/**************************************************************************
       Title:   Sound Effects (dacTone Example)
        Date:   05 Dec 2020
      Author:   Bruce E. Hall, w8bh.net
       Legal:   Open Source under the terms of the MIT License 
                                
 Description:   Demonstrate a few sound effects
                using the super-fast setFrequency() routine
                Listen with piezo or audio amp/headphones on GPIO25
          
***************************************************************************/


#include "IoTW_Tone.h"
#include "Arduino.h"
#include "DacTone.h"
int PlayTimeSEC  = 10;  // song duration
int PlayStopMS   = 0;   // stop playing
#define LEDC_RESOLUTION 10  // Set resolution to 10 bits

DacTone audio(0);  
DacTone audio2(1);  

// int count=4;


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


void IoTW_Tone_BuzzerTone(int f, int dur, float vol) {
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
  
  /*
  audio.setVolume((int) vol);
  if (f > 0) {
    audio.tone(f);
    if  (dur>0) { 
		delay(dur); 
	   audio.noTone();
	}
   } else {
     audio.noTone();
  }
 */

 
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

void Song1(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  beep(NOTE_GS5, 240);
  beep(NOTE_FS5, 480);
}
  
  
  
  void whoop() {
  for (int i=10; i<210; i++) {                // 200 tones in all
     audio.setFrequency(6,i);                  // output a tone
     delay(1);                                 // and wait 1mS
   }
}

void unWhoop() {
   for (int i=210; i>10; i--) {                // 200 tones in all
     audio.setFrequency(6,i);                  // output a tone
     delay(1);                                 // and wait 1mS
   }  
}

  
//******* Song2 Sirene ****************
void Song2(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  while (millis() < PlayStopMS) {
     whoop();                                   // sound goes up
     unWhoop();                                 // and sound goes down
  }
  audio.noTone();        
}
// Alarm
void Song3(int PlayTimeSEC) {  
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  while (millis() < PlayStopMS) {
   whoop();  
  }   // phaser sound
  audio.noTone();
 }
  
// Computer
void Song4(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  while (millis() < PlayStopMS) {
		audio.setFrequency(6,random(10,80));       // a few random beeps
		delay(150);
   }
  audio.noTone();  
}
  
// Ticks
void Song5(int PlayTimeSEC) {
  PlayStopMS = millis()+PlayTimeSEC*1000;  // duration to play song  
  while (millis() < PlayStopMS) {
		for (int j=0; j<25; j++) {                 // emit a brief,
		  audio.setFrequency(6,random(200));       // static-like noise
		  delay(1);
		}
		audio.noTone();   
		delay(1000);                               // once a second
  }
}
  
  

void IoTW_Tone_SignalGen(int dac, int vol, float f) { 
  switch (dac) { 
    case 1: { 
     audio.setVolume(vol);
	 if (vol > 0) {
  	   audio.tone(f);
	 } else { 
	   audio.noTone(); 
     }
    }
    break;

    case 2: { 
     audio2.setVolume(vol);
	 if (vol > 0) {
  	   audio2.tone(f);
	 } else { 
	   audio2.noTone(); 
     }
    }
    break;
   default: Serial.print("unknown DAC"); 
   } 
  }
  
void IoTW_Tone_PlaySong(int song, int dur, float vol) {
  audio.setVolume((int) vol);
  switch (song) {
  case 0:
    HappyBirthday(dur);
    break;
  case 1:
    Song1(dur);
    break;
  case 2:
    Song2(dur);
    break;
  case 3:
    Song3(dur);
    break;
  case 4:
    Song4(dur);
    break;
  case 5:
    Song5(dur);
    break;
  }
}