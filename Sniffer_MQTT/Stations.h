#define L0 "Invalido"
#define L1 "Roca"
#define L2 "San_Martin"
#define L3 "Sarmiento"
#define L4 "Mitre"
#define L5 "Belgrano_Sur"

#define FORMAT"Linea:%s|Formacion:%d|Coche:%d|Estado:%s|(%d)%s|RSSI:%d|LHT:%d|Reported:%d|Pasajeros:%d\r\n"

typedef enum{LINEA0,LINEA1,LINEA2,LINEA3,LINEA4,LINEA5}t_lines;

const char* lines[]={L0,L1,L2,L3,L4,L5};
