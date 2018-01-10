
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql.h> // libreria que nos permite hacer el uso de 
#include <ctype.h> //las conexiones y consultas con MySQL 
#include <time.h>
#include <ctype.h>

#define BACKLOG 2 /* El número de conexiones permitidas */
#define MAXDATASIZE 1000 
#define RESPALDO 10

//Estructura que almacena todos los datos del Drive Test

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


/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : IsANumber
 **     Función     : Verificar si un caracter es un numero
 **     Parámetros  : cadena de caracteres
 **     Retorna     : 1 si es un numero, -1 si no
 ** ----------------------------------------------------------------------------
*/

int IsANumber(const char* numero)
{
    int i=0;

    while(numero[i] != 0)
    {
        if(!isdigit(numero[i]))
        {
          return -1;
        }
        i++;
    }
    return 1;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Iniciar_Socket
 **     Función     : Realiza configuraciones iniciales del socket
 **     Parámetros  : fd (Descriptor del socket), sin_size (Tamaño de la estructura sockaddr_in)
                      argv (Contiene el puerto de escucha)
 **     Retorna     : Estructura del servidor
 ** ----------------------------------------------------------------------------
*/

struct sockaddr_in Iniciar_Socket(int *fd, int *sin_size, const char* argv)
{
   struct sockaddr_in server; 
   /* para la Información de la dirección del servidor */

   /* A continuación la llamada a socket() */
   if (((*fd)=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  
      perror("error en socket()\n");
      exit(-1);
   }

   server.sin_family = AF_INET;         

   server.sin_port = htons(atoi(argv)); 
   /* ¿Recuerdas a htons() de la sección "Conversiones"? =) */

   server.sin_addr.s_addr = INADDR_ANY; 
   /* INADDR_ANY coloca nuestra dirección IP automáticamente */

   bzero(&(server.sin_zero),8); 
   /* escribimos ceros en el reto de la estructura */


   /* A continuación la llamada a bind() */
   if(bind((*fd),(struct sockaddr*)&server,
           sizeof(struct sockaddr))==-1) {
      perror("error en bind() \n");
      exit(-1);
   }     

   if(listen((*fd),BACKLOG) == -1) {  /* llamada a listen() */
      perror("error en listen()\n");
      exit(-1);
   }

   *sin_size=sizeof(struct sockaddr_in);
   
   return server;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : finish_with_error
 **     Función     : Guardar en un archivo el error generado y cerrar la conexion
 **     Parámetros  : Descriptor de la conexion conn
 **     Retorna     : -
 ** ----------------------------------------------------------------------------
*/

void finish_with_error(MYSQL *conn)
{
  fprintf(stderr, "%s\n", mysql_error(conn));
  mysql_close(conn);
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Guardar_DB
 **     Función     : Almacenar los valores del Drive Test en la base de datos
 **     Parámetros  : Estructura de Drive Test y nombre de la base de datos
 **     Retorna     : 1 si se realizo con exito, -1 si fallo
 ** ----------------------------------------------------------------------------
*/

int Guardar_DB(Info Dtest, const char* Tabla)
{
    
    MYSQL *conn; /* variable de conexión para MySQL */
    char *server = "localhost"; /*direccion del servidor 127.0.0.1, localhost o direccion ip */
    char *user = "root"; /*usuario para consultar la base de datos */
    char *password = "1414425"; /* contraseña para el usuario en cuestion */
    char *database = "tesis"; /*nombre de la base de datos a consultar */
    conn = mysql_init(NULL); /*inicializacion a nula la conexión */
    char query[1000];

    if(conn==NULL)
    {
        finish_with_error(conn);
        return -1;
    }

    /* conectar a la base de datos */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    { /* definir los parámetros de la conexión antes establecidos */
        finish_with_error(conn);
        return -1;
    }

    /* enviar consulta SQL */
    snprintf(query,1000,"INSERT INTO %s VALUES (%.6f,%.6f,%.4f,%u,%.4f"
        ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.0lf,%d,%d,%.0lf,%d,%d,%d,%d,"
        "%d,%d,%.0f,%.0f,%d,%d,%d,%d,%d,%.0f,%.0f,%d,%d,%d,%d,%d,%.0f,"
        "%.0f,%d,%d,%d,%d,%d,%.0f,%.0f,%d,%d,%d,%d,%d,%.0f,%.0f,%d,%d,"
        "%d,%d,%d,%.0f,%.0f)",Tabla,Dtest.latitude,Dtest.longitude,
        Dtest.HMS,Dtest.DMA,Dtest.speedOTG,Dtest.Llamadas_realizadas,
        Dtest.Llamadas_perdidas,Dtest.RSSI,Dtest.BER,Dtest.arfcn_bcch[0],
        Dtest.rxl[0],Dtest.rqx,Dtest.mcc[0],Dtest.mnc[0],Dtest.bsic[0],
        Dtest.cellid[0],Dtest.rla,Dtest.txp,Dtest.lac[0],Dtest.TA,
        Dtest.arfcn_bcch[1],Dtest.rxl[1],Dtest.mcc[1],Dtest.mnc[1],
        Dtest.bsic[1],Dtest.cellid[1],Dtest.lac[1],Dtest.arfcn_bcch[2],
        Dtest.rxl[2],Dtest.mcc[2],Dtest.mnc[2],Dtest.bsic[2],Dtest.cellid[2],
        Dtest.lac[2],Dtest.arfcn_bcch[3],Dtest.rxl[3],Dtest.mcc[3],
        Dtest.mnc[3],Dtest.bsic[3],Dtest.cellid[3],Dtest.lac[3],
        Dtest.arfcn_bcch[4],Dtest.rxl[4],Dtest.mcc[4],Dtest.mnc[4],
        Dtest.bsic[4],Dtest.cellid[4],Dtest.lac[4],Dtest.arfcn_bcch[5],
        Dtest.rxl[5],Dtest.mcc[5],Dtest.mnc[5],Dtest.bsic[5],Dtest.cellid[5],
        Dtest.lac[5],Dtest.arfcn_bcch[6],Dtest.rxl[6],Dtest.mcc[6],
        Dtest.mnc[6],Dtest.bsic[6],Dtest.cellid[6],Dtest.lac[6]);
    
    if (mysql_query(conn, query))
    { 
        finish_with_error(conn);
        return -1;
    }
    
    /* se libera la variable res y se cierra la conexión */
    mysql_close(conn);
    return 1;
}

/*
 ** ----------------------------------------------------------------------------
 **     Nombre      : Crear_DB
 **     Función     : Crear tabla en la base de datos
 **     Parámetros  : Nombre de la tabla en la base de datos
 **     Retorna     : 1 si fue exitoso, 0 si no
 ** ----------------------------------------------------------------------------
*/

int Crear_DB(const char* Tabla)
{    
    MYSQL *conn; /* variable de conexión para MySQL */
    char *server = "localhost"; /*direccion del servidor 127.0.0.1, 
                                localhost o direccion ip */
    char *user = "root"; /*usuario para consultar la base de datos */
    char *password = "1414425"; /* contraseña para el usuario en cuestion */
    char *database = "tesis"; /*nombre de la base de datos a consultar */
    conn = mysql_init(NULL); /*inicializacion a nula la conexión */
    char query[1000];

    if(conn==NULL)
    {
        finish_with_error(conn);
        return -1;
    }

    /* conectar a la base de datos */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    { /* definir los parámetros de la conexión antes establecidos */
        finish_with_error(conn);
        return -1;
    }

    /* enviar consulta SQL */
    snprintf(query, 1000, "create table %s (LATITUD FLOAT(9,4),LONGITUD"
        " FLOAT(9,4),HMS INT,DMA INT,SPEEDOTG FLOAT(6,2),CALL_MADE INT,"
        "CALL_LOST INT,RSSI INT,BER INT,ARFCN_BCCH_0 INT,RXL_0 INT,RQX_0"
        " INT,MCC_0 INT,MNC_0 INT,BSIC_0 INT,CELL_ID_0 INT,RLA_0 INT,TXP_0"
        " INT,LAC_0 INT,TA_0 INT,ARFCN_BCCH_1 INT,RXL_1 INT,MCC_1 INT,MNC_1"
        " INT,BSIC_1 INT,CELL_ID_1 INT,LAC_1 INT,ARFCN_BCCH_2 INT,RXL_2 INT,"
        "MCC_2 INT,MNC_2 INT,BSIC_2 INT,CELL_ID_2 INT,LAC_2 INT,ARFCN_BCCH_3"
        " INT,RXL_3 INT,MCC_3 INT,MNC_3 INT,BSIC_3 INT,CELL_ID_3 INT,LAC_3 "
        "INT,ARFCN_BCCH_4 INT,RXL_4 INT,MCC_4 INT,MNC_4 INT,BSIC_4 INT"
        ",CELL_ID_4 INT,LAC_4 INT,ARFCN_BCCH_5 INT,RXL_5 INT,MCC_5 INT,"
        "MNC_5 INT,BSIC_5 INT,CELL_ID_5 INT,LAC_5 INT,ARFCN_BCCH_6 INT,"
        "RXL_6 INT,MCC_6 INT,MNC_6 INT,BSIC_6 INT,CELL_ID_6 INT,LAC_6 INT);",
        Tabla);
 
    if (mysql_query(conn, query))
    { 
        finish_with_error(conn);
        return -1;
    }
    
    /* se libera la variable res y se cierra la conexión */
    mysql_close(conn);
    return 1;
}

int main(int argc, char const *argv[])
{

    if(argc != 3)
    {
        printf("son dos argumentos: ./Program Port DataBase\n");
        return -1;
    }
    if(IsANumber(argv[1])==-1)
    {
        printf("El segundo argumento es un numero\n");
        return -1;
    }

    int k = 1, contador = 1;
    Info Dtest;
    int fd, fd2 , j = 0 , numbytes; /* los ficheros descriptores */
          
    struct sockaddr_in server; 
    /* para la Información de la dirección del servidor */
    
    struct sockaddr_in client; 
    /* para la Información de la dirección del cliente */
    
    int sin_size;
    
    server = Iniciar_Socket(&fd, &sin_size, argv[1]);

    if(Crear_DB(argv[2]) != 1)
    {
        printf("Error al crear la base de datos\n");
        return -1;
    }

   /* A continuación la llamada a accept() */
  
    while(1)
    {
        if((fd2 = accept(fd,(struct sockaddr *)&client,
                          &sin_size))==-1) {
           printf("Con %d files\t",contador );
           perror("error en accept()\n");
           exit(-1);
        }
    
        if(k % RESPALDO == 0)
        {
           printf("contador va en %d\n",contador );
        }
          
        /* que mostrará la IP del cliente */
        //send(fd2,(void*)&msg,sizeof(Info),0); 
        /* que enviará el mensaje de bienvenida al cliente */
          
        for (int j = 0; j <= 9; j++)
        {
            if ((numbytes=recv(fd2,(void*)&Dtest,sizeof(Info),0)) == -1){  
                /* llamada a recv() */
                perror(" error Error \n");
                exit(-1);
            }

            if(Guardar_DB(Dtest,argv[2]) != 1)
            {
                printf("Error al guardar en la base de datos\n");
                return -1;
            }                      
        }
    
        contador++;
        k++;
        close(fd2);
    }
}
