#include "Sender.h"
// ####################### Buffers ########################
const int numChars = BUFFER_IN;
char receivedChars[numChars];
char tempChars[numChars];
// ####################### Configuracion ########################
extern int purge;
extern int RSSILimit;
extern int SendTime;
extern int TimesSeen;    
// ################## Datos parseados ###################
int line = 0;
int train = 0;
int car = 0;
int people = 0;

int rssi = 0;
int lht = 0;
int reported = 0;
   
int state = 0;
int idx = 0;

char MAC[MAC_SIZE];
// ############### Direccion e informacion ##############
String address = "";
String information = "";
extern Data_t DataInformation[260];
// ################ Fecha y hora  ############
extern String date;
extern String hora;
extern String minuto;
extern String anio;
extern String mes;
extern String dia;
extern String t;
// ################ Funciones  ############

// ################ Lector e identificador de marcadores  ############
void recvWithStartEndMarkers() {
    boolean recvInProgress = false;
    int ndx = 0;
    char startMarker = '<';
    char configMarker = '[';
    char endMarker = '\n';
    char rc;
  
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                  //while(Serial.available())
                    //Serial.read();
                   ndx = 0;
                }
            }
            else {
                receivedChars[ndx] = '\0';
                recvInProgress = false;
                newData = true;
                //Serial.printf("%i\n",ndx); 
                //Serial.printf("((%i))",Serial.available()); 
                //Serial.printf("%s\n",receivedChars);             
            }
        }
        if (rc == startMarker) {
            recvInProgress = true;
            ConfigData = false;
        }
        if (rc == configMarker) {
            recvInProgress = true;
            ConfigData = true;
        }
        if (rc == '!') {
            SendingFlag = true;
            recvInProgress = false;
        }      
    }
}

// ################ Parseador de datos  ############
void parseData() {
    //[0|250|5|200<1|AA:BB:CC:DD:EE:FF|-100|50|25
    char * strtokIndx;

    if(ConfigData)
    {
      strtokIndx = strtok(tempChars,"|");
      line = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      train = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      car = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      people = atoi(strtokIndx); 

      //setAddress();
    }
    else
    {
      strtokIndx = strtok(tempChars, "|");
      state = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      idx = atoi(strtokIndx);
      
      strtokIndx = strtok(NULL, "|");
      strcpy(MAC, strtokIndx);
  
      strtokIndx = strtok(NULL, "|");
      rssi = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      lht = atoi(strtokIndx);

      strtokIndx = strtok(NULL, "|");
      reported = atoi(strtokIndx);
    }
}

// ################ Imprimir los datos parseados  ############
void showParsedData() {
    Serial.printf(FORMAT,lines[line],train,car,state?"ADD":"DEL",idx,MAC,rssi,lht,reported,people);
}

// ################ Generar direccion destino  ############
void SetAddress(void){
  address = "";
  address += "Linea/";
  address += line;//lines[line];
  address += "/Formacion/";
  address += train;
  address += "/Coche/";
  address += car;
  address += "/Dispositivos/";
}

// ################ Generar datos a enviar  ############
void SetInfo(){
    information = "";
    if(!(idx%2)){information += '|';}
    information += MAC;  
    information += '|';
    information += String(rssi);
    information += '|';
    information += String(lht);
    information += '|';
    information += String(reported);
    //information += '\0';
}

// ################ Enviar datos por Firebase ############
void SendParsedData(void){

  int maxVector;
  int k=1;

  Firebase.remove(address);
  Firebase.remove(address);
  
  if (idx%2)
  {
    maxVector = (idx+1)/2;
    DataInformation[idx].Data = "";
  }
  else{ maxVector = idx/2;}

  Firebase.setString(address+0+"/1/",String(date)+' '+String(t)+'|'+String(people)+'|'+String(idx));
  while(Firebase.failed())
  {
    Serial.println("Fallo de cabecera!");
    Firebase.setString(address+0+"/1/",String(date)+' '+String(t)+'|'+String(people)+'|'+String(idx));
  }
    
  for(int i=1; i <= idx; i+=2)
  {    
     Firebase.setString(address+k+"/1/",DataInformation[i-1].Data+DataInformation[i].Data); 
     while(Firebase.failed())
     {
      Serial.println("Fallo de dato!");
      Firebase.setString(address+k+"/1/",DataInformation[i-1].Data+DataInformation[i].Data);
     }
     k++;
  }

  /*
  for(int k=maxVector+1; k<=130;k++)
  {
    Firebase.remove(address+k);
  }
  */
  j++;
  
}

// ################ Enviar datos por Firebase BACKUP ############
void SendParsedData_2(void){

  for(int i=1; i<= idx; i++)
  {
    if(!((i-1)%20))
    {
      //Firebase.setString(address+0+"/0/","Contador de pasajeros");  
      Firebase.setString(address+0+"/"+j+"/",String(date)+' '+String(t)+'|'+String(people)+'|'+String(idx));
    }
     Firebase.setString(address+i+"/1/",DataInformation[i-1].Data); 
     if(Firebase.failed())
      Serial.println("Falle!");
  }
  
  for(int i=idx+1; i<=260;i++)
  {
    Firebase.remove(address+i);
  }
  j++;
}

// ################ Leer datos de Firebase ############
void ReadConfig(){
    digitalWrite(LED, !HIGH);
    String Config = "";
    //purge=Firebase.getInt("Purge Time");   // 150
    //RSSILimit=Firebase.getInt("RSSI");     // -90
    //SendTime=Firebase.getInt("Send Time");   // 180
    //TimesSeen=Firebase.getInt("Times Seen"); // 10
    Config += '[';
    Config += Firebase.getString("Configuracion");
    //Serial.printf("[%d|%i|%i|%i\n",purge,RSSILimit,SendTime,TimesSeen);
    Serial.println(Config);
}
