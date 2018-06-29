#include "Stations.h"

#define LINE LINEA3
#define TRAIN 12
#define CAR 5

#define SENDER "GICSAFe"
#define USER "fb033abd"
#define KEY "e2ebc570ab73e0b5"

String address = "/"+String(lines[LINE])+"/Formacion_"+TRAIN+"/Coche_"+CAR+"/";

MQTTClient client;
 String date = "HOLA";
  String t = "MUNDO";

// ################ Leer datos de MQTT ############
void ReadConfig_MQTT(){
    digitalWrite(LED, !HIGH);
    connect();

    client.subscribe("/Configuracion/");
    client.publish("/Configuracion/","150|-99|180|3");
    //client.subscribe("/Configuracion/");
}

// ################ Enviar por MQTT ############
void sendMQTT(){
  //Serial.println("A");
  client.loop();
  //Serial.println("B");
  delay(10);  // <- fixes some issues with WiFi stability
  //Serial.println("C");
  
  if (!client.connected()) {
    connect();
  }
   
  //Serial.println("D");
  client.subscribe(address);
  //client.publish(address,String(date)+' '+String(t)+'|'+String(people)+'|'+String(idx));
  //Serial.println("E");
  delay(100);
  int j=0;
  while(!client.publish(address,"{\"Fecha\":\""+date+' '+t+"\",\"People\":"+people+",\"MACs\":"+probes_known_count+"}"))
  {
    j++;
    Serial.println("Falla al enviar cabecera");
    connect();
    client.publish(address,"{\"Fecha\":\""+date+' '+t+"\",\"People\":"+people+",\"MACs\":"+probes_known_count+"}");
    if(j>=5) break;
  }
  //Serial.println("F");
  client.subscribe(address);
  delay(100);


  for(int u=0; u < probes_known_count; u++)
  {
     int j=0;
     uint64_t lht = (millis()/1000 - probes_known[u].lastDiscoveredTime);
     
     delay(150);     

     Serial.print(u+1);
     Serial.printf("(%i)\r\n",lht);
  
     while(!client.publish(address+String(u+1)+"/","{\"MAC\":\""+formatMac1(probes_known[u].station)+"\",\"RSSI\":"+probes_known[u].rssi+",\"LHT\":"+(int)lht+",\"Reported\":"+probes_known[u].reported+"}"))
     {
      j++;
      Serial.printf("Falla al enviar dato:%i\r\n",u);
      connect();
      client.publish(address+String(u+1)+"/","{\"MAC\":\""+formatMac1(probes_known[u].station)+"\",\"RSSI\":"+probes_known[u].rssi+",\"LHT\":"+(int)lht+",\"Reported\":"+probes_known[u].reported+"}");
      if(j>=5) break;
     } 
  }
}

// ################ Reconectar Wifi y MQTT server ############
void connect() {
  int retry = 0;
  //Serial.print("checking wifi");
  connectToWiFi();
  /*
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  */
  
  while (!client.connect(SENDER,USER,KEY)) {
    Serial.print(".");
    delay(500);
    if (retry > 20)
    {
      Serial.println(" -> No conectado");
      return;
    }
  }
  Serial.println();
  
  //Serial.println("\nconnected!");

  //Serial.println("C");
  //client.subscribe(address);
  //client.unsubscribe("/hello");
}

// ################ Mensaje recibido ############
void messageReceived(String &topic, String &payload) {

  /*
  if(payload[3] == '|')
  {
    Serial.println('['+payload);
    return;
  }
  */
  Serial.println("Leyendo: " + topic + " - " + payload);
}

// ################ Mensaje recibido ############
void GenerateInsane() {

  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
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
