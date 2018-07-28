#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <String.h>
/*################## Informacion ################*/
const char *VERSION = "1.3.1";
/*################## Trenes #####################*/
#define L0 "Invalido"
#define L1 "Roca"
#define L2 "San_Martin"
#define L3 "Sarmiento"
#define L4 "Mitre"
#define L5 "Belgrano_Sur"

typedef enum{LINEA0,LINEA1,LINEA2,LINEA3,LINEA4,LINEA5}t_lines;

const char* lines[]={L0,L1,L2,L3,L4,L5};
/*################## Configuracion ################*/
ADC_MODE(ADC_VCC);                            // Pin a sensar (Conectar A0 con 3V para medir 3.3V)
#define SERIAL_SET_DEBUG_OUTPUT  false        // Activar modo debug
const uint16_t MAX_DEVICES = 200;             // Maxima cantidad de dispositivos a almacenar
const int8_t MIN_RSSI = -99;                  // Minima RSSI que debe tener un dispositivo para ser almacenado
const uint32_t LIST_TIMEOUT = 150*1000;       // Maxima tolerancia (segundos) antes de eliminar un dispositivo
const uint16_t REPORTED = 50;                 // Minima cantidad de reportes para ser considerado pasajero
const uint16_t SENDTIME = 10*60;              // Cantidad de segundos para enviar por MQTT
#define JBUFFER (MAX_DEVICES * 40)+15         // Tamaño del buffer para el Json
#define LINE LINEA2                           // Linea de Trenes Argentinos (ver Trenes)
#define TRAIN 1                               // Numero de formacion
#define CAR 5                                 // Numero de coche (1a7 o 1a9 segun linea)

/*################## WiFi ################*/
const char *ESP_NAME = "Contador de pasajeros"; // Host name
const char *AP_SSID = "GICSAFe";                // Nombre del AP
const char *AP_PASSWORD = "GICSAFeMOS";         // Contraseña del AP
const ap_t AP_LIST[] = {                        // Lista de conexiones a probar
        {"Telecentro-cb70", "VWN52Y4ETZ2Q"},    // Martin casa
        {"PruebaTBA", "pruebaTBA"},             // Martin celular
        {"BCYL-Invitados" , "bcyl2016"},        // Trenes Argentinos
        {"WiFi-Arnet-3b37", "33B3FF74EE"},      // Pablo casa
        {"alutenberg", "contador2018"},         // Ariel celular
        {"pgomez", "contador2018"},             // Pablo celular
        {"WT19a", "contador2018"}               // Pablo celular 2
};
/*################## MQTT ################*/
const char* MQTT_HOST = "191.239.243.244";                               // Direccion del broker
//const char* MQTT_HOST = "linsse.com.ar";                             
#define MQTT_PORT  1883                                                  // Puerto del broker   
String Address = "/cdp/"+String(lines[LINE])+"/F_"+TRAIN+"/C_"+CAR+"/";
const char* MQTT_OUT_TOPIC = Address.c_str();                            // Topico del broker                 
//const char* MQTT_OUT_TOPIC = "cdp/test";                               // Topico del broker
/*################## Timer ################*/
#define day  86400000 // 86400000 milisegundos por dia 
#define hour  3600000 // 3600000 milisegundos en una hora
#define minute  60000 // 60000 millisegundos en un minuto
#define second   1000 // 1000 millisegundos en un segundo

#endif //CONFIGURACION_H