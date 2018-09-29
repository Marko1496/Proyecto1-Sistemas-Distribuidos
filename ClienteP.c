#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <string.h>
/* netbd.h es necesitada por la estructura hostent ;-) */

#define PORT 9999
/* El Puerto Abierto del nodo remoto */

#define MAXDATASIZE 2000
/* El número máximo de datos en bytes */

int main(int argc, char *argv[])
{
        int fd, numbytes;
        /* ficheros descriptores */

        char buf[MAXDATASIZE];
        /* en donde es almacenará el texto recibido */

        struct hostent *he;
        /* estructura que recibirá información sobre el nodo remoto */

        struct sockaddr_in server;
        /* información sobre la dirección del servidor */
        char message[2000], server_reply[2000];

        if (argc !=2) {
                /* esto es porque nuestro programa sólo necesitará un
                   argumento, (la IP) */
                printf("Uso: %s <Dirección IP>\n",argv[0]);
                exit(-1);
        }

        if ((he=gethostbyname(argv[1]))==NULL) {
                /* llamada a gethostbyname() */
                printf("gethostbyname() error\n");
                exit(-1);
        }

        if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1) {
                /* llamada a socket() */
                printf("socket() error\n");
                exit(-1);
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        /* htons() es necesaria nuevamente ;-o */
        server.sin_addr = *((struct in_addr *)he->h_addr);
        /*he->h_addr pasa la información de ``*he'' a "h_addr" */
        bzero(&(server.sin_zero),8);

        if(connect(fd, (struct sockaddr *)&server,
                   sizeof(struct sockaddr))==-1) {
                /* llamada a connect() */
                printf("connect() error\n");
                exit(-1);
        }

        while(1)
        {
                printf("Enter message : ");
                scanf("%s", message);

                //Send some data
                if( send(fd, message, strlen(message), 0) < 0)
                {
                        puts("Send failed");
                        return 1;
                }

                //Receive a reply from the server
                if( recv(fd, buf, MAXDATASIZE, 0) < 0)
                {
                        puts("recv failed");
                        break;
                }

                puts("Server reply :");
                puts(buf);
        }



        buf[numbytes]='\0';

        printf("Mensaje del Servidor: %s\n",buf);
        /* muestra el mensaje de bienvenida del servidor =) */

        close(fd); /* cerramos fd =) */

}
