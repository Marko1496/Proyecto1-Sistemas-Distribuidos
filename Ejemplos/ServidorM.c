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

#define TRUE   1
#define FALSE  0
#define PORT 8888
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
char* itoa(int val, int base);
int main(int argc, char *argv[])
{
        int opt = TRUE;
        int nuevo = -1;
        int cantidadPag = 5;
        struct pagina paginas[cantidadPag];
        int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
        int max_sd;
        struct sockaddr_in address;
        char buffer[1025]; //data buffer of 1K
        inicializarPaginas(paginas, cantidadPag);
        //set of socket descriptors
        fd_set readfds;

        //a message
        char *message = "ECHO Daemon v1.0 \r\n";

        //initialise all client_socket[] to 0 so not checked
        for (i = 0; i < max_clients; i++)
        {
                client_socket[i] = 0;
        }

        //create a master socket
        if( (master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
                perror("socket failed");
                exit(EXIT_FAILURE);
        }

        //set master socket to allow multiple connections , this is just a good habit, it will work without this
        if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
        {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        //type of socket created
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );

        //bind the socket to localhost port 8888
        if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }
        printf("Listener on port %d \n", PORT);

        //try to specify maximum of 3 pending connections for the master socket
        if (listen(master_socket, 3) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        //accept the incoming connection
        addrlen = sizeof(address);
        puts("Waiting for connections ...");

        while(TRUE)
        {
                //clear the socket set
                FD_ZERO(&readfds);

                //add master socket to set
                FD_SET(master_socket, &readfds);
                max_sd = master_socket;

                //add child sockets to set
                for ( i = 0; i < max_clients; i++)
                {
                        //socket descriptor
                        sd = client_socket[i];

                        //if valid socket descriptor then add to read list
                        if(sd > 0)
                                FD_SET( sd, &readfds);

                        //highest file descriptor number, need it for the select function
                        if(sd > max_sd)
                                max_sd = sd;
                }

                //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
                activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

                if ((activity < 0) && (errno!=EINTR))
                {
                        printf("select error");
                }

                //If something happened on the master socket , then its an incoming connection
                if (FD_ISSET(master_socket, &readfds))
                {

                        if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
                        {
                                perror("accept");
                                exit(EXIT_FAILURE);
                        }

                        //inform user of socket number - used in send and receive commands
                        printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                        //send new connection greeting message
                        /*if( send(new_socket, message, strlen(message), 0) != strlen(message) )
                           {
                                perror("send");
                           }

                           puts("Welcome message sent successfully");*/

                        //add new socket to array of sockets
                        for (i = 0; i < max_clients; i++)
                        {
                                //if position is empty
                                if( client_socket[i] == 0 )
                                {
                                        client_socket[i] = new_socket;
                                        printf("Adding to list of sockets as %d\n", i);

                                        break;
                                }
                        }

                        for(i=0; i < cantidadPag; i++) {
                                if(strcmp(paginas[i].ip, "VACIO") == 0) {
                                        strcpy(paginas[i].ip,inet_ntoa(address.sin_addr));
                                        paginas[i].socket = new_socket;
                                        nuevo = i;
                                        break;
                                }
                        }
                }

                //else its some IO operation on some other socket :)
                for (i = 0; i < max_clients; i++)
                {
                        sd = client_socket[i];

                        if (FD_ISSET( sd, &readfds))
                        {
                                //Check if it was for closing , and also read the incoming message
                                if ((valread = read( sd, buffer, 1024)) == 0)
                                {
                                        //Somebody disconnected , get his details and print
                                        getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                                        printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                                        //Close the socket and mark as 0 in list for reuse
                                        close( sd );
                                        client_socket[i] = 0;
                                }

                                //Echo back the message that came in
                                else
                                {
                                        if(nuevo == -1) {
                                                getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                                                int accion;
                                                int numero_pagina;
                                                int count = 0;
                                                int init_size = strlen(buffer);
                                                char delim[] = "-";

                                                char *ptr = strtok(buffer, delim);

                                                while(ptr != NULL)
                                                {
                                                        if(count==0) {
                                                                accion = atoi(ptr);
                                                        }else if (count == 1) {
                                                                numero_pagina = atoi(ptr);
                                                        }
                                                        count++;
                                                        ptr = strtok(NULL, delim);
                                                }
                                                //printf("%i-%i\n",accion,numero_pagina);

                                                if(accion == 1) {
                                                        system("clear");
                                                        printf("Ip guardada: %s\n", inet_ntoa(address.sin_addr));
                                                        strcpy(paginas[numero_pagina].ip,inet_ntoa(address.sin_addr));
                                                        imprimirPaginas(paginas, cantidadPag);
                                                        send(sd, "1", strlen("1"), 0 );
                                                }
                                                else if(accion == 2) {
                                                        send(sd, "2", strlen("2"), 0 );
                                                }
                                                else{
                                                        send(sd, "3", strlen("3"), 0 );
                                                }
                                        }
                                        else{
                                                char cadena [30];
                                                toString(paginas[nuevo],cadena);
                                                send(sd, cadena, strlen(cadena), 0 );
                                                nuevo = -1;
                                        }

                                }
                        }
                }
        }

        return 0;
}
char* itoa(int val, int base){

        static char buf[1025] = {0};

        int i = 30;

        for(; val && i; --i, val /= base)

                buf[i] = "0123456789abcdef"[val % base];

        return &buf[i+1];

}
