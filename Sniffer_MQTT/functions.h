// This-->tab == "functions.h"

// Expose Espressif SDK functionality
extern "C" {
#include "user_interface.h"
  typedef void (*freedom_outside_cb_t)(uint8 status);
  int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  void wifi_unregister_send_pkt_freedom_cb(void);
  int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}

extern int RSSILimit;

#include <ESP8266WiFi.h>
#include "./structures.h"

#define MAX_APS_TRACKED 0
#define MAX_CLIENTS_TRACKED 0
#define MAX_PROBES_TRACKED 300

#define CLIENT_DEBUG 0
#define BEACON_DEBUG 0
#define PROBE_DEBUG 0

clientinfo clients_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
beaconinfo aps_known[MAX_APS_TRACKED];                    // Array to save MACs of known APs
probeinfo probes_known[MAX_PROBES_TRACKED];                // Array to save MACs of known PROBEs

volatile uint8_t nothing_new = 0;

uint16_t clients_known_count = 0;                              // Number of known CLIENTs
uint16_t aps_known_count = 0;                                  // Number of known APs
volatile uint16_t probes_known_count = 0;                              // Number of known  PROBEs

String formatMac1(uint8_t mac[ETH_MAC_LEN]) {
  String hi = "";
  for (int i = 0; i < ETH_MAC_LEN; i++) {
    if (mac[i] < 16) hi = hi + "0" + String(mac[i], HEX);
    else hi = hi + String(mac[i], HEX);
    if (i < 5) hi = hi + ":";
  }
  return hi;
}

int register_client(clientinfo ci) {
  int known = 0;   // Clear known flag
  for (int u = 0; u < clients_known_count; u++)
  {
    if (! memcmp(clients_known[u].station, ci.station, ETH_MAC_LEN)) {
      clients_known[u].lastDiscoveredTime = ci.lastDiscoveredTime;
      clients_known[u].rssi = ci.rssi;
      known = 1;
      break;
    }
  }

  //Uncomment the line below to disable collection of probe requests from randomised MAC's
  if (ci.channel == -2) known = 1; // This will disable collection of probe requests from randomised MAC's

  if (! known) {
    ci.lastDiscoveredTime = millis()/1000;
    // search for Assigned AP
    for (int u = 0; u < aps_known_count; u++) {
      if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
        ci.channel = aps_known[u].channel;
        break;
      }
    }
    if (ci.channel != 0) {
      memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
      /*   Serial.println();
         Serial.print("Register Client ");
         Serial.print(formatMac1(ci.station));
         Serial.print(" Channel ");
         Serial.print(ci.channel);
         Serial.print(" RSSI ");
         Serial.println(ci.rssi);*/
      clients_known_count++;
    }

    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) ) {
      //Serial.printf("exceeded max clients_known\n");
      clients_known_count--;
    }
  }
  return known;
}

int register_beacon(beaconinfo beacon) {
  int known = 0;   // Clear known flag
  for (int u = 0; u < aps_known_count; u++)
  {
    if (! memcmp(aps_known[u].bssid, beacon.bssid, ETH_MAC_LEN)) {
      aps_known[u].lastDiscoveredTime = beacon.lastDiscoveredTime;
      aps_known[u].rssi = beacon.rssi;
      known = 1;
      break;
    }   // AP known => Set known flag
  }
  if (! known && (beacon.err == 0))  // AP is NEW, copy MAC to array and return it
  {
    beacon.lastDiscoveredTime = millis()/1000;
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
    /*    Serial.print("Register Beacon ");
        Serial.print(formatMac1(beacon.bssid));
        Serial.print(" Channel ");
        Serial.print(aps_known[aps_known_count].channel);
        Serial.print(" RSSI ");
        Serial.println(aps_known[aps_known_count].rssi);*/

    aps_known_count++;

    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) ) {
      //Serial.printf("exceeded max aps_known\n");
      aps_known_count--;
    }
  }
  return known;
}

int register_probe(probeinfo pi) {
  //add probe to list if not already included.
  int known = 0;   // Clear known flag
  for (int u = 0; u < probes_known_count; u++)
  {
    if ((memcmp(probes_known[u].station, pi.station, ETH_MAC_LEN) == 0)) {
      
      if(pi.rssi > RSSILimit)
      {
      known = 1;
      probes_known[u].lastDiscoveredTime = pi.lastDiscoveredTime;
      probes_known[u].rssi = pi.rssi;
      probes_known[u].reported++;
      break;
      }
    }
  }

    if (!known)
    {
      if(pi.rssi > RSSILimit)
      {
      memcpy(&probes_known[probes_known_count], &pi, sizeof(pi));
      probes_known[probes_known_count].reported=0;
      probes_known_count++; 
      if ((unsigned int) probes_known_count >= sizeof (probes_known) / sizeof (probes_known[0]) ) {
        //Serial.printf("exceeded max probes_known\n");
        probes_known_count--;
      }
     }
  }
  return known;
}

String print_client(clientinfo ci) {
  String hi = "";
  int u = 0;
  int known = 0;   // Clear known flag
  if (ci.err != 0) {
    // nothing
  } else {
    #if(CLIENT_DEBUG)
    Serial.printf("CLIENT: ");
    Serial.print(formatMac1(ci.station));  //Mac of device
    Serial.printf(" ==> ");

    //    for (u = 0; u < aps_known_count; u++)
    //    {
    //      if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
    //        //       Serial.print("   ");
    //        //        Serial.printf("[%32s]", aps_known[u].ssid);   // Name of connected AP
    //        known = 1;     // AP known => Set known flag
    //        break;
    //      }
    //    }
    //
    //    if (! known)  {
    //      Serial.printf("   Unknown/Malformed packet \r\n");
    //      for (int i = 0; i < 6; i++) Serial.printf("%02x", ci.bssid[i]);
    //    } else {
    //      //    Serial.printf("%2s", " ");

    Serial.print(formatMac1(ci.ap));   // Mac of connected AP
    Serial.printf("  % 3d", ci.channel);  //used channel
    Serial.printf("   % 4d\r\n", ci.rssi);
    #endif
    //    }
  }
  return hi;
}

String print_beacon(beaconinfo beacon) {
  String hi = "";
  if (beacon.err != 0) {
    //Serial.printf("BEACON ERR: (%d)  ", beacon.err);
  } else {
    #if(BEACON_DEBUG)
    Serial.printf("BEACON: <============ [%32s]  ", beacon.ssid);
    Serial.print(formatMac1(beacon.bssid));
    Serial.printf("   %2d", beacon.channel);
    Serial.printf("   %4d\r\n", beacon.rssi);
    #endif
  }
  return hi;
}

String print_probe(probeinfo pi) {
  uint32_t now=millis()/1000;
  String hi = "";
  int u = 0;
  int known = 0;   // Clear known flag
  if (pi.err != 0) {
    // nothing
  } else {
    #if(PROBE_DEBUG)
    Serial.printf("PROBE: ");
    Serial.print(formatMac1(pi.station));  //Mac of device
    Serial.printf(" ==> ");

    Serial.print(formatMac1(pi.ap));   // Mac of connected AP
    Serial.printf("  % 3d", pi.lastDiscoveredTime);  
    Serial.printf("   % 4d\r\n", pi.rssi);
    #endif
    //    }
  }
  return hi;
};

void promisc_cb(uint8_t *buf, uint16_t len) {
  int i = 0;
  uint16_t seq_n_new = 0;
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
  } else if (len == 128) {
    uint8_t frame_control_pkt = buf[12]; // just after the RxControl Structure
    uint8_t frame_type = (frame_control_pkt & 0x0C) >> 2;
    uint8_t frame_subtype = (frame_control_pkt & 0xF0) >> 4;
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    if (frame_type == 0 && (frame_subtype == 8 || frame_subtype == 5)) {
      struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
      /*
      if (register_beacon(beacon) == 0) {
        print_beacon(beacon);
        nothing_new = 0;
      }
      */
    } else if (frame_type ==0 && frame_subtype==4) {
      struct probeinfo probe = parse_probe(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
      //if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
      if (register_probe(probe) == 0) {
        print_probe(probe);
        nothing_new = 0;
      }
      //}
    }
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    //if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
    struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
    //if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
    /*
    if (register_client(ci) == 0) {
      print_client(ci);
      nothing_new = 0;
    }
    */
    //}
    //}
  }
}






