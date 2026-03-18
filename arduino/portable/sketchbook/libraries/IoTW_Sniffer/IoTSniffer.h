#include <Arduino.h>

#if defined(ESP8266)
#include <Hash.h>
#elif defined(ESP32)
#include <mbedtls/sha1.h>
#include <esp_wifi.h> // ESP32-spezifische Header für Wi-Fi
#endif

extern int IOTW_debug_level; // Print debug information

#define ETH_MAC_LEN 6


extern uint8_t broadcast1[3] ;
extern uint8_t broadcast2[6] ;
extern uint8_t broadcast3[3] ;

struct beaconinfo
{
  uint8_t bssid[ETH_MAC_LEN];
  uint8_t ssid[33];
  int ssid_len;
  int channel;
  int err;
  signed rssi;
  uint8_t capa[2];
  long lastDiscoveredTime;
};

struct clientinfo
{
  uint8_t bssid[ETH_MAC_LEN];
  uint8_t station[ETH_MAC_LEN];
  uint8_t ap[ETH_MAC_LEN];
  int channel;
  int err;
  signed rssi;
  uint16_t seq_n;
  long lastDiscoveredTime;
};

/* ==============================================
   Promiscous callback structures, see ESP manual
   ============================================== */
struct RxControl {
  signed rssi: 8;
  unsigned rate: 4;
  unsigned is_group: 1;
  unsigned: 1;
  unsigned sig_mode: 2;
  unsigned legacy_length: 12;
  unsigned damatch0: 1;
  unsigned damatch1: 1;
  unsigned bssidmatch0: 1;
  unsigned bssidmatch1: 1;
  unsigned MCS: 7;
  unsigned CWB: 1;
  unsigned HT_length: 16;
  unsigned Smoothing: 1;
  unsigned Not_Sounding: 1;
  unsigned: 1;
  unsigned Aggregation: 1;
  unsigned STBC: 2;
  unsigned FEC_CODING: 1;
  unsigned SGI: 1;
  unsigned rxend_state: 8;
  unsigned ampdu_cnt: 8;
  unsigned channel: 4;
  unsigned: 12;
};

struct LenSeq {
  uint16_t length;
  uint16_t seq;
  uint8_t  address3[6];
};

struct sniffer_buf {
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

struct sniffer_buf2 {
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};

#if defined(ESP8266)
extern void promisc_cb(uint8_t* buf, uint16_t len);
#elif defined(ESP32)
extern void promisc_cb(void* buf, wifi_promiscuous_pkt_type_t type);
#endif
	  
extern int SnifferCountDevices(int MinRSSI,int timeout, String myMAC, int randMAC, int mydisplay);
struct clientinfo parse_data(uint8_t *frame, uint16_t framelen, signed rssi, unsigned channel);
struct clientinfo parse_probe(uint8_t *frame, uint16_t framelen, signed rssi);
struct beaconinfo parse_beacon(uint8_t *frame, uint16_t framelen, signed rssi);