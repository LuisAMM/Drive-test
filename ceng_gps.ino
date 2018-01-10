#include "AT_libreria_1.h"

const char* ssid     = "luis-pc";   //Red a la que el D1mini se conectará
const char* password = "W6T4fAkP";  //Contraseña de la red

const char* ip_servidor = "10.42.0.1";  //dirección IP del servidor

uint16_t Port = 51002;                  //Puerto del servidor

IPAddress staticIP(10,42,0,2);          //Direccion IP estatica del equipo
IPAddress gateway(10,42,0,1);           //Gateway de la red
IPAddress subnet(255,255,255,0);        //Mascara de la red

unsigned long previous;                 //Temporizador
unsigned long antes;                    //Temporizador

WiFiClient client;                      //Clase que permite conectarse por WIFI con un servidor
int i;  
unsigned int aux=4;

void setup() {

  ESP.wdtDisable();                     //Se desabilita el WDT

  Serial.begin(9600);
  while(!Serial) ESP.wdtFeed();

  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  
  //nos conectamos a la red
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  //verificamos el estado de la conexion
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(F("."));
  }

  Serial.println(F(""));
  Serial.println(F("WiFi connected"));  

  antes=millis();
  //tiempo de espera para encender el modulo SIM900
  while(millis()-antes<=10000)
  {
    ESP.wdtFeed();
  }

  begin_myserial();
  iniciar_variables();
  
  Probar_GPS();
  if(Serial.available()>0)
    Serial.read();
}

void loop() {
  
  //Se realiza la conexion con el servidor
  if(!client.connect(ip_servidor, Port)){
    ESP.wdtFeed();
    Serial.println(F(""));
    delay(1000);
    return;
  }
  //Se obtienen los datos del GPS
  get_data();

  //Se verifica si el tiempo de la llamada termino ó si la llamada se cayo
  if(millis()-previous >= 120000 || (aux-verificar_llamada())!=0)
  {
    Trancar();            //se cuelga la llamada
    reiniciar_RSSI_BER(); //se reinician los valores de rssi y ber
    Llamar("*627568");    //se llama al numero de prueba
    aux=verificar_llamada();  //se le asigna a aux el valor de las llamadas perdidas
    previous=millis();        //se reinicia el temporizador
  }

  //se toman las 10 medidas de ingenieria, las de 
  //calidad y se envia el reporte al servidor
  for(i=0;i<=9;i++)
  {
    if(get_CENG()!=1)
      continue;
    ESP.wdtFeed();
    if(verificar_CSQ()==1)
      get_CSQ();
    Send_WiFi(client);
  }
  //se termina la conexion con el servidor
  client.stop();

}
