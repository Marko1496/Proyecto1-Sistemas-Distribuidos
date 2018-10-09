/*
    C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <stdlib.h>
#include <time.h>    // time()
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h> // for close
#include <math.h>

typedef struct pagina pagina;
struct pagina
{
        char ip[20];
        int socket;
        int version;
        int noPag;                     // Asigna un nuevo valor a A                       // Asigna un nuevo valor a B
        int dueno;
};
void imprimirPagina(pagina *p_pagina){
        printf("---------------------------\n" );
        printf("Ip dueno: %s. Socket: %i. Version: %i. # pagina: %i.",p_pagina->ip,p_pagina->socket,p_pagina->version,p_pagina->noPag );
        if(p_pagina->dueno == 1) {
                printf("Es dueno de la pagina.\n" );
        }
        else{
                printf("No es dueno de la pagina.\n" );
        }
        printf("\n");
};
void imprimirPaginas(pagina *paginas, int total_paginas){
        struct pagina *p_pagina;
        int num_pagina;
        p_pagina = paginas; /* apunta al primer elemento del array */
        for( num_pagina=0; num_pagina<total_paginas; num_pagina++ ) {

                imprimirPagina(p_pagina);
                p_pagina++;
        }
}
void inicializarPaginas(pagina *paginas, int total_paginas){
        struct pagina *p_pagina;
        int num_pagina;
        p_pagina = paginas; /* apunta al primer elemento del array */
        for( num_pagina=0; num_pagina<total_paginas; num_pagina++ ) {

                strcpy(p_pagina->ip,"VACIO");
                p_pagina->socket = 0;
                p_pagina->version = 0;
                p_pagina->noPag = num_pagina+1;
                p_pagina->dueno = 0;
                p_pagina++;
        }
}
int main(int argc, char *argv[])
{
        int sock, instruccion, pagina, cantidad_de_paginas;
        int cont = 0;
        double media;
        double probabilidad;
        double tiempoEspera;
        char *ip_servidor;
        struct sockaddr_in server;
        char message[1025], server_reply[1025];
        FILE *fichero;
        char cadena[256];
        char *contenido;
        char delim[] = ",";

        double numeroRand = rand () % 20;   // Este está entreo 20 y 30

        fichero = fopen("archivoDeConfiguracion.txt", "rt");
        if (fichero == NULL) {
                printf("Error: No se ha podido crear el fichero fichero1.txt");
        } else {
                //Se obtiene contenido del arcivo
                contenido = fgets(cadena, 256, fichero);
                fclose(fichero);
                char *ptr = strtok(contenido, delim);
                //Se lee el contenido y se inicializan las respectivas variables
                while(ptr != NULL)
                {
                        if(cont==0) {
                                ip_servidor = ptr;
                        }else if (cont == 1) {
                                cantidad_de_paginas = atoi(ptr);
                        }else if (cont == 2) {
                                media = atof(ptr);
                        }else if (cont == 3) {
                                probabilidad = atof(ptr);
                        }
                        cont++;
                        ptr = strtok(NULL, delim);
                }
                printf("Ip del servidor: %s\n", ip_servidor);
                printf("Cantidad de paginas: %i\n", cantidad_de_paginas);
                printf("Media: %f\n", media);
                printf("Probabilidad: %f\n", probabilidad);

                //Tiempo de espera
                tiempoEspera = -1*media*log(numeroRand);
                if( tiempoEspera < 0)
                        tiempoEspera = tiempoEspera* -1;
                printf("tiempoespera: %f\n", tiempoEspera);

                //Se inicializan paginas
                struct pagina paginas[cantidad_de_paginas];
                inicializarPaginas(paginas, cantidad_de_paginas);

                //Create socket
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == -1)
                {
                        printf("Could not create socket");
                }
                puts("Socket created");

                server.sin_addr.s_addr = inet_addr(ip_servidor);
                server.sin_family = AF_INET;
                server.sin_port = htons( 8888 );

                //Connect to remote server
                if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
                {
                        perror("connect failed. Error");
                        return 1;
                }

                puts("Connected\n");
                srand((unsigned int)time(NULL));
                //keep communicating with server
                while(1)
                {
                        pagina = rand() %cantidad_de_paginas+0;
                        char instruccion_s[10];
                        char pagina_s[10];
                        float c = rand() / (float) RAND_MAX;

                        float prob_lec = probabilidad;

                        float prob_esc = 1 - prob_lec;

                        if (c < prob_lec)
                        {
                                instruccion = 2;
                        }
                        else
                        {
                                instruccion = 1;
                        }
                        memset(server_reply,0,1025);
                        //puts(itoa (instruccion,10) );
                        sleep(tiempoEspera);
                        //memset(message, 0, 1025);
                        //message[0] = '1';
                        //message[1] = '\n';
                        //message[2] = '\0';
                        //-1*media*logN(random)

                        //Send some data
                        if(instruccion == 1) {
                                //No tiene la pagina pero es el dueno
                                if(strcmp(paginas[pagina].ip, "VACIO") == 0 && paginas[pagina].dueno == 0) {
                                        sprintf(instruccion_s,"%i", 1);
                                        sprintf(pagina_s,"%i", pagina);
                                        strcat(instruccion_s,"-");
                                        strcat(instruccion_s,pagina_s);
                                        strcpy(message, instruccion_s);
                                        printf("No se es dueno y se quiere leer pero no se tiene la pag. Piediendo la pag...\n");
                                        printf("%s\n", message);
                                        if( send(sock, message, strlen(message), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }else if(strcmp(paginas[pagina].ip, "VACIO") != 0 && paginas[pagina].dueno == 0) {
                                        printf("Se tiene la pag %i y no se es dueno. La version de la pagina es: %i\n",paginas[pagina].noPag, paginas[pagina].version);
                                        if( send(sock, "message", strlen("message"), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }else if(paginas[pagina].dueno == 1) {
                                        printf("Se tiene la pag %i y se es dueno. La version de la pagina es: %i\n",paginas[pagina].noPag, paginas[pagina].version);
                                        if( send(sock, "message", strlen("message"), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }
                        }
                        else{
                                if(strcmp(paginas[pagina].ip, "VACIO") == 0 && paginas[pagina].dueno == 0) {
                                        sprintf(instruccion_s,"%i", 2);
                                        sprintf(pagina_s,"%i", pagina);
                                        strcat(instruccion_s,"-");
                                        strcat(instruccion_s,pagina_s);
                                        strcpy(message, instruccion_s);
                                        printf("No se es dueno y se quiere escribir pero no se tiene la pag. Piediendo la pag...\n");
                                        printf("%s\n", message);
                                        if( send(sock, message, strlen(message), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }else if(strcmp(paginas[pagina].ip, "VACIO") != 0 && paginas[pagina].dueno == 0) {
                                        printf("Se quiere escribir y se tiene la pag %i y no se es dueno. La version de la pagina es: %i\n",paginas[pagina].noPag, paginas[pagina].version);
                                        if( send(sock, "message", strlen("message"), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }else if(paginas[pagina].dueno == 1) {
                                        printf("Se quiere escribir y se tiene la pag %i y se es dueno. La version de la pagina es: %i\n",paginas[pagina].noPag, paginas[pagina].version);
                                        if( send(sock, "message", strlen("message"), 0) < 0)
                                        {
                                                puts("Send failed");
                                                return 1;
                                        }
                                }
                        }


                        //Receive a reply from the server
                        if( recv(sock, server_reply, 1025, 0) < 0)
                        {
                                puts("recv failed");
                                break;
                        }
                        else{
                                if(server_reply[0] == '1' || server_reply[0] == '0') {
                                        puts("Se imprime la pagina:");
                                        puts(server_reply);
                                        int accion;
                                        int numPagina;
                                        char *ip;
                                        int puerto;
                                        int version;
                                        int count = 0;
                                        char delim[] = "-";

                                        char *ptr = strtok(server_reply, delim);

                                        while(ptr != NULL)
                                        {
                                                if(count==0) {
                                                        accion = atoi(ptr);
                                                }else if (count == 1) {
                                                        numPagina = atoi(ptr);
                                                }else if (count == 2) {
                                                        ip = ptr;
                                                }else if (count == 3) {
                                                        puerto = atoi(ptr);
                                                }else if (count == 4) {
                                                        version = atoi(ptr);
                                                }
                                                count++;
                                                ptr = strtok(NULL, delim);
                                        }
                                        strcpy(paginas[numPagina].ip,ip);
                                        paginas[numPagina].socket = puerto;
                                        paginas[numPagina].version = version;
                                        paginas[numPagina].dueno = accion;
                                        printf("Se recibe la pag %i y su dueno es %s. La version de la pagina es: %i\n", numPagina+1, ip, version);
                                }
                                else if(server_reply[0] == '2') {
                                        puts("Se imprime la pagina:");
                                        puts(server_reply);
                                        int accion;
                                        int numPagina;
                                        char *ip;
                                        int puerto;
                                        int version;
                                        int count = 0;
                                        char delim[] = "-";

                                        char *ptr = strtok(server_reply, delim);

                                        while(ptr != NULL)
                                        {
                                                if(count==0) {
                                                        accion = atoi(ptr);
                                                }else if (count == 1) {
                                                        numPagina = atoi(ptr);
                                                }else if (count == 2) {
                                                        ip = ptr;
                                                }else if (count == 3) {
                                                        puerto = atoi(ptr);
                                                }else if (count == 4) {
                                                        version = atoi(ptr);
                                                }
                                                count++;
                                                ptr = strtok(NULL, delim);
                                        }
                                        strcpy(paginas[numPagina].ip,ip);
                                        paginas[numPagina].socket = puerto;
                                        paginas[numPagina].version = version;
                                        paginas[numPagina].dueno = accion-2;
                                        printf("Se recibe la pag %i y ahora soy su dueno. La nueva version de la pagina es: %i\n", numPagina+1, version);
                                }else {
                                        puts("Se imprime la pagina:");
                                        puts(server_reply);
                                }
                        }
                }

                close(sock);
        }
        return 0;
}
