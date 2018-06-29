

void showClient(uint64_t now){
#if(CLIENT_DEBUG)
  for (int u = 0; u < clients_known_count; u++) {
    Serial.printf("%4d ", u + 1); // Show client number
    Serial.print("C ");
    Serial.print(formatMac1(clients_known[u].station));
    Serial.print(" ==> ");
    Serial.print(formatMac1(clients_known[u].ap));
    Serial.print(" RSSI ");
    Serial.print(clients_known[u].rssi);
    //Serial.printf(" %im", calculateDistance(clients_known[u].rssi));
    Serial.print(" Seen: ");
    Serial.print((now - clients_known[u].lastDiscoveredTime));
    Serial.print(" Reported: ");
    Serial.println(clients_known[u].reported);
  }
#endif
}

void showBeacon(uint64_t now){
#if(BEACON_DEBUG)
  for (int u = 0; u < aps_known_count; u++) {
    Serial.printf( "%4d ", u + 1); // Show beacon number
    Serial.print("B ");
    Serial.print(formatMac1(aps_known[u].bssid));
    Serial.print(" RSSI ");
    Serial.print(aps_known[u].rssi);
    //Serial.printf(" %im", calculateDistance(aps_known[u].rssi));
    Serial.print(" Seen: ");
    Serial.print((now - aps_known[u].lastDiscoveredTime));
    Serial.print(" Reported: ");
    Serial.println(aps_known[u].reported);
  }
#endif
}

void showProbe(uint64_t now){
#if(PROBE_DEBUG)
  for (int u = 0; u < probes_known_count; u++) {
    Serial.printf( "%4d ", u + 1); // Show probe number
    Serial.print("P ");
    Serial.print(formatMac1(probes_known[u].station));
    //Serial.print(" ==> ");
    //Serial.print(formatMac1(probes_known[u].ap));
    //Serial.print(" RSSI ");
    Serial.printf(" %d", probes_known[u].rssi);
    //Serial.printf(" %im",calculateDistance(probes_known[u].rssi));
    Serial.printf(" %d ", (now - probes_known[u].lastDiscoveredTime));
    //Serial.print(" Reported: ");
    Serial.println(probes_known[u].reported);
  }
#endif
}

void showInformation(uint64_t now){
#if(SENDER_MODE)
state = 1;
for (int u = 0; u < probes_known_count; u++) {
  uint64_t lht = (now - probes_known[u].lastDiscoveredTime);
  if(!u){ Serial.printf("[%i|%i|%i|%i\n",LINE,TRAIN,CAR,people);delay(150);}
  Serial.printf("<%i|",state);
  Serial.printf("%i|",u+1);
  Serial.print(formatMac1(probes_known[u].station));
  //Serial.printf("|%i|%i|%i\n",probes_known[u].rssi,lht,probes_known[u].reported);
  Serial.print('|');
  Serial.print(probes_known[u].rssi);
  Serial.print('|');
  Serial.printf("%i",lht);
  Serial.print('|');
  Serial.println(probes_known[u].reported);
  delay(150);
}
  Serial.printf("%c",'!');
#endif
}

void showRandom(uint64_t now){
#if(RANDOM_MODE)
state = 1;

uint16_t aleatorio = random(1,260);

for(int j=0; j<aleatorio; j++){
  if(!j){ Serial.printf("[%i|%i|%i|%i\n",LINE,TRAIN,CAR,random(0,aleatorio));delay(150);}
  Serial.printf("<%i|%03i|",state,j+1);
  //Serial.printf("%03i",j+1);
  Serial.print(String(j+1,DEC)+":"+"AA"+":"+"BB"+":"+"CC"+":"+"DD"+":"+"EE");
  Serial.printf("|%i|%i|%03i\n",-10,30,j+1);
  delay(150);
}
  Serial.printf("%c",'!');
#endif
}

void showDevices() {
  uint64_t now = millis() / 1000;
  
#if(DEBUG_MODE)
  Serial.println("\r\n------------Base de datos de dispositivos------------------");
  Serial.printf("%4d Clients + %4d Devices + %4d Probes = %4d Devices\n", clients_known_count, aps_known_count, probes_known_count, clients_known_count + aps_known_count + probes_known_count); // show count
  Serial.printf("%4d Smartphones\n",people);
#endif

  showClient(now);
  showBeacon(now);
  showProbe(now);
  showInformation(now);
  showRandom(now);
}
