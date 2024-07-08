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
    int client_sock;
    struct sockaddr_in serveraddr;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    } else {
        printf("Successfully created socket\n");
    }

    memset(&serveraddr, '\0', sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char filename[100];
    int choice;
    FILE *fptr;

    while (1) {
        printf("\n1. Upload a file (PUT)\n2. Download a file (GET)\n3. Exit (BYE)\nEnter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                bzero(buffer, BUFFER_SIZE);
                strcpy(buffer, "PUT");
                send(client_sock, buffer, BUFFER_SIZE, 0);

                printf("Enter the filename: ");
                scanf("%s", filename);

                fptr = fopen(filename, "r");
                if (!fptr) {
                    printf("The given file does not exist\n");
                } else {
                    bzero(buffer, BUFFER_SIZE);
                    strcpy(buffer, filename);
                    send(client_sock, buffer, BUFFER_SIZE, 0);

                    while (fgets(buffer, BUFFER_SIZE, fptr)) {
                        send(client_sock, buffer, BUFFER_SIZE, 0);
                        bzero(buffer, BUFFER_SIZE);
                    }
                    bzero(buffer, BUFFER_SIZE);
                    strcpy(buffer, "END$");
                    send(client_sock, buffer, BUFFER_SIZE, 0);
                    fclose(fptr);
                    printf("File %s sent\n", filename);
                }
                break;

            case 2:
                bzero(buffer, BUFFER_SIZE);
                strcpy(buffer, "GET");
                send(client_sock, buffer, BUFFER_SIZE, 0);

                bzero(buffer, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE, 0);
                printf("Process ID: %s\n", buffer);

                printf("Enter the filename: ");
                scanf("%s", filename);

                bzero(buffer, BUFFER_SIZE);
                strcpy(buffer, filename);
                send(client_sock, buffer, BUFFER_SIZE, 0);

                bzero(buffer, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE, 0);

                if (strcmp(buffer, "404") == 0) {
                    printf("File does not exist on the remote server\n");
                } else if (strcmp(buffer, "200") == 0) {
                    fptr = fopen(filename, "w");
                    if (!fptr) {
                        perror("Error opening file");
                        break;
                    }
                    while (1) {
                        bzero(buffer, BUFFER_SIZE);
                        recv(client_sock, buffer, BUFFER_SIZE, 0);
                        if (strcmp(buffer, "END$") == 0)
                            break;
                        fprintf(fptr, "%s", buffer);
                        printf("%s",buffer);
                    }
                    fclose(fptr);
                    printf("File %s received\n", filename);
                }
                break;

            case 3:
                bzero(buffer, BUFFER_SIZE);
                strcpy(buffer, "BYE");
                send(client_sock, buffer, BUFFER_SIZE, 0);
                close(client_sock);
                printf("Disconnected from server\n");
                exit(EXIT_SUCCESS);

            default:
                printf("Invalid choice, please try again\n");
        }
    }
}
