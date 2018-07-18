#include "Stations.h"
#include "Sniffer.h"

#define LINE LINEA2
#define TRAIN 1
#define CAR 5

#define SENDER "PruebaTBA1"
#define USER "fb033abd"
#define KEY "e2ebc570ab73e0b5"

#define ADDRESS "/cdp/"+String(lines[LINE])+"/Formacion_"+TRAIN+"/Coche_"+CAR+"/"
String Json = "";

extern String date;
extern WiFiClient client_wifi;
extern volatile uint16_t probes_known_count;
extern bool ConnectWiFi();

MQTTClient client(256);

// ################ Enviar por MQTT ############
void sendMQTT() {
  int retry = 0;
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    if (!MQTTconnect())
    {
      retry++;
      Serial.print(" --> Reintentando ");
      Serial.print(retry);
      Serial.println("/3");
      delay(100);
      if (retry > 3)
      {
        Serial.println("\r\n Falla de conexion ... Abortando subida de datos");
        return;
      }
    }
  }

  BreakMQTT();

  client.subscribe(ADDRESS);
  delay(150);

  retry = 0;
  //Serial.print("MQTT > ");
  String Header = "";
  Header += "{\"Id\":\"" + String(SENDER) + "\",\"Date\":\"" + date + "\",\"Type\":\"Header\",\"People\":" + people + ",\"MACs\":" + probes_known_count + "}";
  while (!client.publish(ADDRESS, Header))
  {
    retry++;
    Serial.println("MQTT > Falla al enviar cabecera");
    MQTTconnect();
    //Serial.print("MQTT > ");
    //client.publish(ADDRESS,Header);
    if (retry > 3) break;
  }
  client.subscribe(ADDRESS);
  //delay(150);

  /*
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
  */
}

// ################ Reconectar Wifi y MQTT server ############
boolean MQTTconnect() {
  int retry = 0;

  client.unsubscribe(ADDRESS);
  delay(100);
  client.disconnect();
  delay(100);
  client.begin("linsse.com.ar", 2000 , client_wifi);
  delay(100);

  while (!client.connect(SENDER)) {
    delay(500);
    Serial.print("*");
    retry++;
    if (retry > 20)
    {
      Serial.println("\r\n MQTT > No se pudo conectar");
      return false;
    }
  }
  Serial.println("\r\nMQTT > Conectado");

  client.subscribe(ADDRESS);

  return true;
}

// ################ Mensaje recibido ############
void messageReceived(String &topic, String &payload) {
  Serial.println("MQTT > Leyendo: " + topic + " - " + payload);
}

// ################ Romper aleatoriamente la conexion al broker ############
void BreakMQTT() {
  int i = random(0, 5);

  if (i == 2)
  {
    Serial.println("BROKER > ESTO SE VA A DESCONECTAAAR!");
    client.disconnect();
  }

}

// ################ Mensaje recibido ############
void GenerateInsane() {

  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    if (!MQTTconnect())
    {
      Serial.println("Falla de conexion ... Abortando subida de datos");
      return;
    }
  }

  for (int i = 1; i <= 6 ; i++)
  {
    for (int j = 1; j <= 5; j++)
    {
      for (int k = 1; k <= 5; k++)
      {
        String linea = "";
        linea += lines[i];
        linea += "/Formacion_";
        linea += String(j);
        linea += "/Coche_";
        linea += String(k);
        linea += '/';
        linea += '\0';

        if (!client.publish(linea, "0"))
        {
          Serial.println("Falla al generar");
          client.publish(linea, "0");
        }
      }
    }
  }
}

