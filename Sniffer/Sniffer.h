#include <ESP8266WiFi.h>

#include <string.h>
#include "Stations.h"
#include "functions.h"

#define LINE LINEA3
#define TRAIN 123
#define CAR 5

#define WIFI_SSID "Telecentro-cb70"
#define WIFI_PASSWORD "VWN52Y4ETZ2Q"

#define BAUD_RATE 115200
#define DISABLE 0
#define ENABLE  1

#define LED 2
#define TXPOWER -20.5

#define CLIENT_DEBUG 0
#define BEACON_DEBUG 0
#define PROBE_DEBUG 0
#define DEBUG_MODE 0
#define SENDER_MODE 1
#define RANDOM_MODE 0

#define BUFFER_SIZE 256*2
#define BUFFER_IN 256
