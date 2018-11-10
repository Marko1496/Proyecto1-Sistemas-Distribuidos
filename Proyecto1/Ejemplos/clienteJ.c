#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h> // for close
#include <math.h>

int main(int argc, char *argv[])
{
        FILE *fichero;
        char cadena[256];
        char *resultado;
        char *ip;
        int numPaginas;
        double media;
        double probabilidad;
        int sock;
        int count = 0;
        double tiempoEspera = 0;
        struct sockaddr_in server;
        char message[1000], server_reply[2000];
        char dest[500];
        double numeroRand = rand () % 100 -50;;   // Este está entreo 20 y 30

        //Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
                printf("Could not create socket");
        }
        puts("Socket created");

        fichero = fopen("archivoDeConfiguracion.txt", "rt");

        if (fichero == NULL) {
                printf("Error: No se ha podido crear el fichero fichero1.txt");
        } else {
                resultado = fgets(cadena, 256, fichero);
                fclose(fichero);

                char *token = strtok(resultado, ",");
                while (resultado != NULL) {
                        if(count == 0) {
                                ip = resultado;
                        }else if(count ==1) {
                                numPaginas = atoi(resultado);
                        }
                        else if(count ==2) {
                                media = atof(resultado);
                        }
                        else if(count ==3) {
                                probabilidad = atof(resultado);
                        }
                        resultado = strtok(NULL, ",");
                        count++;
                }
                tiempoEspera = -1*media*log(numeroRand);

                if( tiempoEspera < 0)
                        tiempoEspera = tiempoEspera* -1;

                int i;
                for(i = 2; i >= 0; i--)
                {
                        printf("%i\n",i); // Escribe el número actual de la 'cuenta regresiva'
                        sleep(tiempoEspera); // Espera un segundo
                }

                printf( "El Tiempo de espera del nodo es de: %f\n", tiempoEspera );
                server.sin_addr.s_addr = inet_addr(ip);
                server.sin_family = AF_INET;
                server.sin_port = htons( 8888 );

                //Connect to remote server
                if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
                {
                        perror("connect failed. Error");
                        return 1;
                }

                puts("Connected\n");
                count = 0;

                //keep communicating with server
                while(1)
                {
                        sleep(1);
                        if(count == 0) {
                                //strcpy(dest, "pag ");
                                //strcat(dest, numPaginas);
                                //strcat(dest, "\n");
                                //send(sock, dest, strlen(dest),0);
                                send(sock, "HOLA\n", strlen("HOLA"),0);
                                count = 1;
                        }
                        //printf("Enter message : ");
                        //scanf("%s", message);

                        //Send some data
                        if( send(sock, message, strlen(message), 0) < 0)
                        {
                                puts("Send failed");
                                return 1;
                        }

                        //Receive a reply from the server
                        if( recv(sock, server_reply, 2000, 0) < 0)
                        {
                                puts("recv failed");
                                break;
                        }
                        puts("Server reply :");
                        puts(server_reply);
                }
        }
        close(sock);
        return 0;
}
