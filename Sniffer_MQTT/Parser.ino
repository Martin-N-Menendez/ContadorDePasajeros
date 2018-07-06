
#define PURGETIME 150000
#define MINRSSI -99
#define SENDTIME 120
#define TIMES_SEEN 10

int purge = PURGETIME;
int RSSILimit = MINRSSI;
int SendTime = SENDTIME;
int TimesSeen = TIMES_SEEN;

/*
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static int ndx = 0;
    char startMarker = '[';
    char endMarker = '\n';
    char rc;

    if(Serial.available() >= BUFFER_SIZE-1)
  { 
    while(Serial.available())
      Serial.read();
    recvInProgress = false;
    ndx = 0;
  }
  
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= BUFFER_IN) {
                    ndx = 0;
                }
            }
            else {
                receivedChars[ndx] = '\0';
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData(void){
    char * strtokIndx;
    
    strtokIndx = strtok(tempChars,"|");
    purge = atoi(strtokIndx);

    strtokIndx = strtok(NULL, "|");
    RSSILimit = atoi(strtokIndx);

    strtokIndx = strtok(NULL, "|");
    SendTime = atoi(strtokIndx);
  
    strtokIndx = strtok(NULL, "|");
    TimesSeen = atoi(strtokIndx);
}

void showParsedData() {
    Serial.printf("[%i|%i|%i|%i\n",purge,RSSILimit,SendTime,TimesSeen);
}
*/
