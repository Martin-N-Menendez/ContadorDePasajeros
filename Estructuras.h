#ifndef ESPRESENCE_ESPRESENCE_H
#define ESPRESENCE_ESPRESENCE_H

typedef struct {
    const char *ssid;
    const char *pwd;
} ap_t;

typedef struct {
    String mac;
    int8_t rssi;         // -128 a 127
    uint32_t ms;         // 0 a 4,294,967,295
    uint16_t reported;   // 0 a 65535 
} clt_device_t;

void checkList();
void printlist();
void onProbeRequest(const WiFiEventSoftAPModeProbeRequestReceived &evt);
void wifiConnect();
void initSerial();
void initOTA();
void initWiFi();
void initMQTT();
void mqttConnect();
uint32_t getUptimeSecs();
String prettyBytes(uint32_t bytes);
String macToString(const uint8 mac[6]);
void HeaderToMQTT();

void myDataCb(String& topic, String& data);
void myPublishedCb();
void myDisconnectedCb();
void myConnectedCb();


#endif //ESPRESENCE_ESPRESENCE_H
