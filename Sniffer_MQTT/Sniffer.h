#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <string.h>

#define WIFI_CHOSEN 1  // 1: Martin , 2: Mi celular, 3: Trenes Argentinos, 4: Pablo, 5: Ari 6:PabloCel 7:PabloCel2

// ####################### Redes a elegir ########################
#if(WIFI_CHOSEN==1)
#define WIFI_SSID "Telecentro-cb70"
#define WIFI_PASSWORD "VWN52Y4ETZ2Q"
#endif

#if(WIFI_CHOSEN==2)
#define WIFI_SSID "PruebaTBA"
#define WIFI_PASSWORD "pruebaTBA"
#endif

#if(WIFI_CHOSEN==3)
#define WIFI_SSID "BCYL-Invitados" 
#define WIFI_PASSWORD "bcyl2016"
#endif

#if(WIFI_CHOSEN==4)
#define WIFI_SSID "WiFi-Arnet-3b37"
#define WIFI_PASSWORD "33B3FF74EE"
#endif

#if(WIFI_CHOSEN==5)
#define WIFI_SSID "alutenberg"
#define WIFI_PASSWORD "contador2018"
#endif

#if(WIFI_CHOSEN==6)
#define WIFI_SSID "pgomez"
#define WIFI_PASSWORD "contador2018"
#endif

#if(WIFI_CHOSEN==7)
#define WIFI_SSID "WT19a"
#define WIFI_PASSWORD "contador2018"
#endif



#define BAUD_RATE 115200
#define DISABLE 0
#define ENABLE  1

#define LED 2
#define TXPOWER -20.5

#define CLIENT_DEBUG 0
#define BEACON_DEBUG 0
#define PROBE_DEBUG 0
#define DEBUG_MODE 1
#define SENDER_MODE 1
#define RANDOM_MODE 0

#define BUFFER_SIZE 256*2
#define BUFFER_IN 256

