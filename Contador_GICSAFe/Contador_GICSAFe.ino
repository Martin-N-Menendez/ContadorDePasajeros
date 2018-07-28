#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <set>

#include "Estructuras.h"
#include "Configuracion.h"

ESP8266WiFiMulti wifiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiEventHandler probeRequestHandler;

clt_device_t devicelist[MAX_DEVICES];

char jsonString[JBUFFER];
StaticJsonBuffer<JBUFFER>  jsonBuffer;

uint64_t sendEntry = 0;

void setup() {
    initSerial();
    initWiFi();
    initMQTT();
    mqttClient.setCallback(callback);   
    probeRequestHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequest); // Handler de probe request
}

void loop() {
    
    // Handle wifi connection
    if (wifiMulti.run() != WL_CONNECTED) {
        WiFi.disconnect();
        wifiConnect();
        return;
    }

    // Handle MQTT
    if (!mqttClient.connected()) {
        mqttConnect();
    }
    mqttClient.loop();

    // Handle devices list
    checkList();

    if (millis() - sendEntry > SENDTIME * 1000) {
      sendEntry = millis();
      jSonToMQTT();
    }    
}

void jSonToMQTT(){

  uint8_t i, c = 0, people = 0;

   // Count devices
    for (i = 0; i < MAX_DEVICES; i++){       
        if ((devicelist[i].mac != "") && (devicelist[i].rssi != 0)){
            c++;
            if(devicelist[i].reported > REPORTED)
              people++;
        }        
    } 
    
  jsonBuffer.clear();
  
  JsonObject& root = jsonBuffer.createObject();

  //root["Devices"] = c;
  //root["People"] = people;
  //root["Battery"] = (float)ESP.getVcc()/1024.0
  
  JsonArray& jMAC = root.createNestedArray("MAC");
  JsonArray& jRSSI = root.createNestedArray("RSSI");  
  JsonArray& jREPORTED = root.createNestedArray("REPORTED");  
  JsonArray& jLHT = root.createNestedArray("LHT");

  for (int i = 0; i < MAX_DEVICES; i++)
  {      
    if (devicelist[i].mac != "")
    {
      jMAC.add((devicelist[i].mac).c_str()); 
      jRSSI.add(devicelist[i].rssi);
      jLHT.add((millis() - devicelist[i].ms) / 1000UL);  
      jREPORTED.add(devicelist[i].reported); 
    }        
   }    
    
    Serial.println("---------JSON--------");
    root.prettyPrintTo(Serial);
    root.printTo(jsonString);

    mqttClient.subscribe(MQTT_OUT_TOPIC);
    if (mqttClient.publish(MQTT_OUT_TOPIC, jsonString) == 1) 
      Serial.printf("\r\nMQTT > Publicado exitosamente\r\n");
    else
    {
      Serial.printf("\r\nMQTT > Error al publicar\r\n");
    }
    mqttClient.subscribe(MQTT_OUT_TOPIC);
  
}

void checkList() {

    uint8_t i;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (devicelist[i].mac != "" && (millis() - devicelist[i].ms > LIST_TIMEOUT * 1000)) {
            mqttClient.subscribe(MQTT_OUT_TOPIC);
            // Publish "Out" event
            char msg[90] = "";
            snprintf(msg, sizeof(msg),
                     "{\"event\":\"out\",\"mac\":\"%s\",\"rssi\":%d,\"uptime\":%d,\"heap\":%d}",
                     (devicelist[i].mac).c_str(), devicelist[i].rssi, getUptimeSecs(), ESP.getFreeHeap()
            );
            mqttClient.publish(MQTT_OUT_TOPIC, msg);
            mqttClient.subscribe(MQTT_OUT_TOPIC);
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

void printlist() {

    uint8_t i, c = 0, people = 0;

    uint32_t timeNow = millis();
    
    uint16_t days = timeNow / day ;                                
    uint8_t hours = (timeNow % day) / hour;                       
    uint8_t minutes = ((timeNow % day) % hour) / minute ;         
    uint8_t seconds = (((timeNow % day) % hour) % minute) / second;
     
    // Count devices
    for (i = 0; i < MAX_DEVICES; i++){       
        if ((devicelist[i].mac != "") && (devicelist[i].rssi != 0)){
            c++;
            if(devicelist[i].reported > REPORTED)
              people++;
        }        
    } 
  
    Serial.printf("\r\nDispositivos: %d | Personas: %d | Tension: %.2f V (Funcionando hace %02ld:%02ld:%02ld:%02ld)\r\n", c,people,(float)ESP.getVcc()/1024.0,(long int)days,(long int)hours,(long int)minutes,(long int)seconds);

    // List devices
    if (c > 0) {
        for (i = 0; i < MAX_DEVICES; i++) {
            if (devicelist[i].mac != "") {
                Serial.printf(
                        "MAC: %s, RSSI: %d, Reported: %d, Visto hace %u segundos\r\n",
                        (devicelist[i].mac).c_str(),
                        devicelist[i].rssi,
                        devicelist[i].reported,
                        (unsigned int)((millis() - devicelist[i].ms) / 1000UL)
                );
            }
        }
    }
    Serial.println();
}
  
void onProbeRequest(const WiFiEventSoftAPModeProbeRequestReceived &evt) {

    String mac = macToString(evt.mac);

    uint8_t i;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (devicelist[i].mac == mac) {
            // Update device
            devicelist[i].rssi = evt.rssi;
            devicelist[i].ms = millis();
            devicelist[i].reported++;
            return;
        }
    }

    if(evt.rssi >= MIN_RSSI)
      return;
              
    for (i = 0; i < MAX_DEVICES; i++) {
        if (devicelist[i].mac == "") {
          
            //Add device
            devicelist[i] = {.mac = mac, .rssi = (int8_t)evt.rssi, .ms = millis(), .reported = 0};
            
            Serial.printf("[Device In] MAC: %s, RSSI: %d, LHT: %u Reported: %d\n", mac.c_str(), evt.rssi,(unsigned int)(devicelist[i].ms-millis()),devicelist[i].reported);

            mqttClient.subscribe(MQTT_OUT_TOPIC);
            // Publish "In" event
            char msg[90] = "";
            snprintf(msg, sizeof(msg),
                     "{\"event\":\"in\",\"mac\":\"%s\",\"rssi\":%d,\"reported\":%d,\"uptime\":%d,\"heap\":%d}",
                     mac.c_str(), evt.rssi,devicelist[i].reported, getUptimeSecs(), ESP.getFreeHeap()
            );
            mqttClient.publish(MQTT_OUT_TOPIC, msg);
            mqttClient.subscribe(MQTT_OUT_TOPIC);
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
    Serial.printf("Chip ID:            %6X\n",ESP.getChipId());
    Serial.printf("Sketch size:        %s\n", prettyBytes(ESP.getSketchSize()).c_str());
    Serial.printf("Free Sketch size:   %s\n", prettyBytes(ESP.getFreeSketchSpace()).c_str());
    Serial.printf("Chip size:          %s\n", prettyBytes(ESP.getFlashChipRealSize()).c_str());
    Serial.printf("SDK version:        %s\n", ESP.getSdkVersion());
    Serial.printf("CPU frequency:      %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Boot Mode           %u\n", ESP.getBootMode());
    Serial.printf("Boot Version:       %u\n", ESP.getBootVersion());
    Serial.println("******** Configuration *********");
    Serial.printf("Max devices:        %d\n", MAX_DEVICES);
    Serial.printf("Min RSSI:           %d\n", MIN_RSSI);
    Serial.printf("Max time:           %d segundos\n", LIST_TIMEOUT);
    Serial.printf("Min seen:           %d veces\n", REPORTED);
    Serial.printf("Send each:          %d segundos\n", SENDTIME);
    Serial.println("********************************");
}

void wifiConnect() {
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(500);
    }
    Serial.printf("Conectado a %s con IP %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void initWiFi() {

    WiFi.hostname(ESP_NAME);
    WiFi.mode(WIFI_AP_STA);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("ACK: [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  /*
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  */
}

void mqttConnect() {

    while (!mqttClient.connected()) {
        Serial.println("MQTT > Conectando...");
        if (mqttClient.connect(ESP_NAME)) {
            Serial.printf("Conectado a %s\n", MQTT_HOST);
            mqttClient.publish(MQTT_OUT_TOPIC, "{\"event\":\"connected\"}");
        } else {
            Serial.print("MQTT > Falla!, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Reintentando en 5 segundos");
            delay(5000);
        }
    }
}

void initMQTT() {
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttConnect();
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
