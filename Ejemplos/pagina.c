/**
    Handle multiple socket connections with select and fd_set on Linux
 */

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

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
        printf("Ip de la pagina: %s\n",p_pagina->ip );
        printf("Socket del dueño de la pagina: %i\n",p_pagina->socket );
        printf("Version del dueño de la pagina: %i\n",p_pagina->version );
        printf("No de la pagina: %i\n",p_pagina->noPag );
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
void toString(pagina pagina, char * cadena){
        char salida [30];
        char socket [30];
        char version [30];
        strcpy(salida, pagina.ip);
        strcat(salida, "-");
        sprintf(socket,"%i", pagina.socket);
        sprintf(version,"%i", pagina.version);
        strcat(salida, socket);
        strcat(salida, "-");
        strcat(salida, version);
        strcpy(cadena, salida);
}
int main(int argc, char *argv[])
{
        int cantidadPag = 10;
        char cadena [30];
        struct pagina paginas[cantidadPag];
        inicializarPaginas(paginas, cantidadPag);
        imprimirPaginas(paginas, cantidadPag);
        strcpy(paginas[2].ip,"192.168.0.8");
        imprimirPaginas(paginas, cantidadPag);
        toString(paginas[2], cadena);
        printf("Prueba: %s\n", cadena);
        return 0;
};
