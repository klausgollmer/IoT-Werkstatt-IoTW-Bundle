/*
  Copyright 2017 Andreas Spiess

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This software is based on the work of Andreas Spiess, https://github.com/SensorsIot/Wi-Fi-Sniffer-as-a-Human-detector/blob/master/WiFi_Sniffer/WiFi_Sniffer.ino
  and Ray Burnette: https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a
  
*/

//#define ETH_MAC_LEN 6
#include <IoTSniffer.h>



#define MAX_APS_TRACKED 100
#define MAX_CLIENTS_TRACKED 200

beaconinfo aps_known[MAX_APS_TRACKED];                    // Array to save MACs of known APs
int aps_known_count = 0;                                  // Number of known APs
int nothing_new = 0;
clientinfo clients_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int clients_known_count = 0;                              // Number of known CLIENTs
int Sniffer_erstemal = 1;

uint8_t broadcast1[3] = { 0x01, 0x00, 0x5e };
uint8_t broadcast2[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t broadcast3[3] = { 0x33, 0x33, 0x00 };



// Gemeinsame SHA1-Funktion
String mysha1(const String& input) {
    uint8_t hash[20]; // SHA1-Ergebnis ist 20 Bytes groß

#if defined(ESP8266)
    // ESP8266-spezifische Funktion
    ::sha1(input.c_str(), input.length(), hash);
#elif defined(ESP32)
    // ESP32 verwendet mbedTLS
    mbedtls_sha1(reinterpret_cast<const uint8_t*>(input.c_str()), input.length(), hash);
#endif

    // Konvertiere das Ergebnis in einen hexadezimalen String
    String result;
    for (int i = 0; i < 20; i++) {
        if (hash[i] < 16) result += "0"; // Füge führende Null hinzu
        result += String(hash[i], HEX);
    }
    return result;
}

String formatMac1(uint8_t mac[ETH_MAC_LEN]) {
  String hi = "";
  for (int i = 0; i < ETH_MAC_LEN; i++) {
    if (mac[i] < 16) hi = hi + "0" + String(mac[i], HEX);
    else hi = hi + String(mac[i], HEX);
    if (i < 5) hi = hi + ":";
  }
  return hi;
}

int register_beacon(beaconinfo beacon)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < aps_known_count; u++)
  {
    if (! memcmp(aps_known[u].bssid, beacon.bssid, ETH_MAC_LEN)) {
      aps_known[u].lastDiscoveredTime = millis();
      aps_known[u].rssi = beacon.rssi;
      known = 1;
      break;
    }   // AP known => Set known flag
  }
  if (! known && (beacon.err == 0))  // AP is NEW, copy MAC to array and return it
  {
    beacon.lastDiscoveredTime = millis();
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
	/*
        if (IOTW_debug_level) Serial.print("Register Beacon ");
        if (IOTW_debug_level) Serial.print(formatMac1(beacon.bssid));
        if (IOTW_debug_level) Serial.print(" Channel ");
        if (IOTW_debug_level) Serial.print(aps_known[aps_known_count].channel);
        if (IOTW_debug_level) Serial.print(" RSSI ");
        if (IOTW_debug_level) Serial.println(aps_known[aps_known_count].rssi);
    */
    aps_known_count++;

    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) ) {
      if (IOTW_debug_level) Serial.printf("exceeded max aps_known\n");
      aps_known_count = 0;
    }
  }
  return known;
}

int register_client(clientinfo &ci,int isdata) {
  int known = 0;   // Clear known flag
  
  /*
  if (isdata) if (IOTW_debug_level) Serial.print("\nD "); else if (IOTW_debug_level) Serial.print("\nP");
  if (IOTW_debug_level) Serial.print(" St: ");
  for (int i = 0; i < 3; i++) if (IOTW_debug_level) Serial.printf("%02x", ci.station[i]);
  if (IOTW_debug_level) Serial.print(" bssid: ");
  for (int i = 0; i < 3; i++) if (IOTW_debug_level) Serial.printf("%02x", ci.bssid[i]);
  if (IOTW_debug_level) Serial.print(" ch:");
  if (IOTW_debug_level) Serial.print(ci.channel);
  */
 
  for (int u = 0; u < clients_known_count; u++)  {
    if (! memcmp(clients_known[u].station, ci.station, ETH_MAC_LEN)) { // Knoten ist bereits bekannt
	  if (isdata) {                                                    // und sendet Daten
    	  clients_known[u].lastDiscoveredTime = millis();                    
		  for (int ua = 0; ua < aps_known_count; ua++) {               // Wir schauen, auf welchem Kanal
            if (! memcmp(aps_known[ua].bssid, ci.bssid, ETH_MAC_LEN)) {
                clients_known[u].channel = aps_known[ua].channel;
		        //if (IOTW_debug_level) Serial.printf(" alt%8s", aps_known[ua].ssid); 
                break;
            }
         }
	  }
	  if ((!isdata) && (clients_known[u].channel < 0)) clients_known[u].lastDiscoveredTime = millis(); // Bekannt, schon bisher ohne Verbindung 
      clients_known[u].rssi = ci.rssi;
      known = 1;
      break;
    } // unbekannt
  }

  //Uncomment the line below to disable collection of probe requests from randomised MAC's
  //if (ci.channel == -2) known = 1; // This will disable collection of probe requests from randomised MAC's
  
  if (! known) {
    ci.lastDiscoveredTime = millis();
    // search for Assigned AP
	if (isdata) { // Knoten sendet Daten, also suchen wir den dazugehörigen AP
      for (int u = 0; u < aps_known_count; u++) {
       if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
        ci.channel = aps_known[u].channel;
        //if (IOTW_debug_level) Serial.printf("neu%8s", aps_known[u].ssid);
        break;
       }
      }
	}
    if (ci.channel != 0) {
      memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
	  /*
         if (IOTW_debug_level) Serial.println();
         if (IOTW_debug_level) Serial.print("Register Client ");
         if (IOTW_debug_level) Serial.print(formatMac1(ci.station));
         if (IOTW_debug_level) Serial.print(" Channel ");
         if (IOTW_debug_level) Serial.print(ci.channel);
         if (IOTW_debug_level) Serial.print(" RSSI ");
         if (IOTW_debug_level) Serial.println(ci.rssi);
		*/ 
      clients_known_count++;
    }

    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) ) {
      if (IOTW_debug_level) Serial.printf("exceeded max clients_known\n");
      clients_known_count = 0;
    }
  }
  return known;
}


String print_beacon(beaconinfo beacon)
{
  String hi = "";
  if (beacon.err != 0) {
    //if (IOTW_debug_level) Serial.printf("BEACON ERR: (%d)  ", beacon.err);
  } else {
    if (IOTW_debug_level) Serial.printf(" BEACON: <=============== [%32s]  ", beacon.ssid);
    if (IOTW_debug_level) Serial.print(formatMac1(beacon.bssid));
    if (IOTW_debug_level) Serial.printf("   %2d", beacon.channel);
    if (IOTW_debug_level) Serial.printf("   %4d\r\n", beacon.rssi);
  }
  return hi;
}

String print_client(clientinfo ci)
{
  String hi = "";
  int u = 0;
  int known = 0;   // Clear known flag
  if (ci.err != 0) {
    // nothing
  } else {
    if (IOTW_debug_level) Serial.printf("CLIENT: ");
    if (IOTW_debug_level) Serial.print(formatMac1(ci.station));  //Mac of device
    if (IOTW_debug_level) Serial.printf(" ==> ");

    for (u = 0; u < aps_known_count; u++)
    {
      if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
               if (IOTW_debug_level) Serial.print("   ");
               if (IOTW_debug_level) Serial.printf("[%32s]", aps_known[u].ssid);   // Name of connected AP
        known = 1;     // AP known => Set known flag
        break;
      }
    }

	/*
    if (! known)  {
      if (IOTW_debug_level) Serial.printf("   Unknown/Malformed packet \r\n");
      for (int i = 0; i < 6; i++) if (IOTW_debug_level) Serial.printf("%02x", ci.bssid[i]);
    } else {
      //    if (IOTW_debug_level) Serial.printf("%2s", " ");
      */
      if (IOTW_debug_level) Serial.print(formatMac1(ci.ap));   // Mac of connected AP
      if (IOTW_debug_level) Serial.printf("  % 3d", ci.channel);  //used channel
      if (IOTW_debug_level) Serial.printf("   % 4d\r\n", ci.rssi);
    //}
  }
  return hi;
}

#if defined(ESP8266)
void promisc_cb(uint8_t *buf, uint16_t len)
{
  int i = 0;
  uint16_t seq_n_new = 0;
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    if ((sniffer->buf[0] == 0x80)) {
      struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
      if (register_beacon(beacon) == 0) {
        //print_beacon(beacon);
        nothing_new = 0;
      }
    } else if ((sniffer->buf[0] == 0x40)) {
      struct clientinfo ci = parse_probe(sniffer->buf, 36, sniffer->rx_ctrl.rssi);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci,0) == 0) {
      //    print_client(ci);
          nothing_new = 0;
        }
      }
    }
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci,1) == 0) {
      //    print_client(ci);
          nothing_new = 0;
        }
      }
    }
  }
}
#endif

#if defined(ESP32)
void promisc_cb(void* buf, wifi_promiscuous_pkt_type_t type)
{
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  int len = pkt->rx_ctrl.sig_len;
  
  int i = 0;
  uint16_t seq_n_new = 0;
  
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    if ((sniffer->buf[0] == 0x80)) {
      struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
      if (register_beacon(beacon) == 0) {
        //print_beacon(beacon);
        nothing_new = 0;
      }
    } else if ((sniffer->buf[0] == 0x40)) {
      struct clientinfo ci = parse_probe(sniffer->buf, 36, sniffer->rx_ctrl.rssi);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci,0) == 0) {
      //    print_client(ci);
          nothing_new = 0;
        }
      }
    }
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci,1) == 0) {
      //    print_client(ci);
          nothing_new = 0;
        }
      }
    }
  }
}
#endif





void SnifferPurgeDevice(int timeout) {
  long lastseen; 	
  for (int u = 0; u < clients_known_count; u++) {
	// if (IOTW_debug_level) Serial.println(((millis() - clients_known[u].lastDiscoveredTime)/1000));
//    if (((millis() - clients_known[u].lastDiscoveredTime)/1000) > timeout) {
	lastseen = ((millis() - clients_known[u].lastDiscoveredTime)/1000);
	
    if ((lastseen > timeout) || ((clients_known[u].channel==-2) && (lastseen>1))) {  
      //if (IOTW_debug_level) Serial.print("purge Client" );
      //if (IOTW_debug_level) Serial.print(u);
      for (int i = u; i < clients_known_count; i++) memcpy(&clients_known[i], &clients_known[i + 1], sizeof(clients_known[i]));
      clients_known_count--;
      //break;
    }
  }
  for (int u = 0; u < aps_known_count; u++) {
	lastseen = ((millis() - aps_known[u].lastDiscoveredTime)/1000);
    if (lastseen > timeout) {
      //if (IOTW_debug_level) Serial.print("purge Bacon" );
      //if (IOTW_debug_level) Serial.println(u);
      for (int i = u; i < aps_known_count; i++) memcpy(&aps_known[i], &aps_known[i + 1], sizeof(aps_known[i]));
      aps_known_count--;
      break;
    }
  }
  
  
  
}

int SnifferCountDevices(int MinRSSI,int timeout, String myMAC, int randMAC, int mydisplay)
 {
  int ddebug = 1; 
  int mycount = 0;
  if (mydisplay == 0) ddebug = 0;  
  if (Sniffer_erstemal) {
    // if (IOTW_debug_level) Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
    if (IOTW_debug_level) Serial.println(F("Human detector by Andreas Spiess. ESP8266 mini-sniff by Ray Burnette"));
    //if (IOTW_debug_level) Serial.println(F("Human detector by Andreas Spiess. ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
	Sniffer_erstemal = 0;
  }  
  SnifferPurgeDevice(timeout); // entferne stumme Teilnehmer ( die den Raum verlassen haben)
  for (int u = 0; u < clients_known_count; u++) {
    if (clients_known[u].rssi >= MinRSSI) { 
      if (myMAC.length() >= 17) { // MAC Adresse vorgegeben 
	    if (IOTW_debug_level) Serial.println("sorry, no MAC identification available in school version");
        if (myMAC == formatMac1(clients_known[u].station)) mycount = 1;
      } 
      else { // sonst alles sniffen
	    if (clients_known[u].channel >= 0) { // special randomised MAC
           mycount++; // echte immer zaehlen
		} else {
			if (randMAC <= clients_known[u].channel) {
               mycount++; // auch randomized zaehlen
			}
		}
      }
    }
  } 
  
  if (ddebug >= 1) {
    Serial.println("");
    Serial.print("Detected WiFi-CLIENTS: ");
    Serial.println(mycount);
    long lastseen;
	// show Clients
   for (int u = 0; u < clients_known_count; u++) {
     Serial.printf("%4d ",u); // Show client number
	 String isMAC;
	 isMAC = formatMac1(clients_known[u].station);
	 
	 if (ddebug > 1) {
         Serial.print(formatMac1(clients_known[u].station));
	     Serial.print(" ap: ");
		 Serial.print(formatMac1(clients_known[u].ap));
	 } else {
	     Serial.print(isMAC.substring(0,8)+String(":xx:xx:xx"));
	 }	 

     Serial.print(" RSSI ");
 	 Serial.print(clients_known[u].rssi);
	 if (clients_known[u].rssi < MinRSSI) Serial.print(" (to weak) "); else if (IOTW_debug_level) Serial.print("           ");
     Serial.print(" channel ");
     Serial.print(clients_known[u].channel);
	 lastseen = ((millis() - clients_known[u].lastDiscoveredTime)/1000);
     Serial.print(" Tout ");
	 Serial.println(lastseen);
   }
   if (ddebug>2) {
		 // show Beacons
		  Serial.println("----------- BEACONS"); 
		  for (int u = 0; u < aps_known_count; u++) {
			Serial.printf( "%4d ",u); // Show beacon number
			Serial.print(formatMac1(aps_known[u].bssid));
			Serial.print(" RSSI ");
			Serial.print(aps_known[u].rssi);
			Serial.print(" channel ");
			Serial.print(aps_known[u].channel);
			Serial.printf(" %-15s \n",aps_known[u].ssid);
		  }
    }
  }
  return mycount;
}


void hashMAC(uint8_t mac[]) {
	
    String hi = ""; 
    for (int i = 0; i < ETH_MAC_LEN; i++) {
      if (mac[i] < 16) hi = hi + "0" + String(mac[i], HEX);
      else hi = hi + String(mac[i], HEX);
      if (i < 5) hi = hi + ":";
    }
	//if (IOTW_debug_level) Serial.println(hi);
	String hash = mysha1(hi);
	//if (IOTW_debug_level) Serial.println(hash);
	
	String hashbyte;
    char hex[] = "000";
	
	for (int i = 0; i < 3; i++) {
   	    hashbyte=hash.substring(i*2,i*2+2);
	    hashbyte.toCharArray(hex,3);
		mac[i+3] = strtoul(hex,NULL,16);
	}
	return;
}


struct clientinfo parse_data(uint8_t *frame, uint16_t framelen, signed rssi, unsigned channel)
{
  struct clientinfo ci;
  ci.channel = channel;
  ci.err = 0;
  ci.rssi = rssi;
  int pos = 36;
  uint8_t *bssid;
  uint8_t *station;
  uint8_t *ap;
  uint8_t ds;

  ds = frame[1] & 3;    //Set first 6 bits to 0
  switch (ds) {
    // p[1] - xxxx xx00 => NoDS   p[4]-DST p[10]-SRC p[16]-BSS
    case 0:
      bssid = frame + 16;
      station = frame + 10;
      ap = frame + 4;
      break;
    // p[1] - xxxx xx01 => ToDS   p[4]-BSS p[10]-SRC p[16]-DST
    case 1:
      bssid = frame + 4;
      station = frame + 10;
      ap = frame + 16;
      break;
    // p[1] - xxxx xx10 => FromDS p[4]-DST p[10]-BSS p[16]-SRC
    case 2:
      bssid = frame + 10;
      // hack - don't know why it works like this...
      if (memcmp(frame + 4, broadcast1, 3) || memcmp(frame + 4, broadcast2, 3) || memcmp(frame + 4, broadcast3, 3)) {
        station = frame + 16;
        ap = frame + 4;
      } else {
        station = frame + 4;
        ap = frame + 16;
      }
      break;
    // p[1] - xxxx xx11 => WDS    p[4]-RCV p[10]-TRM p[16]-DST p[26]-SRC
    case 3:
      bssid = frame + 10;
      station = frame + 4;
      ap = frame + 4;
      break;
  }

  hashMAC(station);
  hashMAC(ap);
  hashMAC(bssid);
  
  memcpy(ci.station, station, ETH_MAC_LEN);
  memcpy(ci.bssid, bssid, ETH_MAC_LEN);
  memcpy(ci.ap, ap, ETH_MAC_LEN);

  ci.seq_n = frame[23] * 0xFF + (frame[22] & 0xF0);
  
  return ci;
}

struct clientinfo parse_probe(uint8_t *frame, uint16_t framelen, signed rssi)
{
  struct clientinfo pi;
  pi.channel = -1;
  pi.err = 0;
  pi.rssi = rssi;
  struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) frame;
  memset(pi.bssid,0xFF,ETH_MAC_LEN);
  memcpy(pi.station, frame + 10, ETH_MAC_LEN);
  if ((pi.station[0] & 2) == 2) pi.channel=-2; // Randomised MAC !
  hashMAC(pi.station);
  return pi;
}

struct beaconinfo parse_beacon(uint8_t *frame, uint16_t framelen, signed rssi)
{
  struct beaconinfo bi;
  bi.ssid_len = 0;
  bi.channel = 0;
  bi.err = 0;
  bi.rssi = rssi;
  int pos = 36;

  if (frame[pos] == 0x00) {
    while (pos < framelen) {
      switch (frame[pos]) {
        case 0x00: //SSID
          bi.ssid_len = (int) frame[pos + 1];
          if (bi.ssid_len == 0) {
            memset(bi.ssid, '\x00', 33);
            break;
          }
          if (bi.ssid_len < 0) {
            bi.err = -1;
            break;
          }
          if (bi.ssid_len > 32) {
            bi.err = -2;
            break;
          }
          memset(bi.ssid, '\x00', 33);
          memcpy(bi.ssid, frame + pos + 2, bi.ssid_len);
          bi.err = 0;  // before was error??
          break;
        case 0x03: //Channel
          bi.channel = (int) frame[pos + 2];
          pos = -1;
          break;
        default:
          break;
      }
      if (pos < 0) break;
      pos += (int) frame[pos + 1] + 2;
    }
  } else {
    bi.err = -3;
  }

  bi.capa[0] = frame[34];
  bi.capa[1] = frame[35];
  memcpy(bi.bssid, frame + 10, ETH_MAC_LEN);
  hashMAC(bi.bssid);
  return bi;
}
