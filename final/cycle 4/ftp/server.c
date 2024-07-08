#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 1030
#define BUFFER_SIZE 1024

void main() {
    int server_sock, client_sock;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    } else {
        printf("Successfully created socket\n");
    }

    memset(&serveraddr, '\0', sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("Error binding");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 10);
    printf("Server is listening on port %d\n", PORT);

    addr_size = sizeof(clientaddr);
    client_sock = accept(server_sock, (struct sockaddr *)&clientaddr, &addr_size);
    if (client_sock < 0) {
        perror("Error accepting connection");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char filename[100];
    FILE *fptr;

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (strcmp(buffer, "PUT") == 0) {
            bzero(buffer, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);
            strcpy(filename, buffer);

            bzero(buffer, BUFFER_SIZE);
            fptr = fopen(filename, "w");
            if (!fptr) {
                perror("Error opening file");
                continue;
            }

            while (1) {
                bzero(buffer, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE, 0);
                if (strcmp(buffer, "END$") == 0) {
                    break;
                }
                fprintf(fptr, "%s", buffer);
            }
            printf("File %s received\n", filename);
            fclose(fptr);
        } else if (strcmp(buffer, "GET") == 0) {
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "%d", getpid());
            send(client_sock, buffer, BUFFER_SIZE, 0);

            bzero(buffer, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);
            strcpy(filename, buffer);

            bzero(buffer, BUFFER_SIZE);
            fptr = fopen(filename, "r");
            if (!fptr) {
                printf("The given file does not exist\n");
                strcpy(buffer, "404");
                send(client_sock, buffer, BUFFER_SIZE, 0);
            } else {
                strcpy(buffer, "200");
                send(client_sock, buffer, BUFFER_SIZE, 0);

                while (fgets(buffer, BUFFER_SIZE, fptr)) {
                    send(client_sock, buffer, BUFFER_SIZE, 0);
                    bzero(buffer, BUFFER_SIZE);
                }
                strcpy(buffer, "END$");
                send(client_sock, buffer, BUFFER_SIZE, 0);
                fclose(fptr);
                printf("File %s sent\n", filename);
            }
        } else if (strcmp(buffer, "BYE") == 0) {
            close(client_sock);
            close(server_sock);
            exit(0);
        }
    }
}
