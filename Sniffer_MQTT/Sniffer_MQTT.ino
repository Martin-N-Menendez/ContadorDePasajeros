#include "Sniffer.h"
#include "functions.h"

//boolean newData = false;
extern int purge;
extern int RSSILimit;
extern int SendTime;
extern int TimesSeen;

uint8_t channel = 1;
uint32_t sendEntry = 0;
//int state = 0;
volatile uint16_t people = 0;

//char receivedChars[BUFFER_IN];
//char tempChars[BUFFER_IN];

extern void sendMQTT();
extern boolean MQTTconnect();
bool ConnectWiFi();

String date;
WiFiClient client_wifi;
extern MQTTClient client;
WiFiUDP udp;

NTPClient ntp(udp, "ca.pool.ntp.org", -3 * 3600, 60000);

void setup() {
  Serial.begin(BAUD_RATE);
  #if(DEBUG_MODE)
  Serial.println("Contador > Iniciando");
  #endif
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(-TXPOWER);
  //ConnectWiFi();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.begin("linsse.com.ar", 2000 ,client_wifi);
  //client.begin("broker.shiftr.io",client_wifi); 
  client.onMessage(messageReceived);

  //ConnectWiFi();
  
  snifferSetup();
  //Serial.setRxBufferSize(BUFFER_SIZE);
  //pinMode(LED, OUTPUT);
  ntp.begin();
  //ntp.forceUpdate();
}

void SnifferStop(){
  #if(DEBUG_MODE)
  Serial.println("Sniffer > OFF");
  #endif
  wifi_promiscuous_enable(DISABLE);
  #if(DEBUG_MODE)
  Serial.println("AP ==> STATION");
  #endif
}

void StationBegin(){
  delay(10);// We start by connecting to a WiFi network
  #if(DEBUG_MODE)
  Serial.println("STATION > Configurando modo estacion");
  #endif
  delay(100);
  WiFi.disconnect();
  delay(100);
  WiFi.persistent(false);
  delay(100);

  WiFi.mode(WIFI_OFF);

  WiFi.mode(WIFI_STA);
}

bool ConnectWiFi(){
  int retry = 0;

  #if(DEBUG_MODE)
  Serial.print("STATION > Conectando al SSID: ");
  Serial.println(WIFI_SSID);
  #endif

  channel = 1;
  wifi_set_channel(channel);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, channel);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if(DEBUG_MODE)
      Serial.print(".");
    #endif
    retry++;
    if (retry > 20)
    {
    #if(DEBUG_MODE)
      Serial.print(" -> No conectado");
    #endif
      return false;
    }
  }
#if(DEBUG_MODE)
  Serial.println("\r\nSTATION > Conexion exitosa");
  Serial.print("STATION > Dirreccion IP: ");
  Serial.println(WiFi.localIP());
#endif
  return true;
}

void snifferSetup() {
  wifi_promiscuous_enable(DISABLE);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
#if(DEBUG_MODE)
  Serial.println("Sniffer > ON");
#endif
  wifi_promiscuous_enable(ENABLE);
}

void calculatePeople() {
  int i = 0;

  for (int u = 0; u < probes_known_count; u++)
  {
    if (probes_known[u].reported >= TimesSeen)
    {
      i++;
    }
  }
  people = i;
}

void SnifferBegin(){
  #if(DEBUG_MODE)
  Serial.println("Sniffer > ON");
  #endif
  wifi_promiscuous_enable(ENABLE);
  sendEntry = millis() / 1000;
}

void purgeDevice() {
  uint32_t now = millis() / 1000;

  for (int u = 0; u < clients_known_count; u++) {
    if ((now - clients_known[u].lastDiscoveredTime) > purge) {
#if(CLIENT_DEBUG)
      Serial.printf("Descartando Client: %d  -> Ausente \r\n", u + 1);
#endif
      for (int i = u; i < clients_known_count; i++) memcpy(&clients_known[i], &clients_known[i + 1], sizeof(clients_known[i]));
      clients_known_count--;
      break;
    }
  }
  for (int u = 0; u < aps_known_count; u++) {
    if ((now - aps_known[u].lastDiscoveredTime) > purge) {
#if(BEACON_DEBUG)
      Serial.printf("Descartando Beacon: %d  -> Ausente \r\n", u + 1);
#endif

      for (int i = u; i < probes_known_count; i++) memcpy(&probes_known[i], &probes_known[i + 1], sizeof(probes_known[i]));
      probes_known_count--;
      break;
    }
  }
  
  for (int u = 0; u < probes_known_count; u++) {    
    if ((now - probes_known[u].lastDiscoveredTime) > purge) {
    #if(PROBE_DEBUG)
     Serial.printf("Descartando Probe: %d -> Ausente\r\n", u + 1);
     #endif
      #if(SENDER_MODE)
      //state = 0;
      uint32_t lht = (now - probes_known[u].lastDiscoveredTime);
       /*
       Serial.printf("<%i|",state);
       Serial.print(formatMac1(probes_known[u].station));
       Serial.printf("|%d|%d|%d\n",probes_known[u].rssi,lht,probes_known[u].reported);
       */
      #endif

      for (int i = u; i < probes_known_count; i++) memcpy(&probes_known[i], &probes_known[i + 1], sizeof(probes_known[i]));
      probes_known_count--;
      break;
    }
  }
}

void sendDevices() {
  int retry = 0;
  
  //WiFiClient client_wifi;
  
  SnifferStop();
  StationBegin();
  
  delay(100);
  if(!ConnectWiFi())
  {
    Serial.println("WiFi > Falla de conexion --> Envio de datos anulado");
    SnifferBegin();
    return;
  }
  delay(100);

  ntp.forceUpdate();
  date = ntp.getFormattedTime();
  //Serial.println(ntp.getFormattedTime());

  delay(100);

  #if(DEBUG_MODE)
  Serial.println("CLIENT > Conectandose a la base de datos de: MQTT");
  #endif
  MQTTconnect();
  #if(DEBUG_MODE)
  Serial.println("MQTT > Enviando datos");
  #endif
  sendMQTT();
  //GenerateInsane();
  SnifferBegin();
}

void loop() {
  //digitalWrite(LED, !LOW);
  channel = 1;
  boolean sendInfo = false;
  uint32_t now = millis() / 1000;
  wifi_set_channel(channel);

  while (true) {
    nothing_new++;                          // Array is not finite, check bounds and adjust if required
    if (nothing_new > 200) {                // monitor channel for 200 ms
      nothing_new = 0;
      channel++;
      if (channel == 15) break;             // Only scan channels 1 to 14
      wifi_set_channel(channel);
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
    if((now - sendEntry) > SendTime){
      sendEntry = millis() / 1000;
      sendInfo = true;
    }
    purgeDevice();
  }
  calculatePeople();
  //recvWithStartEndMarkers(); 
  //[150|-80|60|10
  //[150000|-80|10|10

  /*
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    //showParsedData();
    newData = false;
  }
  */  

  //timeClient.update();
  //hora = ntp.getFormattedTime();
  
  //Serial.println(ntp.getFormattedTime());
  
  if (sendInfo) {
    //digitalWrite(LED, !HIGH);
    //showDevices();
    sendDevices();
  }
}

