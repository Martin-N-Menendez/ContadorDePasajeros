#include <Arduino.h>
//#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
//#include <ArduinoOTA.h>
//#include <PubSubClient.h>
#include <MQTT.h>
#include <set>

#include "Estructuras.h"
#include "Configuracion.h"

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;
//PubSubClient mqttClient(wifiClient);
MQTT myMqtt(ESP_NAME, MQTT_HOST, MQTT_PORT);

WiFiEventHandler probeRequestHandler;

clt_device_t devicelist[MAX_DEVICES];

//char jsonString[JBUFFER];
//StaticJsonBuffer<200> jsonBuffer;

uint64_t sendEntry = 0;
uint32_t times = 1;
uint32_t ack = 0;

void inline handler (void);

void setup() {
  initSerial();
  initWiFi();
  initMQTT();
  //mqttClient.setCallback(callback);
  
  //probeRequestHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequest); // Handler de probe request

  //timer0_isr_init();
  //timer0_attachInterrupt(handler);
  //timer0_write(ESP.getCycleCount() + timer0_preload * my_delay);
  //interrupts();
  delay(10);
  SuperFakeSniffer();
}

void inline handler (void) {

  uint32_t heap = ESP.getFreeHeap();
  //Serial.printf("<%u>\r\n",ESP.getFreeHeap());
  if (heap < 2500)
  {
    Serial.printf("<%u>\r\n", ESP.getFreeHeap());
    Serial.println("Cantidad de heap critico! desconectando MQTT");
    //mqttClient.disconnect();
    //  Serial.printf("<%u>\r\n",ESP.getFreeHeap());
  }

  timer0_write(ESP.getCycleCount() + timer0_preload * my_delay);
}

void loop() {

  // Handle wifi connection
  if (wifiMulti.run() != WL_CONNECTED) {
    WiFi.disconnect();
    wifiConnect();
    return;
  }

  if(!myMqtt.isConnected())
  {
    myMqtt.connect();
    delay(500);
    return;  
  }
  
  // Handle MQTT
  /*if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();*/
    
  checkList();    // Handle devices list
  calculatePeople();
  //ReadConfig();
  
  if (millis() - sendEntry > SENDTIME * 1000) {
    sendEntry = millis();
    HeaderToMQTT();
    jSonToMQTT();
    times++;
  }
}

void SuperFakeSniffer()
{
   for (int i = 0; i < MAX_DEVICES; i++) { 
      devicelist[i].mac = "AA:BB:CC:DD:EE:FF";
      devicelist[i].rssi = -15;
      devicelist[i].ms = 11111;
      devicelist[i].reported = 65535;
    }
}

void ReadConfig(){
  //mqttClient.subscribe("/cdp/configout");
}

void HeaderToMQTT() {

  String date = getTime();
  String aux =  "{\"Type\":\"Header\",\"i\":" + String(times) + ",\"N\":" + String(N_devices) + ",\"P\":" + String(people) + ",\"B\":" + String((float)ESP.getVcc() / 1024.0) + ",\"T\":\"" + date.c_str() + "\",\"heap\":" + ESP.getFreeHeap() + "}";

  myMqtt.publish(MQTT_OUT_TOPIC, aux);
  myMqtt.subscribe(MQTT_OUT_TOPIC);
  //delay(FREQ);
  
  Serial.println("---------HEADER--------");
  Serial.printf("[Header] Veces: %d, Dispositivos: %d, Personas: %d, Batería: %.2fV, Duración: %s, Heap: %d\n", times, N_devices, people, (float)ESP.getVcc() / 1024.0, date.c_str(), ESP.getFreeHeap());
  Serial.println("---------HEADER--------");

  ack = 1;
}

void jSonToMQTT() {

  uint16_t i, j, packet, sent, sub;
  uint16_t retry = 0;
  uint32_t memory = 0;

  String JsonBuffer = "";
  String Place = "";
  String MACBuffer = "\"MAC\":[";
  String RSSIBuffer = "\"RSSI\":[";
  String LHTBuffer = "\"LHT\":[";
  String REPBuffer = "\"REP\":[";

  Serial.println("---------DATA--------");
  
  j = 0;
  packet = 0;
  sent = 0;
  sub = 0;
  for (i = 0; i < MAX_DEVICES; i++)
  {
    ESP.wdtFeed();
    ESP.wdtEnable(0);
    if (devicelist[i].mac != "")
    {
      j++;
      MACBuffer += "\"";
      MACBuffer += (devicelist[i].mac).c_str();
      MACBuffer += "\"";
      RSSIBuffer += devicelist[i].rssi;
      LHTBuffer += ((millis() - devicelist[i].ms) / 1000UL);
      REPBuffer += devicelist[i].reported;

      if (!(j % CHOP))
      {
        packet++;
        
        MACBuffer += ']';
        RSSIBuffer += ']';
        LHTBuffer += ']';
        REPBuffer += ']';
    
        Place = "\"i:\""+String(times) + ',' + "\"P\":"+String(packet);
        
        memory = ESP.getFreeHeap() ;
        JsonBuffer = "{\"Type\":\"Data\"," + Place + ',' + MACBuffer + ',' + RSSIBuffer + ',' + LHTBuffer + ',' + REPBuffer + '}';

        Serial.printf("%d - %d  | %d -%d | ", N_devices, packet, memory, ESP.getFreeHeap());
        Serial.println(JsonBuffer);     

        myMqtt.publish(MQTT_OUT_TOPIC, JsonBuffer);
        myMqtt.subscribe(MQTT_OUT_TOPIC);
        sub++;

        //delay(FREQ);

        JsonBuffer = "";
        MACBuffer = "\"MAC\":[";
        RSSIBuffer = "\"RSSI\":[";
        LHTBuffer = "\"LHT\":[";
        REPBuffer = "\"REP\":[";
        continue;
      }
      if (j < N_devices)
      {
        MACBuffer += ',';
        RSSIBuffer += ',';
        LHTBuffer += ',';
        REPBuffer += ',';
      }
    }
  }

  if (N_devices % CHOP)
  {
    packet++;
    
    MACBuffer += ']';
    RSSIBuffer += ']';
    LHTBuffer += ']';
    REPBuffer += ']';

    Place = "\"i\":"+String(times) + ',' + "\"P\":"+String(packet);
        
    memory = ESP.getFreeHeap() ;
    JsonBuffer = "{\"Type\":\"Data\"," + Place + ',' + MACBuffer + ',' + RSSIBuffer + ',' + LHTBuffer + ',' + REPBuffer + '}';

    Serial.printf("%d - %d  | %d -%d | ", N_devices, packet, memory, ESP.getFreeHeap());
    Serial.println(JsonBuffer);

    Serial.println("---------DATA--------");

    myMqtt.publish(MQTT_OUT_TOPIC, JsonBuffer);
    myMqtt.subscribe(MQTT_OUT_TOPIC);
    sub++;
    //delay(FREQ);
  }

  Serial.printf("%d ACK de %d paquetes\r\n", sub,packet);
  ack += sub;
}

void checkList() {

  uint16_t i;
  for (i = 0; i < MAX_DEVICES; i++) {
    if (devicelist[i].mac != "" && (millis() - devicelist[i].ms > LIST_TIMEOUT * 1000)) {
      /* mqttClient.subscribe(MQTT_OUT_TOPIC);
        // Publish "Out" event
        char msg[110] = "";
        snprintf(msg, sizeof(msg),
                "{\"event\":\"out\",\"mac\":\"%s\",\"rssi\":%d,\"uptime\":%d,\"heap\":%d}",
                (devicelist[i].mac).c_str(), devicelist[i].rssi, getUptimeSecs(), ESP.getFreeHeap()
        );

        mqttClient.publish(MQTT_OUT_TOPIC, msg);
        mqttClient.subscribe(MQTT_OUT_TOPIC);
      */
      // Clear expired device
      Serial.printf("[Device Out] MAC: %s\n", (devicelist[i].mac).c_str());
      devicelist[i] = {.mac = "", .rssi = 0, .ms = 0, .reported = 0};

    }
  }

  // Check console and print list if \n char is detected
  char c;
  while (Serial.available() > 0) {
    c = Serial.read();
    if (c == '\n') {
      printlist();
      //jSonToMQTT();
    }
  }
}

String getTime() {
  String date = "";
  uint32_t timeNow = millis();

  uint16_t days = timeNow / day ;
  uint8_t hours = (timeNow % day) / hour;
  uint8_t minutes = ((timeNow % day) % hour) / minute ;
  uint8_t seconds = (((timeNow % day) % hour) % minute) / second;

  date += (days < 10) ? ('0' + String(days)) : (String(days));
  date += ':';
  date += (hours < 10) ? ('0' + String(hours)) : (String(hours));
  date += ':';
  date += (minutes < 10) ? ('0' + String(minutes)) : (String(minutes));
  date += ':';
  date += (seconds < 10) ? ('0' + String(seconds)) : (String(seconds));

  return date;
}

void calculatePeople() {
  uint16_t i;

  N_devices = 0;
  people = 0;

  // Count devices
  for (i = 0; i < MAX_DEVICES; i++) {
    if ((devicelist[i].mac != "") && (devicelist[i].rssi != 0)) {
      N_devices++;
      if (devicelist[i].reported > REPORTED)
        people++;
    }
  }
}

void printlist() {

  uint16_t i;
  String date = getTime();

  Serial.printf("\r\nDispositivos: %d | Personas: %d | Tension: %.2f V (Funcionando hace %s)\r\n", N_devices, people, (float)ESP.getVcc() / 1024.0, date.c_str());

  if (N_devices > 0) {                  // List devices
    for (i = 0; i < MAX_DEVICES; i++) {
      ESP.wdtFeed();
      ESP.wdtEnable(0);
      if (devicelist[i].mac != "") {
        Serial.printf(
          "MAC: %s, RSSI: %d, Reported: %d, Visto hace %u segundos\r\n",
          (devicelist[i].mac).c_str(),
          devicelist[i].rssi,
          devicelist[i].reported,
          (unsigned int)((millis() - devicelist[i].ms) / 1000UL)
        );
        delay(1);
      }
    }
  }
  Serial.println();
}

void onProbeRequest(const WiFiEventSoftAPModeProbeRequestReceived &evt) {

  String mac = macToString(evt.mac);

  if (evt.rssi <= MIN_RSSI) // Filtro de potencia
  {
    Serial.printf("%d,%d\r\n", evt.rssi, MIN_RSSI);
    return;
  }

  //da:a1:19:XX:XX:XX son RANDOM, hay que filtrar
  //0c:cb:85:XX:XX:XX son RANDOM? AVERIGUAR!
  if (!strncmp(mac.c_str(), "da:a1:19", 8)) // Filtro de MAC aleatoria
  {
    Serial.println("Random mac!");
    return;
  }

  if (!strncmp(mac.c_str(), "0c:cb:85", 8)) // Filtro de MAC aleatoria
  {
    Serial.println("Random mac!");
    return;
  }
  uint16_t i;
  for (i = 0; i < MAX_DEVICES; i++) {   // Actualizando MACs ya ingresadas
    if (devicelist[i].mac == mac) {
      // Update device
      devicelist[i].rssi = evt.rssi;
      devicelist[i].ms = millis();
      devicelist[i].reported++;
      return;
    }
  }

  for (i = 0; i < MAX_DEVICES; i++) {   // Agregando MAC nueva
    if (devicelist[i].mac == "") {

      //Add device
      devicelist[i] = {.mac = mac, .rssi = (int8_t)evt.rssi, .ms = millis(), .reported = 0};

      Serial.printf("[Device In] MAC: %s, RSSI: %d, LHT: %u Reported: %d\n", mac.c_str(), evt.rssi, millis(), devicelist[i].reported);

      /*
        mqttClient.subscribe(MQTT_OUT_TOPIC);
        // Publish "In" event
        char msg[110] = "";
        snprintf(msg, sizeof(msg),
               "{\"event\":\"in\",\"mac\":\"%s\",\"rssi\":%d,\"reported\":%d,\"uptime\":%d,\"heap\":%d}",
               mac.c_str(), evt.rssi,devicelist[i].reported, getUptimeSecs(), ESP.getFreeHeap()
        );
        mqttClient.publish(MQTT_OUT_TOPIC, msg);
        mqttClient.subscribe(MQTT_OUT_TOPIC);
      */
      return;
    }
  }
  Serial.println("No se pudo agregar a la lista");
}

void initSerial() {
  Serial.begin(115200);
  String resetInfo = ESP.getResetReason();

  Serial.setDebugOutput(SERIAL_SET_DEBUG_OUTPUT);
  Serial.printf("\n\n     %s v%s\n\n", ESP_NAME, VERSION);
  Serial.println("********** Crashes ***********");
  Serial.printf("Reset Reason:       %s\n", resetInfo.c_str());
  Serial.println("********** Firmware ***********");
  Serial.printf("STA MAC:            %s\n", WiFi.macAddress().c_str());
  Serial.printf("AP MAC:             %s\n", WiFi.softAPmacAddress().c_str());
  Serial.printf("Chip ID:            %6X\n", ESP.getChipId());
  Serial.printf("Sketch size:        %s\n", prettyBytes(ESP.getSketchSize()).c_str());
  Serial.printf("Free Sketch size:   %s\n", prettyBytes(ESP.getFreeSketchSpace()).c_str());
  Serial.printf("Chip size:          %s\n", prettyBytes(ESP.getFlashChipRealSize()).c_str());
  Serial.printf("Heap size:          %s\n", prettyBytes(ESP.getFreeHeap()).c_str());
  Serial.printf("SDK version:        %s\n", ESP.getSdkVersion());
  Serial.printf("CPU frequency:      %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Boot Mode           %u\n", ESP.getBootMode());
  Serial.printf("Boot Version:       %u\n", ESP.getBootVersion());
  Serial.println("******** Configuration *********");
  Serial.printf("Max devices:        %d\n", MAX_DEVICES);
  Serial.printf("Packet size:        %d\n", CHOP);
  Serial.printf("Min RSSI:           %d\n", MIN_RSSI);
  Serial.printf("Max time:           %d segundos\n", LIST_TIMEOUT);
  Serial.printf("Min seen:           %d veces\n", REPORTED);
  Serial.printf("Send each:          %d segundos\n", SENDTIME);
  Serial.println("************ Train  ************");
  Serial.printf("Line:               %s\n", String(lines[LINE]).c_str());
  Serial.printf("Formation:          %d\n", TRAIN);
  Serial.printf("Car:                %d\n", CAR);
  Serial.println("*******************************");
}

void wifiConnect() {
  //WiFi.setOutputPower(20.5);

  while (wifiMulti.run() != WL_CONNECTED) {
    wifiClient = WiFiClient();
    delay(500);
  }
  Serial.printf("Conectado a %s con IP %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void initWiFi() {
  WiFi.hostname(ESP_NAME);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setOutputPower(20.5);
  // Connect station
  Serial.println("Conectando a WiFi...");
  uint8_t i, s = sizeof(AP_LIST) / sizeof AP_LIST[0];
  for (i = 0; i < s; i++) {
    wifiMulti.addAP(AP_LIST[i].ssid, AP_LIST[i].pwd);
  }
  wifiConnect();

  // Start AP
  WiFi.softAP(AP_SSID, AP_PASSWORD);
}

/*
void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("ACK: [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
     Serial.print((char)payload[i]);
    }
    Serial.println();

    if(strcmp(topic,"/cdp/configout"))
    {
        ack--;
        if(ack == 0)
          mqttClient.disconnect();
    }
}*/

void mqttConnect() {

  Serial.println("MQTT > Conectando...");
  myMqtt.connect();

  myMqtt.subscribe(MQTT_OUT_TOPIC);
  delay(10);
  /*
  wifiClient = WiFiClient();
  while (!mqttClient.connected()) {
    Serial.println("MQTT > Conectando...");
    delay(1000);
    if (mqttClient.connect(ESP_NAME)) {
      Serial.printf("Conectado a %s\n", MQTT_HOST);
      //mqttClient.publish(MQTT_OUT_TOPIC, "{\"event\":\"connected\"}");
      //mqttClient.subscribe(MQTT_OUT_TOPIC);
    } else {
      Serial.print("MQTT > Falla!, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Reintentando en 5 segundos");
      delay(5000);
    }
  }
  */
}

void initMQTT() {
  //mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  //mqttConnect();

  myMqtt.onConnected(myConnectedCb);
  myMqtt.onDisconnected(myDisconnectedCb);
  myMqtt.onPublished(myPublishedCb);
  myMqtt.onData(myDataCb);
  
  mqttConnect();
}

void myConnectedCb(){
  Serial.println("MQTT > Conectado");
  String aux = "Conectado";
  boolean result = myMqtt.publish(MQTT_OUT_TOPIC, aux);
  myMqtt.subscribe(MQTT_OUT_TOPIC);
}

void myDisconnectedCb(){
  Serial.println("MQTT > Desconectado, reintentando conectar");
  delay(500);
  myMqtt.connect();
}

void myPublishedCb(){
  Serial.println("Publicado");
}

void myDataCb(String& topic, String& data)
{
  Serial.print("ACK: [");
  Serial.print(topic);
  Serial.print("] ");
  //Serial.print(" : ");
  Serial.println(data);
}

uint32_t getUptimeSecs() {
  static uint32_t uptime = 0;
  static uint32_t previousMillis = 0;
  uint32_t now = millis();

  uptime += (now - previousMillis) / 1000UL;
  previousMillis = now;
  return uptime;
}

String prettyBytes(uint32_t bytes) {

  const char *suffixes[7] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
  uint8_t s = 0;
  double count = bytes;

  while (count >= 1024 && s < 7) {
    s++;
    count /= 1024;
  }
  if (count - floor(count) == 0.0) {
    return String((int) count) + suffixes[s];
  } else {
    return String(round(count * 10.0) / 10.0, 1) + suffixes[s];
  };
}

String macToString(const uint8 mac[6]) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}
