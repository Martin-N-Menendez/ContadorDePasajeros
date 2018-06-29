
String date = "";
String hora = "";
String minuto = "";
String anio = "";
String mes = "";
String dia = "";
String t = "";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void ConfigTime(){
 
    date = "";
    t = "";
    hora = "";
    minuto = "";
    anio = "";
    mes = "";
    dia = "";
    
    timeClient.update();
    uint64_t epochTime =  timeClient.getEpochTime();
    time_t local, utc;
    utc = epochTime;
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -240};   //UTC - 6 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);
    
    anio += year(local);  
    if(month(local) < 10)  mes += "0";
    mes += month(local); 
    if(day(local) < 10)  dia += "0";
    dia += day(local);
    
    date += anio + '-' + mes + '-' + dia;
    
    if(hour(local) < 10) hora += "0";
    hora += hour(local);
    if(minute(local) < 10) minuto += "0";
    minuto += minute(local); 
    
    t = hora + ":" + minuto;
}

