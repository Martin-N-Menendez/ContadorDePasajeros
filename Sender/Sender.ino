#include "Sender.h"
#include "Stations.h"

int j = 1;
// ####################### Buffers ########################
extern const int numChars;
extern char receivedChars[];
extern char tempChars[];
// ################## Datos parseados ###################
extern int line,train,car,people,rssi,lht,reported,state,idx;
extern char MAC[MAC_SIZE];
// ####################### Flags ########################
boolean newData = false;
boolean ConfigData = false;
boolean SendingFlag = false;
// ############### Direccion e informacion ##############
extern String address;
extern String information;
struct Data_t{
  String Data = "";
  uint8_t State;
};
Data_t DataInformation[260];
// ############### Interrupcion ##############
volatile int toggle;
int contador = 0;
int purge = 150;
int RSSILimit = -90;
int SendTime = 180;
int TimesSeen = 10;    
// ################ Fecha y hora  ############
extern String date;
extern String hora;
extern String minuto;
extern String anio;
extern String mes;
extern String dia;
extern String t;
//WiFiUDP ntpUDP;
extern NTPClient timeClient;
uint32_t before = 0;
// ################ Prototipos ############
void inline handler (void);
void SaveData();
// ################ Interrupcion ############
void inline handler (void){
  toggle = (toggle == 1) ? 0 : 1;
  if(contador == 10)
  {
    digitalWrite(LED, toggle);
    contador = 0;
  } 
  timer0_write(ESP.getCycleCount() + timer0_preload * my_delay); // 
  contador++;
  recvWithStartEndMarkers();
}

// ################ Setup ############    
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); // Turn OFF LED to start with
  noInterrupts();
  
  Serial.begin(BAUDRATE);
  #if(DEBUG_MODE)
  Serial.println("Booting");
  Serial.println(ESP.getFreeSketchSpace());
  #endif
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    #if(DEBUG_MODE)
    Serial.println(".");
    #endif
    delay(5000);
    //ESP.restart();
  }
  #if(DEBUG_MODE)
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  #endif

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Serial.setRxBufferSize(BUFFER_SIZE);

  timer0_isr_init();
  timer0_attachInterrupt(handler);
  timer0_write(ESP.getCycleCount() + timer0_preload * my_delay);
  interrupts();
   
  timeClient.begin();
  ConfigTime();
}

// ################ Loop ############
void loop() {
  digitalWrite(LED, !LOW);
  uint32_t now = millis();

   //Firebase.remove("Linea:Roca");
   //Firebase.remove("Linea:Mitre");
   //Firebase.remove("Linea:San Martin");  

   /*
   ConfigTime();
   Serial.print(date);
   Serial.print(" ");
   Serial.println(t);
   delay(20*1000);
   */
   
   if(SendingFlag == true && line != 0)
   {  
     before = millis(); 
     ConfigTime(); 
     now = millis();     
     Serial.printf("Configurar hora:%i ms\r\n",(now-before));
     
     Serial.print(date);
     Serial.print(" ");
     Serial.println(t);
     before = millis(); 
     SetAddress();   
     now = millis();   
     Serial.printf("Setear direccion:%i ms\r\n",(now-before));
     before = millis(); 
     SendParsedData();  
     now = millis();
     Serial.printf("Enviar a FB:%i ms\r\n",(now-before));
     before = millis(); 
     ReadConfig(); 
     now = millis();
     Serial.printf("Leer config:%i ms\r\n",(now-before));
     SendingFlag = false;     
   }
    
   if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    //showParsedData();
    SetInfo();
    SaveData();
    newData = false;
   }
}

void SaveData(){
  if(ConfigData == true) return;
  
  DataInformation[idx-1].Data = "";  
  DataInformation[idx-1].Data += information;
}
