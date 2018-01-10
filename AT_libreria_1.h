#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

typedef struct Info_t 
{
    uint16_t arfcn_bcch[7];     // Número de canal de frecuencia absoluta del BCCH
    uint8_t  rxl[7];            // Nivel de potencia de la señal recibida
    uint8_t  rqx;               // Calidad de la señal recibida
    uint16_t mcc[7];            // Código de país
    uint8_t  mnc[7];            // Código de la red
    uint8_t bsic[7];            // Código de identidad de la estacion base
    double cellid[7];           // Identificador de la celda
    uint8_t  rla;               // Nivel mínimo de transmisión para acceder a la red 
    uint8_t  txp;               // Máximo nivel de transmisión en el CCCH
    double    lac[7];           // Área de ubicación
    uint8_t   TA;               // Avance temporal
        
    uint8_t RSSI;               // Nivel de la señal
    uint8_t BER;                // Bit error rate

    unsigned int Llamadas_perdidas;     //contador de llamadas perdidas
    unsigned int Llamadas_realizadas;   //contador de llamadas realizadas

    double latitude;            //Longitud de la coordenada
    double longitude;           //Latitud de la coordenada
    double HMS;                 //hora minuto segundo
    uint DMA;                   //dia mes y año
    double speedOTG;            // speed over ground

}Info;

void separador(char token, uint *salida, SoftwareSerial *my_Serial);

void separador(char token, uint16_t *salida, SoftwareSerial *my_Serial);

void separador(char token, uint8_t *salida, SoftwareSerial *my_Serial);

void separador(char token, double *salida, SoftwareSerial *my_Serial);

void separadorHEX(char token, double *salida, SoftwareSerial *my_Serial);

void separador(char token, SoftwareSerial *my_Serial);

void separador_signo(char * salida, SoftwareSerial *my_Serial);

void begin_myserial();

void Send_WiFi(WiFiClient client);

void DegToDec(char signo, double *posicion);

uint8_t Verificar_Gps();

int8_t Probar_GPS();

void get_data();

void get_CSQ();

int Llamar(char* numero);

void iniciar_variables();

void reiniciar_RSSI_BER();

void Recibir_Serial(char* response, unsigned long timeout, uint16_t *x);

uint16_t Buscar_inicio(char* header, char* response);

int8_t get_CENG();

uint16_t separador(char token, uint *salida, uint16_t i, char* response);

uint16_t separador(char token, uint16_t *salida, uint16_t i, char* response);

uint16_t separador(char token, uint8_t *salida, uint16_t i, char* response);

uint16_t separador(char token, double *salida, uint16_t i, char* response);

uint16_t separadorHEX(char token, double *salida, uint16_t i, char* response);

uint16_t separador(char token, uint16_t i, char* response);

uint16_t separador_signo(char * salida, uint16_t i, char* response);

uint8_t sendATcommand(char* ComandoAT, char* Respuesta_esperada, unsigned long timeout, char* response);

int size(char* contador);

char* comparador(char* respuesta, char* respuesta_esperada);

char* comparador(char* respuesta, char* respuesta_esperada, uint16_t *w);

void Enviar_comando(char* ComandoAT);

unsigned int verificar_llamada();

void Trancar();

uint8_t verificar_CSQ();

void begin_myserial_Datos();

void ftp_get();
