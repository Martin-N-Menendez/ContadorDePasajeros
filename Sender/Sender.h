#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoOTA.h>
#include <String.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

//[3|123|1|10
//<1|5|AA:BB:CC:DD:EE:FF|-40|50|60

// ####################### Buffers ########################
#define BUFFER_SIZE 256
#define BUFFER_IN 128
#define BAUDRATE 115200
// ####################### Interrupciones ########################
#define timer0_preload 40161290/500
#define my_delay 100
// ####################### Miscelaneos ########################
#define MAC_SIZE 18
#define LED 2
#define DEBUG_MODE 0
// ####################### Configuracion de redes ########################
#define WIFI_CHOSEN 5  // 1: Martin , 2: Mi celular, 3: Trenes Argentinos, 4: Pablo, 5: Pablo Saavedra
#define SV_CHOSEN 0 // 0: Principal , 1: Secundario

// ####################### Servidor PRINCIPAL ########################
#if(!SV_CHOSEN)
#define FIREBASE_HOST "contador-de-pasajeros.firebaseio.com"
#define FIREBASE_AUTH "EwnPei16BE1rHuQJ4hXBULN47hOLkuLrQQITOWRu"
#endif

// ####################### Servidor SECUNDARIO ########################
#if(SV_CHOSEN)
#define FIREBASE_HOST "gicsafe-cdp.firebaseio.com"
#define FIREBASE_AUTH "JzjC8HuWCp2xvqE37n71488Gv9XuWQkRTsFsnnl5"
#endif

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
#define WIFI_SSID "WiFi-Arnet-425o"
#define WIFI_PASSWORD "7kql4vx3Yv7PK"
#endif

// ################ Fecha y hora  ############
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// ################ Prototipos  ############
void ConfigTime();
void SendParsedData(void);
void recvWithStartEndMarkers();
void parseData();
void showParsedData();
void SetAddress(void);
void SetInfo();
void SendParsedData(void);
void ReadConfig();
