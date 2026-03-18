/*--------------------------------------------------
  Photometer Programm
  ein LED Photometer mit dem Lichtsensor BH1750
  einer dreifarbigen LED (RGB-LED) und
  mit eigenem WIFI AccessPoint und kleinem Webserver

  HAW Hamburg - Labor BPA - Ulrich Scheffler
  Version 0.30, 02.11.2017
  modified for iot-werkstatt by iot-werkstatt umwelt-campus Birkenfeld oct 2019
   --------------------------------------------------*/
#include <DIY_Spectrometer_BH.h>
bool server_init = false;   
int led_farben_max = 1;          // Anzahl der LED Farben :=1 für eine einfarbige LED
int NeoNo =1;                    // linker=1/rechter=0 Neopixel
int ledGPIOPin=-1;               // >= 0 gültiger GPIOPIN
String sVersion = "Version 0.30BH -";
String sVersion2 = " vom 02.11.2017 - HAW Hamburg, Lab. BPA, U. Scheffler";

unsigned long ulReqcount;                   // Zähler für die Aufrufe der Webseite
const String farbkennung[] = {"Gr&uumlne LED &nbsp", "Rote LED &nbsp", "Blaue LED &nbsp", "Gr&uumlne, rote und blaue LED"}; // Texte für die RGB-Auswahl Buttons (Grün, Rot, Blau)
const int sdaPin =  4;                      // SDA an GPIO/Pin  4 / D2   Anschluss-Pin für das SDA-Signal zur Datenkommunikation mit dem Lichtsensor
const int sclPin =  5;                      // SCL an GPIO/Pin  5 / D1   Anschluss-Pin für das SCL-Signal zur Datenkommunikation mit dem Lichtsensor
const int data_runs  = 7;                   // Anzahl der Wiederholungsmessungen aus denen dann ein Mittelwert gebildet wird
float VIS_IRZEROdata[] = {0.0, 0.0, 0.0};   // Für die aktuellen Nullproben-Messwerte vom Lichtsensor.  Hier der Messwert vom Sensorteil, der im VIS und IR Bereich misst
float IRZEROdata[]     = {0.0, 0.0, 0.0};   // Für die aktuellen Nullproben-Messwerte vom Lichtsensor.  Hier der Messwert vom Sensorteil, der nur im IR Bereich misst
float LUXZEROdata[]    = {0.0, 0.0, 0.0};   // Für die aktuellen Nullproben-Messwerte vom Lichtsensor.  Hier die laut Datenblatt berechnete Beleuchtungsstärke in Lux
float VIS_IRdata[]     = {0.0, 0.0, 0.0};   // Für die aktuellen Proben-Messwerte vom Lichtsensor.  Hier der Messwert vom Sensorteil, der im VIS und IR Bereich misst
float IRdata[]         = {0.0, 0.0, 0.0};   // Für die aktuellen Proben-Messwerte vom Lichtsensor.  Hier der Messwert vom Sensorteil, der nur im IR Bereich misst
float LUXdata[]        = {0.0, 0.0, 0.0};   // Für die aktuellen Proben-Messwerte vom Lichtsensor.  Hier die laut Datenblatt berechnete Beleuchtungsstärke in Lux
float E_LUX[]          = {0.0, 0.0, 0.0};   // Für die berechnete Extiktionen aus den Beleuchtungsstärke-Daten
float E_VIS_IR[]       = {0.0, 0.0, 0.0};   // Für die berechnete Extiktionen aus den Sensor-Daten vom Sensorteil der im VIS und IR Bereich misst
float E_IR[]           = {0.0, 0.0, 0.0};   // Für die berechnete Extiktionen aus den Sensor-Daten vom Sensorteil der nur im IR Bereich misst
int probenzeilenIndex     = 0;              // Zeiger auf die aktuell zu füllende Probenzeile
const int probenzeilenMax = 5;              // Anzahl der Zeilen für Proben in der HTML Tabelle
float LUX_werte[3][probenzeilenMax];        // Array für Proben-Messwerte(Beleuchtungsstärke-Daten) vom Lichtsensor in Lux
float E_werte[3][probenzeilenMax];          // Array für berechnete Extiktionen aus den Beleuchtungsstärke-Daten
String anzeige            = "a";            // Kennung für die Anzeige der Tabellenanteile a=alle, g=grün, r=rot, b=blau
bool download             = false;          // Flag: Wenn True Download der DatenTabelle gewünscht, wenn False dan nicht.
String datentabelle       = "";             // Nimmt Datentabelle für den Download auf
const String trennzeichen = "\t";           // Spalten Trennzeichen für die Datentabelle (\t := Tabulatorzeichen)
uint16_t broadband = 0;                     // Sensor-Daten vom Sensorteil der im VIS und IR Bereich misst
uint16_t infrared  = 0;                     // Sensor-Daten vom Sensorteil der nur im IR Bereich misst
WiFiServer server(80);                      // Eine Instanz auf dem Server für Port 80 erzeugen

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void displaySensorDetails(void) { // Zeigt einige Basisinformationen über den Sensor an
  Serial.println("------------------------------------");
  Serial.print("Sensor: BH1750  200ms    "); 
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void configureSensor(void) { // Verstärkung und Integrationszeit konfigurieren
   Serial.println("------------------------------------");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
bool readSensor(int color) {  // Sensor-Messdaten auslesen
 
  int ok = 0;                 // Zähler für geglückte Messungen
  float LUX[data_runs + 1];   // Daten Array zur Aufname der Einzelmessungen (ein Element mehr als Summenspeicher)
  float VIS_IR[data_runs + 1]; // Daten Array zur Aufname der Einzelmessungen
  float IR[data_runs + 1];    // Daten Array zur Aufname der Einzelmessungen
  float LUX_max = 0.0, LUX_min = 0.0;
  for (int j = 0; j <= data_runs; j++) { // Daten Arrays mit "0.0"-Werten vorbelegen
    LUX[j]    = 0.0;
    VIS_IR[j] = 0.0;
    IR[j]     = 0.0;
  }
  if (led_farben_max > 1) {
    if (color==0) {pixels.setPixelColor(NeoNo,0,255,0);} // G
    if (color==1) {pixels.setPixelColor(NeoNo,255,0,0);} // R
    if (color==2) {pixels.setPixelColor(NeoNo,0,0,255);} // B
    pixels.show();                 // und anzeigen
  } else {
	  if (ledGPIOPin>=0) {
		 digitalWrite(ledGPIOPin,HIGH);
        }else {
         pixels.setPixelColor(NeoNo,0,0,0,255); // W
         pixels.show();                 // und anzeigen
		}
  }
  delay(180); // Warte, bis LED stabil
  for (int j = 0; j <= data_runs - 1; j++) { // Messungen "data_runs"-mal wiederholen und Einzelmessungen in Daten Array eintragen
    if (1) {                  // Wird nur dann "Wahr" wenn erfolgreich gemesen wurde
      LUX[j] = (LightSensor.readLightLevel());     // LUX-Wert abholen
      if (j == 0) {
        LUX_max = LUX[j];  // Min- und Max-Werte am Anfang auf den ersten Messwert setzten
        LUX_min = LUX[j];
      }
      if (LUX[j] > LUX_max) LUX_max = LUX[j];          // Neuen Max-Wert suchen und ggf neu setzten
      if (LUX[j] < LUX_min) LUX_min = LUX[j];          // Neuen Min-Wert suchen und ggf neu setzten
      delay(10);                                       // Zwischen den Messungen warten (Zeit in Milli-Sekunden)
      ok += 1;                                         // Zähler für geglückte Messungen um 1 erhöhen
    }
    else { // Wenn "event.light = 0 lux" ist, dann ist der Sensor möglicher Weise auch gesättigt und es können keinen Daten generiert werden!
      Serial.println("Der Sensor-Messwert ist unbrauchbar. -> Sensor fehler!");
  } }

    if (ledGPIOPin>=0) {  
		 digitalWrite(ledGPIOPin,LOW);
	} else {
         pixels.setPixelColor(NeoNo,0,0,0,0); // alle aus
		 pixels.show();
	}     
    if (ok >= data_runs) {                   // Nur wenn alle Einzelmessungen erfolgreich durchgeführt wurden ...
    for (int j = 0; j <= data_runs - 1; j++) { // Einzelmessungen aufaddieren
      LUX[data_runs]    += LUX[j];
      VIS_IR[data_runs] += VIS_IR[j];
      IR[data_runs]     += IR[j];
    }
    // Die aufaddierte Summe der Einzelwerte ohne den Max-Wert und den Min-Wert geteilt durch Anzahl der Einzelmessungen minus 2 ergibt den bereinigten Mittelwert
    LUX[data_runs]    = (LUX[data_runs] - LUX_max - LUX_min)    / (data_runs * 1.0 - 2.0);
    // Die aufaddierte Summe der Einzelwerte geteilt durch Anzahl der Einzelmessungen ergibt den jeweiligen Mittelwert
    VIS_IR[data_runs] = VIS_IR[data_runs] / (data_runs * 1.0);
    IR[data_runs]     = IR[data_runs]     / (data_runs * 1.0);
    LUXdata[color]    = LUX[data_runs];    // Berechneten LUX-Wert in die Datentabelle übertragen
    VIS_IRdata[color] = VIS_IR[data_runs]; // Berechneten VIS-IR-Wert in die Datentabelle übertragen
    IRdata[color]     = IR[data_runs];     // Berechneten IR-Wert in die Datentabelle übertragen
    return true;
  }
  else {
    return false;
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void cleardata() {
  Serial.println("Cleardata(RESET) aufgerufen!" );
  for (int zeilennummer = 0; zeilennummer < probenzeilenMax; zeilennummer++) { // Datentabelle mit "0.0"-Werten vorbelegen
    for (int ledfarbe = 0; ledfarbe < led_farben_max; ledfarbe++) {
      LUX_werte[ledfarbe][zeilennummer] = 0.0;
      E_werte[ledfarbe][zeilennummer]   = 0.0;
      LUXZEROdata[ledfarbe]             = 0.0;
      VIS_IRZEROdata[ledfarbe]          = 0.0;
      IRZEROdata[ledfarbe]              = 0.0;
    }
  }
  probenzeilenIndex = 0;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void DIY_SpectroOctiSetup(int pin, int led_farben) {
  // Globale Voreinstellungen
  led_farben_max = led_farben;       // Anzahl der LED Farben :=1 für eine einfarbige LED
  ledGPIOPin     = pin;
  ulReqcount = 0;                    // Seitenaufrufszähler auf 0 setzten
  if (pin<0) {  
    pixels.setPixelColor(NeoNo,0,0,0,0); // aus
    pixels.show();                 // und anzeigen
  }
  Serial.print("Lampentest ");
  if (led_farben_max > 1) {
    Serial.println(" mehrfarbig");
      pixels.setPixelColor(NeoNo,255,0,0); // R
      pixels.show();                 // und anzeigen
      delay(1000);
      pixels.setPixelColor(NeoNo,0,255,0); // G
      pixels.show();                 // und anzeigen
      delay(1000);
      pixels.setPixelColor(NeoNo,0,0,255); // B
      pixels.show();                 // und anzeigen
      delay(1000);
      pixels.setPixelColor(NeoNo,0,0,0,0); // alle aus
      pixels.show();                 // und anzeigen
 } else {
    Serial.println(" einfarbig");
	 if (ledGPIOPin < 0) {
      pixels.setPixelColor(NeoNo,0,0,0,255); // alle an
      pixels.show();                 // und anzeigen
      delay(1000);
      pixels.setPixelColor(NeoNo,0,0,0,0); // alle aus
      pixels.show();                 // und anzeigen
	 } else {
	  pinMode(ledGPIOPin,OUTPUT);
	  digitalWrite(ledGPIOPin,HIGH);
	  delay(1000);
	  digitalWrite(ledGPIOPin,LOW);
     }
 }
  cleardata();
  Serial.println("");
  Serial.println(String(sVersion + String (led_farben_max) + sVersion2));
  
  // server.begin(); // Server starten
    
  Serial.println("");
  // Lichtsensor TSL2561
//  Wire.pins(sdaPin, sclPin);  // Wire.pins(int sda, int scl) // Anschlusspins für das I2C-Protokoll zuordnen
//  if (!tsl.begin())  {        // Sensor initialisieren und im Fehlerfall eine Meldung ausgeben
//    Serial.print("Ooops, es konnte kein TSL2561-Sensor erkannt werden ...! (Hardware Fehler) Programm Abbruch!!! Reset nötig!!!");
    //while (1) {yield();};
  //}
  configureSensor();          // Verstärkung und Integrationszeit konfigurieren
  displaySensorDetails();     // Zeigt einige Basisinformationen über den Sensor an
  Serial.println("");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void DIY_SpectroOctiLoop() {  
  if (!server_init)	{
    delay(1000);
    server.begin(); // Server starten
	server_init = true;
	Serial.println("Server gestartet");
  }; 
	
  WiFiClient client = server.available(); // Prüfen ob ein WIFI-Client verbunden ist
  if (!client) return;                    // Wenn keiner verbunden ist, wieder zum Anfang. Also warten bis der WIFI-Client Daten gesendet hat
  Serial.println("Neuer WIFI-Client");    // Jetzt ist ein Client verbunden ...
  unsigned long ultimeout = millis() + 250;
  while (!client.available() && (millis() < ultimeout)) delay(1);
  if (millis() > ultimeout) {
    Serial.println("WIFI-Client Fehler: Connection-Time-Out!");
    return;
  }
  String sRequest = client.readStringUntil('\r');  // Die Erste Zeile der Anfrage lesen
  client.flush();
  if (sRequest == "") {  // WIFI-Client stoppen, wenn die Anfrage leer ist
    Serial.println("WIFI-Client Fehler: Leere Anfrage! -> WIFI-Client angehalten");
    client.stop();
    return;
  } else Serial.println(sRequest);
  
  // "get path"; Das Ende des Pfads ist entweder ein " " oder ein "?"  (Syntax z.B. GET /?pin=SENSOR_A HTTP/1.1)
  String sPath = "", sParam = "", sCmd = "";
  String sGetstart = "GET ";
  int iStart, iEndSpace, iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart >= 0) {
    iStart += +sGetstart.length();
    iEndSpace = sRequest.indexOf(" ", iStart);
    iEndQuest = sRequest.indexOf("?", iStart);
    // Den Pfad und die Parameter isolieren
    if (iEndSpace > 0) {
      if (iEndQuest > 0) { // Pfad und Parameter
        sPath  = sRequest.substring(iStart, iEndQuest);
        sParam = sRequest.substring(iEndQuest, iEndSpace);
      }
      else { // Pfad aber keine Parameter
        sPath  = sRequest.substring(iStart, iEndSpace);
      }
    }
  }
  // Den Befehl isolieren
  if (sParam.length() > 0) {
    int iEqu = sParam.indexOf("=");
    if (iEqu >= 0) {
      sCmd = sParam.substring(iEqu + 1, sParam.length());
      Serial.println(sCmd);
    }
  }
  Serial.println("htm erzeugen");
  // Die HTML Seite erzeugen
  String sResponse, sHeader;
  String sResponseStart = "";
  String sResponseTab   = "";
  if (sPath != "/") { // Für unpassenden Pfad eine 404-Fehlerseite generieren
    sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>Die angeforderte Webseite (URL) gibt es auf diesem Server nicht.</p></body></html>";
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n Content-Type: text/html\r\n Connection: close\r\n\r\n";
  } else {  // Für passenden Pfad ...
    if (sCmd.length() > 0) {  // Auf die Parameter reagieren ...
      Serial.print("Command: "); Serial.println(sCmd);
      if (sCmd.indexOf("READZERO") >= 0) {
        for (int color = 0; color < led_farben_max; color++) {
          if (!readSensor(color)) Serial.println("Sensor Fehler"); // Messung durchführen und Sensordaten auslesen - ggf. Fehler melden
          else {
            LUXZEROdata[color]    = LUXdata[color];
            VIS_IRZEROdata[color] = VIS_IRdata[color];
            IRZEROdata[color]     = IRdata[color];
          }
        }
      }
      if (sCmd.indexOf("READTSL") >= 0)  { // Wenn Button "Probe"      
        for (int color = 0; color < led_farben_max; color++) {
          if (!readSensor(color)) Serial.println("Sensor Fehler"); // Messung durchführen und Sensordaten auslesen - ggf. Fehler melden
          LUX_werte[color][probenzeilenIndex] =  LUXdata[color];
          // Die Extinktionen berechnen
          if (LUXdata[color] > 0.0 & LUXZEROdata[color] > 0.0) {
            E_LUX[color] = -log10((LUXdata[color] * 1.0) / (LUXZEROdata[color] * 1.0));
          }
          else {
            E_LUX[color] = 0.0;
          }
          if (VIS_IRdata[color] > 0.0 & VIS_IRZEROdata[color] > 0.0) {
            E_VIS_IR[color] = -log10((VIS_IRdata[color] * 1.0) / (VIS_IRZEROdata[color] * 1.0));
          }
          else {
            E_VIS_IR[color] = 0.0;
          }
          if (IRdata[color] > 0.0 & IRZEROdata[color] > 0.0) {
            E_IR[color] = -log10((IRdata[color] * 1.0) / (IRZEROdata[color] * 1.0));
          }
          else {
            E_IR[color] = 0.0;
          }
        }
        probenzeilenIndex += 1;
        if (probenzeilenIndex >= probenzeilenMax) probenzeilenIndex = 0;
      }

      if (sCmd.indexOf("CLEARDATA") >= 0) {
        cleardata()    ; // Wenn Button "Reset" gedrückt, ...
      }
      if (led_farben_max > 1) {
        if (sCmd.indexOf("GR")       >= 0) {
          anzeige = "g"   ; // Wenn Button "grün" gedrückt, ...
        }
        if (sCmd.indexOf("RT")       >= 0) {
          anzeige = "r"   ; // Wenn Button "rot" gedrückt, ...
        }
        if (sCmd.indexOf("BL")       >= 0) {
          anzeige = "b"   ; // Wenn Button "blau" gedrückt, ...
        }
        if (sCmd.indexOf("ALL")      >= 0) {
          anzeige = "a"   ; // Wenn Button "alle" gedrückt, ...
        }
      }
      if (sCmd.indexOf("DOWNLOAD") >= 0) {
        download = true; // Wenn Button "Download" gedrückt, ...
      }
    }
    // Die Extinktion mit ggf neuer Leerprobe erneut berechnen
    for (int color = 0; color < led_farben_max; color++) {
      for (int zeilennummer = 0; zeilennummer < probenzeilenMax; zeilennummer++) {
        if (LUX_werte[color][zeilennummer] > 0.0 & LUXZEROdata[color] > 0.0) {
          E_werte[color][zeilennummer] = -log10((LUX_werte[color][zeilennummer] * 1.0) / (LUXZEROdata[color] * 1.0));
        }
        else {
          E_werte[color][zeilennummer] = 0.0;
        }
      }
    }
    ulReqcount++;  // Seitenaufrufzähler um 1 raufzählen
   // String logo = "<CENTER> <img src='data:image/png;base64,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' alt=''> </CENTER>";
   // sResponseStart  = "<html><head><title>DIY Photometer</title>"+logo+"</head><body>";
    sResponseStart  = "<html><head><title>DIY Photometer</title></head><body>";
    sResponseStart += "<font color=\"#FFFFFF\"><body bgcolor=\"#003CA0\">";  // Hintergrundfarbe
    sResponseStart += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponseStart += "<p style='text-align: center'><a href=\"?pin=READZERO\"><button style='font-size:26px'>Leerprobe</button></a>";
    sResponseStart += String("&nbsp &nbsp &nbsp &nbsp <a href=\"?pin=READTSL\"> <button style='font-size:26px'>Probe &nbsp" + String(probenzeilenIndex + 1) + "</button></a></p>");
    sResponseStart += "<p style='text-align: center'><table border='0' style=\"font-family: 'Helvetica', sans-serif; color: white; margin-left: auto; margin-right: auto; font-size: 20 \">";
    for (int color = 0; color < led_farben_max; color++) {
      if (led_farben_max > 1) {
        sResponseStart += String("<tr><td>" + farbkennung[color] + "</td><td>:</td><td>" + String(LUXdata[color]) + " lx &nbsp</td><td>" + String(E_LUX[color], 4) + "</td></tr>");
      }
      else {
        sResponseStart += String("<tr><td>" + String(LUXdata[color]) + " lx &nbsp</td><td>" + String(E_LUX[color], 4) + "</td></tr>");
      }
    }
    sResponseStart += "</table></p>";
    sResponseStart += "<p style='text-align: center'>";
//    sResponseStart += "Probentabelle anzeigen f&uumlr:<BR>";
    if (led_farben_max > 1) {
      sResponseStart += "Probentabelle anzeigen f&uumlr:<BR>";
      sResponseStart += String("<a href=\"?pin=GR\"><button>" + farbkennung[0] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=RT\"><button>" + farbkennung[1] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=BL\"><button>" + farbkennung[2] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=ALL\"><button>" + farbkennung[3] + "</button></a>");
    }
    sResponseStart += "</p>";
    if (led_farben_max > 1) {
      datentabelle = String(" " + trennzeichen + "(g)" + trennzeichen + "(g)" + trennzeichen + "(r)" + trennzeichen + "(r)" + trennzeichen + "(b)" + trennzeichen + "(b)\r\n");
      datentabelle += String(" "  + trennzeichen + "M" + trennzeichen + "E" + trennzeichen + "M" + trennzeichen + "E" + trennzeichen + "M" + trennzeichen + "E\r\n");
      datentabelle += String(" " + trennzeichen + "[lx]" + trennzeichen + "[-]" + trennzeichen + "[lx]" + trennzeichen + "[-]" + trennzeichen + "[lx]" + trennzeichen + "[-]\r\n");
      datentabelle += String("Leerprobe"  + trennzeichen + String(LUXZEROdata[0], 2)  + trennzeichen + "-" + trennzeichen + String(LUXZEROdata[1], 2)  + trennzeichen + "-" + trennzeichen + String(LUXZEROdata[2], 2)  + trennzeichen +  "-\r\n");
      for (int i = 0; i < probenzeilenMax; i++) {
        datentabelle += ("Probe " + String(i + 1));
        datentabelle += (trennzeichen +  (String(LUX_werte[0][i], 2)) + trennzeichen + (String(String(E_werte[0][i], 3))));
        datentabelle += (trennzeichen +  (String(LUX_werte[1][i], 2)) + trennzeichen + (String(String(E_werte[1][i], 3))));
        datentabelle += (trennzeichen +  (String(LUX_werte[2][i], 2)) + trennzeichen + (String(String(E_werte[2][i], 3))));
        datentabelle += "\r\n";
      }
    }
    else {
      datentabelle =  String(" "  + trennzeichen + "M" + trennzeichen + "E\r\n");
      datentabelle += String(" " + trennzeichen + "[lx]" + trennzeichen + "[-]\r\n");
      datentabelle += String("Leerprobe"  + trennzeichen + String(LUXZEROdata[0], 2)  + trennzeichen + "-\r\n");
      for (int i = 0; i < probenzeilenMax; i++) {
        datentabelle += ("Probe " + String(i + 1));
        datentabelle += (trennzeichen +  (String(LUX_werte[0][i], 2)) + trennzeichen + (String(String(E_werte[0][i], 3))));
        datentabelle += "\r\n";
      }
    }
    Serial.println(datentabelle);
    if (led_farben_max == 1) {
      anzeige = "g"; // Wenn nur eine einfabige LED verbaut ist, wird immer nur die reduzierte Tabelle angezeigt
    }
    sResponseTab += "<table border='1' width='100%' style=\"font-family: 'Helvetica', sans-serif; color: white;\"><tr><th><a href=\"?pin=CLEARDATA\"> <button>Reset</button></a></th>";
    if (anzeige == "a" or anzeige == "g") sResponseTab += "<th>Messwert(g) [lx]</th> <th>Extinktion(g) [-]</th>";
    if (anzeige == "a" or anzeige == "r") sResponseTab += "<th>Messwert(r) [lx]</th> <th>Extinktion(r) [-]</th>";
    if (anzeige == "a" or anzeige == "b") sResponseTab += "<th>Messwert(b) [lx]</th> <th>Extinktion(b) [-]</th>";
    sResponseTab += "</tr>";
    sResponseTab += "<tr><td>Leerprobe</td>";
    if (anzeige == "a" or anzeige == "g") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[0]) + "</td><td></td>");
    }
    if (anzeige == "a" or anzeige == "r") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[1]) + "</td><td></td>");
    }
    if (anzeige == "a" or anzeige == "b") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[2]) + "</td><td></td>");
    }
    sResponseTab += "</tr>";
    for (int i = 0; i < probenzeilenMax; i++) {
      sResponseTab += "<tr>";
      sResponseTab += String("<td>Probe " + String(i + 1) + "</td>");
      if (anzeige == "a" or anzeige == "g") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[0][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[0][i]  , 3) + "</td>");
      }
      if (anzeige == "a" or anzeige == "r") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[1][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[1][i]  , 3) + "</td>");
      }
      if (anzeige == "a" or anzeige == "b") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[2][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[2][i]  , 3) + "</td>");
      }
      sResponseTab += "</tr>";
    }
    sResponseTab += "</table>";
    sResponseTab += "<p style='text-align: center'><a href=\"?pin=DOWNLOAD\"><button>Download</button></a></p><BR>";
    sResponse += String("<FONT SIZE=-2>Seitenaufrufe: " + String(ulReqcount) + "<BR>"); // Aufrufzähler anzeigen
   // sResponse += String("<META HTTP-EQUIV=\"refresh\" CONTENT=\"15\">");
    sResponse += String(sVersion + String (led_farben_max) + sVersion2 + "<BR>");       // Versionshinweis anzeigen
    sResponse += "</body></html>";

    sHeader  = "HTTP/1.1 200 OK\r\n Content-Length: ";
    sHeader += sResponse.length() + sResponseStart.length() + sResponseTab.length();
    sHeader += "\r\n Content-Type: text/html\r\n Connection: close\r\n\r\n";
  }
  /* Die Antwort an den WIFI-Client senden */
  if (download) { // Wenn der Downloadbutton gedrückt wurde, soll eine Datei gestreamt werden ...
    download = false;
    sResponse  = "HTTP/1.1 200 OK\r\n";
    sResponse += "Content-Type: text/csv; charset=utf-8 \r\n";
    sResponse += "Content-Transfer-Encoding: binary \r\n";
    sResponse += "Content-Disposition: attachment; filename=\"photometer_daten.csv\" \r\n";
    sResponse += "Pragma: no-cache Expires: 0 \r\n";
    sResponse += "Content-Length:";
    sResponse += String(datentabelle.length());
    sResponse += " \r\n";
    sResponse += " Connection: close";
    sResponse += "\r\n\r\n";
    sResponse += datentabelle;
    client.print(sResponse);   // Die Antwort an den WIFI-Client senden
    delay(1000);               // 1000 ms warten
  } else {  // Die HTML Seite ausgeben
    /*
    Serial.println("ausgeben");
    Serial.print(sHeader);
    Serial.print(sResponseStart);
    Serial.print(sResponseTab);
    Serial.print(sResponse);
    */
    client.print(sHeader);
    client.print(sResponseStart);
    client.print(sResponseTab);
    client.print(sResponse);
  }
  // und den WIFI-Client stoppen
  delay(200);
  client.stop();
  Serial.println("WIFI-Client getrennt");
}