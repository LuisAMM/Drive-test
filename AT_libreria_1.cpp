#include "AT_libreria_1.h"

#define TIEMPO 1  
#define BUFFER_RX 400

SoftwareSerial mySerial_GPS(D5,D6,false,100);   // RX, TX

char response[BUFFER_RX]={};    //Buffer que almacena las respuesta del SIM900 en modo de ingenieria
char response_CSQ[50]={};       //Buffer que almacena la respuesta de la funcion get_CSQ
Info *Dtest;                    //Apuntador a estructura principal  
uint8_t flag_CSQ = 0;           //Bandera que le indica al programa que debe tomar las medidas de calidad de la senal

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Size
 **     Función     : Obtener el tamaño de una cadena de caracteres generico.
 **     Parámetros  : Cadena de caracteres generico.
 **     Retorna     : El tamaño de la cadena de caracteres.
 ** ----------------------------------------------------------------------------
*/

int size(char* contador)
{
    int i =0;
    while(contador[i]!='\0')
    {
        i++;
    }
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : verificar_llamada
 **     Función     : Verificar el valor de la variable de llamada perdida ubicada en la estructura de datos.
 **     Parámetros  :  -
 **     Retorna     : El numero de llamadas perdidas.
 ** ----------------------------------------------------------------------------
*/

unsigned int verificar_llamada()
{
    return Dtest->Llamadas_perdidas;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Comparador
 **     Función     : Comparar dos cadenas de caracteres.
 **     Parámetros  : dos cadenas de caracteres
 **     Retorna     : NULL si una no contiene a la otra y un apuntador a la respuesta esperada si la contiene
 ** ----------------------------------------------------------------------------
*/

char* comparador(char* respuesta, char* respuesta_esperada)
{
   int i=0,k;
   int j=0,l;
   char aux3='z';
   k=size(respuesta);
   l=size(respuesta_esperada);

   if(k<l)
    return NULL;
      
   while(aux3!='\0' && respuesta_esperada[j]!='\0')
   {
      aux3=respuesta[i];
      if(aux3==respuesta_esperada[j])
      {
         i++;
         j++;
      }
      else
      {
         i++;
         j=0;
      }
   }
   if(j==l)
    return respuesta_esperada;
   else
    return NULL;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Comparador
 **     Función     : Comparar dos cadenas de caracteres y devolver la posicion donde termina la respuesta esperada
 **     Parámetros  : dos cadenas de caracteres y un apuntador a un entero sin signo de 16 bits
 **     Retorna     : NULL si una no contiene a la otra y un apuntador a la respuesta esperada si la contiene.
 ** ----------------------------------------------------------------------------
*/

char* comparador(char* respuesta, char* respuesta_esperada, uint16_t *w)
{
   int i=0,k;
   int j=0,l;
   char aux3='z';
   k=size(respuesta);
   l=size(respuesta_esperada);

   if(k<l)
    return NULL;
      
   while(aux3!='\0' && respuesta_esperada[j]!='\0')
   {
      aux3=respuesta[i];
      if(aux3==respuesta_esperada[j])
      {
         i++;
         j++;
      }
      else
      {
         i++;
         j=0;
      }
      ESP.wdtFeed();
   }
   *w=i;
   if(j==l)
    return respuesta_esperada;
   else
    return NULL;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Enviar_comando
 **     Función     : Enviar comando AT al SIM900
 **     Parámetros  : Cadena de caracteres que contiene el comando AT
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void Enviar_comando(char* ComandoAT)
{
	Serial.println(ComandoAT);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Recibir_Serial
 **     Función     : Leer del buffer del serial y almacenarlo en la cadena de caracteres response
 **     Parámetros  : cadena de caracteres que sirve para almacenar, temporizador, posicion en el arreglo
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void Recibir_Serial(char* response, unsigned long timeout, uint16_t *x)
{
	unsigned long antes=millis();
  //Ciclo de espera mientras llega un valor por serial o se acaba el temporizador.
    while(Serial.available()<=0 && (antes-millis()) <= timeout)
        ESP.wdtFeed();

    while(Serial.available() > 0 && *x < BUFFER_RX){
        response[*x]=Serial.read();
        (*x)++;
        delay(TIEMPO);
    }
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Buscar_inicio
 **     Función     : Buscar el inicio del mensaje.
 **     Parámetros  : Header: encabezado del mensaje almacenado en response.
 **     Retorna     : Posicion en la que termina el encabezado
 ** ----------------------------------------------------------------------------
*/

uint16_t Buscar_inicio(char* header, char* response)
{
	uint16_t x=0;
	if(comparador(response,header,&x)!=NULL)
		return x;
	else
    return -1;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : get_CENG
 **     Función     : Obtener y almacenar en la estructura los datos de ingenieria del sistema
 **     Parámetros  : -
 **     Retorna     : si se realizó con exito 1, si no repsponde el equipo retorna 0, si no consigue el inicio -1
 ** ----------------------------------------------------------------------------
*/

int8_t get_CENG()
{
    uint16_t j=1;
    uint16_t i=0,aux;
    //Se envia el comando AT+CENG? para tener los parametros de ingeniería
    //Si no se recibe OK, descartar la medición
    if(sendATcommand("AT+CENG?","OK",1000,response) != 1)
    	return 0;
    //Buscar el encabezado +CENG:0," 
    i = Buscar_inicio("+CENG:0,\"",response);
    //Si la posición 0 ó 1  descartar medición
    aux=i;
    if(aux==0 || aux==-1)
    {
        return -1;
    }   
    //si la posicion es del tamaño del buffer, descartar medicion
    if(i==BUFFER_RX-1)
    	return -1;

    //se almacenan los valores de la celda que sirve al movil en la estructura
    i=separador(',' , &(Dtest->arfcn_bcch[0]), i, response);
    i=separador(',' , &(Dtest->rxl[0]), i, response);
    i=separador(',' , &(Dtest->rqx), i, response);
    i=separador(',' , &(Dtest->mcc[0]), i, response);
    i=separador(',' , &(Dtest->mnc[0]), i, response);
    i=separador(',' , &(Dtest->bsic[0]), i, response);
    i=separadorHEX(',' , &(Dtest->cellid[0]), i, response);
    i=separador(',' , &(Dtest->rla), i, response);
    i=separador(',' , &(Dtest->txp), i, response);
    i=separadorHEX(',' , &(Dtest->lac[0]), i, response);
    i=separador('\"' , &(Dtest->TA), i, response);

    while(j<=6)
    {   //valores de las celdas vecinas
        i = separador('\"', i, response);
        i = separador(',' , &(Dtest->arfcn_bcch[j]), i, response);
        i = separador(',' , &(Dtest->rxl[j]), i, response);
        i = separador(',' , &(Dtest->bsic[j]), i, response);
        i = separadorHEX(',' , &(Dtest->cellid[j]), i, response);
        i = separador(',' , &(Dtest->mcc[j]), i, response);
        i = separador(',' , &(Dtest->mnc[j]), i, response);
        i = separadorHEX('\"' , &(Dtest->lac[j]), i, response);
        j++;
    }
    return 1;
}

		/*SEPARADORES DE INGENIERIA*/

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje de ingeniería que se almacenan en una variable de 32 bits.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      i (posicion que se esta leyendo), response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador(char token, uint *salida, uint16_t i, char* response)
{
    int j=0;
    char aux[25];
    char aux2;
    do
    {
        aux2=response[i];
        aux[j]=aux2;
        j++;
        i++;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = atoi(aux);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje de ingeniería que se almacenan en una variable de 16 bits.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      i (posicion que se esta leyendo), response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador(char token, uint16_t *salida, uint16_t i, char* response)
{
    int j=0;
    char aux[25];
    char aux2;
    do
    {
        aux2=response[i];
        aux[j]=aux2;
        j++;
        i++;
    }
    while(aux2 != token);
    aux[j]='\0';
    *salida = atoi(aux);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje de ingeniería que se almacenan en una variable de 8 bits.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      i (posicion que se esta leyendo), response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador(char token, uint8_t *salida, uint16_t i, char* response)
{
    int j=0;
    char aux[25];
    char aux2;
    do
    {
        aux2=response[i];
        aux[j]=aux2;
        j++;
        i++;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = atoi(aux);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje de ingeniería que se almacenan en un double.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      i (posicion que se esta leyendo), response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador(char token, double *salida, uint16_t i, char* response)
{
    int j=0;
    char aux[25];
    char aux2;
    do
    {
        aux2=response[i];
        aux[j]=aux2;
        j++;
        i++;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = atof(aux);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separadorHEX
 **     Función     : Obtener los datos del mensaje de ingeniería que se encuentran en Hexadecimal.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      i (posicion que se esta leyendo), response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separadorHEX(char token, double *salida, uint16_t i, char* response)
{
    int j=0;
    char aux[25];
    char aux2;
    do
    {
        aux2=response[i];
        aux[j]=aux2;
        j++;
        i++;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = (double)strtol(aux,NULL, 16);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obviar espacios sin valor
 **     Parámetros  : token (delimitador de los valores), i (posicion que se esta leyendo), 
                      response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador(char token, uint16_t i, char* response)
{
    char aux; 
    int j=0;
    char aux2;
    do
    {
        aux2=response[i];
        i++;
    }
    while(aux2 != token);
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador_signo
 **     Función     : obtener un caracter del mensaje.
 **     Parámetros  : salida (Variable en la que se almacenara el signo) i (posicion que se esta leyendo), 
                      response (cadena de caracteres que contiene toda la respuesta)
 **     Retorna     : Posicion del proximo valor.
 ** ----------------------------------------------------------------------------
*/

uint16_t separador_signo(char * salida, uint16_t i, char* response)
{
    *salida=response[i];
    i++;
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Llamar
 **     Función     : Llamar a un numero
 **     Parámetros  : Cadena de caracteres (numero)
 **     Retorna     : 1 si la llamada es exitosa, 0 si no es exitosa.
 ** ----------------------------------------------------------------------------
*/

int Llamar(char* numero)
{
    char comando[40]={};
    uint8_t i = 0;
    sprintf(comando,"ATD%s;",numero);
    if(sendATcommand(comando,"OK",1000,response)==1)
	  {
		  i=1;
	  }
    (Dtest->Llamadas_realizadas)++;
    return i;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Trancar
 **     Función     : Finalizar llamada
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void Trancar()
{
	sendATcommand("ATH","OK",1000, response);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Verificar_CSQ
 **     Función     : obtener el valor de la bandera flag_CSQ
 **     Parámetros  : -
 **     Retorna     : valor de la bandera flag_CSQ
 ** ----------------------------------------------------------------------------
*/

uint8_t verificar_CSQ()
{
	return flag_CSQ;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : SendATcommand
 **     Función     : Enviar comando AT y verificar que el mensaje de respuesta concuerde con lo esperado
 **     Parámetros  : comandoAT, Respuesta_esperada, temporizador, cadena de caracteres que almacenará la respuesta
 **     Retorna     : 1 si se recibio la respuesta esperada, 0 si no.
 ** ----------------------------------------------------------------------------
*/

uint8_t sendATcommand(char* ComandoAT, char* Respuesta_esperada, unsigned long timeout, char* response)
{
  //Se envía el comando AT
	Enviar_comando(ComandoAT);
  //Empieza el temporizador
	long unsigned antes=millis();
	uint8_t answer=0,flag=0;
	uint16_t x = 0;
  //se vacia el buffer de almacenamiento
	memset(response,'\0',BUFFER_RX);

	do
	{
		Recibir_Serial(response,1000,&x);
		//comparamos si la respuesta recibida contiene a NO CARRIER
    if(comparador(response,"NO CARRIER")!=NULL && flag==0)
		{
      flag=1;
			(Dtest->Llamadas_perdidas)++;
		}
    //comparamos si la respuesta recibida contiene a MO CONNECTED
		if(comparador(response,"MO CONNECTED")!=NULL)
		{
			flag_CSQ = 1;
		}
    //comparamos si la respuesta recibida contiene a la respuesta esperada
		if(comparador(response,Respuesta_esperada)!=NULL)
		{
			answer=1;
		}
    ESP.wdtFeed();
	}while(answer==0 && ((millis()-antes)<=timeout));
  //se coloca delimitador \0	
	if(x!=400)
		response[x]=0;
	return answer;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Iniciar variables
 **     Función     : inicializar las variables y reservar la memoria de la estructura
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void iniciar_variables()
{
    Dtest = (Info *)malloc(sizeof(Info));
    Dtest->HMS = 0;
    Dtest->latitude = 0;
    Dtest->longitude = 0;
    Dtest->speedOTG = 0;
    Dtest->DMA = 0;    
    Dtest->rqx = 0;
    Dtest->rla = 0;
    Dtest->txp = 0;
    Dtest->TA = 0;
    
    Dtest->RSSI = 0;
    Dtest->BER = 0;
    
    Dtest->Llamadas_perdidas = 0;
    Dtest->Llamadas_realizadas = 0;
    for (int i = 0; i < 7; i++)
    {
        Dtest->arfcn_bcch[i] = 0;
        Dtest->rxl[i] = 0;
        Dtest->bsic[i] = 0;
        Dtest->cellid[i] = 0;
        Dtest->mcc[i] = 0;
        Dtest->mnc[i] = 0;
        Dtest->lac[i] = 0;
    }
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : reiniciar_RSSI_BER
 **     Función     : Reiniciar en 0 los valores de BER, RSSI y de la bandera flag_CSQ
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void reiniciar_RSSI_BER()
{
	flag_CSQ = 0;
    Dtest->RSSI = 0;
    Dtest->BER = 0;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : begin_myserial
 **     Función     : Fijar tamaño del buffer de recepcion serial.
 **     Parámetros  : Enviar mensajes de configuracion tanto al GPS como al SIM900 para llamadas
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void begin_myserial()
{
    Serial.setRxBufferSize(BUFFER_RX);
    mySerial_GPS.begin(9600);
    delay(10);
    //comando del GPS para utilizar los mensajes $GPGGA
    mySerial_GPS.print(F("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"));
    //comando del GPS para configurar su tasa de transmision a 19200 baudios
    mySerial_GPS.print(F("$PMTK251,19200*22\r\n"));
    mySerial_GPS.begin(19200);
    mySerial_GPS.enableRx(false);
    delay(10);
    while(mySerial_GPS.available()>0) mySerial_GPS.read();
    
    //activamos el modo de ingenieria
    sendATcommand("AT+CENG=1,1","OK",1000, response);
    //activamos los mensajes de conexion de la llamada
    sendATcommand("AT+MORING=1","OK",1000, response);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : begin_myserial_Datos
 **     Función     : Fijar tamaño del buffer de recepcion serial.
 **     Parámetros  : Enviar mensajes de configuracion tanto al GPS como al SIM900 para datos
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void begin_myserial_Datos()
{
    Serial.setRxBufferSize(BUFFER_RX);
    mySerial_GPS.begin(9600);
    delay(10);
    //comando del GPS para utilizar los mensajes $GPGGA
    mySerial_GPS.print(F("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"));
    //comando del GPS para configurar su tasa de transmision a 19200 baudios
    mySerial_GPS.print(F("$PMTK251,19200*22\r\n"));
    mySerial_GPS.begin(19200);
    mySerial_GPS.enableRx(false);
    delay(10);
    while(mySerial_GPS.available()>0) mySerial_GPS.read();
    //Se configura las conexionex FTP
    sendATcommand("AT+CENG=1,1","OK",1000, response);
    sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK", 1000,response);
    sendATcommand("AT+SAPBR=3,1,\"APN\",\"internet.movistar.ve\"","OK", 1000,response);
    sendATcommand("AT+SAPBR=1,1","OK",1000,response);
    sendATcommand("AT+FTPCID=1","OK",1000,response);
    sendATcommand("AT+FTPSERV=\"speedtest.tele2.net\"","OK",1000,response);
    sendATcommand("AT+FTPUN=\"anonymous\"","OK",1000,response);
    sendATcommand("AT+FTPPW=\"1414425\"","OK",1000,response);
    sendATcommand("AT+FTPGETNAME=\"1KB.zip\"","OK",1000,response);
    sendATcommand("AT+FTPGETPATH=\"/\"","OK",1000,response);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : ftp_get
 **     Función     : solicitar al servidor FTP un archivo
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void ftp_get()
{
    int j=1;
    sendATcommand("AT+FTPGET=1","+FTPGET:1,1",20000,response);
    if(sendATcommand("AT+FTPGET=2,1024","+FTPGET=2,1024",5000,response)==0)
    {
        while(Serial.available())
        {
/*            response[0]=Serial.read();
            if(response[0]=='O')
                response[1]=Serial.read();
                    if(response[1]='K')
                    {
                        Serial.println(F("bien"));
                        break;
                    }
*/          Serial.print((char)Serial.read());
        }
    }
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Send_WiFi
 **     Función     : Enviar estructura via WiFi
 **     Parámetros  : Objeto WiFiClient client
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void Send_WiFi(WiFiClient client)
{
    client.write((uint8_t*)Dtest,sizeof(*Dtest));
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : DegToDec
 **     Función     : Convertir valor de posicion de grados a decimal
 **     Parámetros  : signo y posicion en grados
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void DegToDec(char signo, double *posicion)
{
    double aux,aux2;
    int aux3;
    aux3 = *posicion/100;
    aux= *posicion - aux3*100;

    aux2 = (double)aux3 + aux/60.0; 

    if(signo == 'W' || signo == 'S')
        *posicion = -aux2;
    else
        *posicion = aux2;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Verificar_GPS
 **     Función     : Verificar que el GPS se encuentre ubicado y funcionando
 **     Parámetros  : -
 **     Retorna     : 1 si esta ubicado, 0 si no
 ** ----------------------------------------------------------------------------
*/

uint8_t Verificar_Gps()
{  
  	mySerial_GPS.enableRx(true);
 	 
  	int x=millis();
  	char aux,aux2;
 	 //Ciclo de espera mientras llega un valor por serial o se acaba el temporizador.
  	while(mySerial_GPS.available()<=0 && (millis()-x)<=3000)
  	{
 	   ESP.wdtFeed();
  	}
  	delay(10);
 	 
  	x=millis();
  	do
  	{
    	if(mySerial_GPS.available()!=0)
    	{
            aux=(char)mySerial_GPS.read();
            //Buscamos el encabezado
            if( aux=='$' )
            {
              //saltamos los primeros 2 valores
            	separador(',',&mySerial_GPS);
            	separador(',',&mySerial_GPS);
              //Almacenamos el valor del estado del GPS en aux2
            	separador_signo(&aux2,&mySerial_GPS);
            	break;
            }
    	}
    	else
      	break;
  	}
  	while(mySerial_GPS.available() && (millis()-x)<=5000);
  	mySerial_GPS.enableRx(false); 
  	while(mySerial_GPS.available()>0) mySerial_GPS.read();
  	if(aux2=='A')
    	return 1;
  	else
    	return 0;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Probar_GPS
 **     Función     : Verificar que el GPS funcione correctamente y esperar hasta que funcione
 **     Parámetros  : -
 **     Retorna     : 1 si funciona correctamente
 ** ----------------------------------------------------------------------------
*/

int8_t Probar_GPS()
{
    while(mySerial_GPS.available()>0) mySerial_GPS.read();
    uint8_t respuesta=0;

    respuesta = Verificar_Gps();
    
    while(respuesta != 1)
    {
        Serial.print(F("."));
        respuesta = Verificar_Gps();
        
    }
    return 1;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : get_data
 **     Función     : Obtener los valores relevantes del GPS y almacenarlos en la estructura
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void get_data()
{
  mySerial_GPS.enableRx(true);
  
  int x=millis();
  char aux,aux2;
  uint8_t i=0;

  //Ciclo de espera mientras llega un valor por serial o se acaba el temporizador.
  while(mySerial_GPS.available()<=0 && (millis()-x)<=5000)
  {
    ESP.wdtFeed();
  }
  delay(10);
  x=millis();
  do
  {
    if(mySerial_GPS.available()!=0)
    {
      aux=(char)mySerial_GPS.read();
      if( aux=='$' || i != 0 )//buscamos el encabezado
      {
        //Almacenar los valores del GPS en la estructura de datos
        separador(',',&mySerial_GPS);
        separador(',',&(Dtest->HMS),&mySerial_GPS);
        separador(',',&mySerial_GPS);
        separador(',',&(Dtest->latitude),&mySerial_GPS);
        separador_signo(&aux2,&mySerial_GPS);
        DegToDec(aux2,&(Dtest->latitude));
        separador(',',&(Dtest->longitude),&mySerial_GPS);
        separador_signo(&aux2,&mySerial_GPS);
        DegToDec(aux2,&(Dtest->longitude));
        separador(',',&(Dtest->speedOTG),&mySerial_GPS);
        separador(',',&mySerial_GPS);
        separador(',',&(Dtest->DMA),&mySerial_GPS);
        break;
      }
    }
    else
      break;
  }
  while(mySerial_GPS.available() && (millis()-x)<=5000);
  mySerial_GPS.enableRx(false);
  while(mySerial_GPS.available()) mySerial_GPS.read();
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : get_CSQ
 **     Función     : Obtener y almacenar en la estructura los valores de RSSI y BER
 **     Parámetros  : -
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void get_CSQ()
{
	uint8_t i=0;
	memset(response_CSQ,'\0',50);

    sendATcommand("AT+CSQ","OK",1000,response_CSQ);

    i=Buscar_inicio("+CSQ: ",response_CSQ);
    i=separador(',',&(Dtest->RSSI),i, response_CSQ);
    i=separador('\r',&(Dtest->BER),i, response_CSQ);
}

	/*SEPARADORES DE GPS*/

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje del GPS que se almacenan en una variable de 32 bits sin signo.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      my_Serial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador(char token, uint *salida, SoftwareSerial *my_Serial)
{
    int j=0;
    char aux[25];
    char aux2;

    do
    {
        if(my_Serial->available()>0)
        {
            delay(TIEMPO);
            aux2=(char)my_Serial->read();
            aux[j]=aux2;
            j++;
        }
        else 
            break;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = atoi(aux);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje del GPS que se almacenan en una variable de 16 bits sin signo.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      my_Serial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador(char token, uint16_t *salida, SoftwareSerial *my_Serial)
{
    int j=0;
    char aux[25];
    char aux2;

    do
    {
        if(my_Serial->available()>0)
        {
            delay(TIEMPO);
            aux2=(char)my_Serial->read();
            aux[j]=aux2;
            j++;
        }
        else 
            break;
    }
    while(aux2 != token);
    aux[j]='\0';
    *salida = atoi(aux);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje del GPS que se almacenan en una variable de 8 bits sin signo.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      my_Serial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador(char token, uint8_t *salida, SoftwareSerial *my_Serial)
{
   int j=0;
    char aux[25];
    char aux2;

    do
    {
        if(my_Serial->available()>0)
        {
            delay(TIEMPO);
            aux2=(char)my_Serial->read();
            aux[j]=aux2;
            j++;
        }
        else 
            break;
    }
    while(aux2 != token);
    aux[j]='\0';
    *salida = atoi(aux);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obtener los datos del mensaje del GPS que se almacenan en un double.
 **     Parámetros  : token (delimitador de los valores), salida (variable en la que se almacenara el valor),
                      my_Serial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador(char token, double *salida, SoftwareSerial *my_Serial)
{
    int j=0;
    char aux[25];
    char aux2;

    do
    {
        if(my_Serial->available()>0)
        {
            delay(TIEMPO);
            aux2=(char)my_Serial->read();
            aux[j]=aux2;
            j++;
        }
        else 
            break;
    }
    while(aux2 != token);
    aux[j]='\0';

    *salida = atof(aux);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador
 **     Función     : Obviar espacios sin valor
 **     Parámetros  : token (delimitador de los valores), mySerial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador(char token, SoftwareSerial *my_Serial)
{
    char aux; 
    do
    {
        if(my_Serial->available()>0)
        {
            delay(TIEMPO);
            aux = my_Serial->read();
        }
        else 
            break;
    }
    while(aux != token);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : separador_signo
 **     Función     : Almacenar el valor del signo 
 **     Parámetros  : Salida (variable en la que se almacenara el valor), 
                      mySerial (Objeto que recibe la informacion del GPS)
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void separador_signo(char * salida, SoftwareSerial *my_Serial)
{
    if(my_Serial->available()>0)
    {
        delay(TIEMPO);
        *salida = (char)my_Serial->read();
    }
    if(my_Serial->available()>0)
    {
        delay(TIEMPO);
        my_Serial->read();
    }
}
