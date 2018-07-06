#include "Stations.h"
#include "Sniffer.h"

#define LINE LINEA1
#define TRAIN 1
#define CAR 1

#define SENDER "Martin"
#define USER "fb033abd"
#define KEY "e2ebc570ab73e0b5"

#define ADDRESS "/cdp/"+String(lines[LINE])+"/Formacion_"+TRAIN+"/Coche_"+CAR+"/"
String Json = "";

extern volatile uint16_t probes_known_count;  

MQTTClient client(256);

// ################ Enviar por MQTT ############
void sendMQTT(){
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  
  if (!client.connected()) {
    if(!connect())
    {
      Serial.println("Falla de conexion ... Abortando subida de datos");
      return;
    }
  }
  client.subscribe(ADDRESS);
  delay(150);

  int j=0;
  Serial.print("MQTT > ");
  String Header = "";
  Header += "{\"Id\":\""+String(SENDER)+"\",\"Type\":\"Header\",\"People\":"+people+",\"MACs\":"+probes_known_count+"}";
  while(!client.publish(ADDRESS,Header))
  {
    j++;
    Serial.println("Falla al enviar cabecera");
    connect();
    Serial.print("MQTT > ");
    client.publish(ADDRESS,Header);
    if(j>=3) break;
  }
  client.subscribe(ADDRESS);
  delay(150);
  
  for(int u=0; u < probes_known_count; u++)
  { 
     int j=0;
     uint64_t lht = (millis()/1000 - probes_known[u].lastDiscoveredTime);
     String Information = "";    
  
     Information += "{\"Id\":\""+String(SENDER)+"\",\"Type\":\"Info\",\"Index\":\""+String(u+1)+"\",\"MAC\":\""+formatMac1(probes_known[u].station)+"\",\"RSSI\":"+probes_known[u].rssi+",\"LHT\":"+(int)lht+",\"Reported\":"+probes_known[u].reported+"}";

     //Serial.println(u+1);
     client.subscribe(ADDRESS+String(u+1)+"/");
     //Serial.printf("(%i)\r\n",lht);
  
     while(!client.publish(ADDRESS+String(u+1)+"/",Information))
     {
      j++;
      Serial.printf("Falla al enviar dato:%i\r\n",u+1);
      connect();
      client.publish(ADDRESS+String(u+1)+"/",Information);
      client.subscribe(ADDRESS+String(u+1)+"/");
      if(j>=3) break;
     } 
     client.subscribe(ADDRESS+String(u+1)+"/");
     delay(150);
  }
}

// ################ Reconectar Wifi y MQTT server ############
boolean connect() {
  int retry = 0;
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  #if(DEBUG_MODE)
    Serial.print(".");
  #endif
    retry++;
    if (retry > 20)
    {
      #if(DEBUG_MODE)
      Serial.println("Wifi > No se pudo conectar");
      #endif
      return false;
    }
  }
  retry = 0;
  while (!client.connect(SENDER,USER,KEY)) {
    delay(500);
    Serial.print(".");
    retry++;
    if (retry > 20)
    {
      Serial.println("MQTT > No se pudo conectar");
      return false;
    }
  }
  Serial.println("MQTT > Conectado");
  //client.subscribe(address);
  //client.unsubscribe("/hello");
  return true;
}

// ################ Mensaje recibido ############
void messageReceived(String &topic, String &payload) {
  Serial.println("Leyendo: " + topic + " - " + payload);
}

// ################ Mensaje recibido ############
void GenerateInsane() {

  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    if(!connect())
    {
      Serial.println("Falla de conexion ... Abortando subida de datos");
      return;
    }
  }
  
  for(int i=1;i <=6 ;i++)
  {   
    for(int j=1;j <= 5;j++)
    {
      for(int k=1;k <= 5;k++)
      {
        String linea = "";
        linea += lines[i];
        linea += "/Formacion_";
        linea += String(j);
        linea += "/Coche_";
        linea += String(k);
        linea += '/';
        linea += '\0';
        
        if(!client.publish(linea, "0"))
        {
         Serial.println("Falla al generar");
         client.publish(linea, "0");
        }
      }
    }
  }  
}
