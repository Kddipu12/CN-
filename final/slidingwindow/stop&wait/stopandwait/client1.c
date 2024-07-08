#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Added for close function

int main(){
    int serversocket, frames, timer;
    char clientbuffer[1024], serverbuffer[1024];
    struct sockaddr_in serveraddr;

    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serversocket < 0){
        perror("Socket not created");
        exit(0);
    }
    else {
        printf("Socket created\n");  // Replaced print with printf
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(8080);

    if(connect(serversocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        perror("Connection not created");
        exit(0);
    }
    else {
        printf("Connected with server\n");  // Replaced print with printf
    }

    printf("Enter the total frames: ");
    scanf("%d", &frames);
    if(send(serversocket, &frames, sizeof(frames), 0) < 0){  // Corrected send function call
        perror("Unable to send the frame count");
    }

    while(frames > 0){
        printf("Enter the frame message: ");
        bzero(clientbuffer, 1024);
        scanf("%s", clientbuffer);

        x:
        timer = 10;
        if(send(serversocket, clientbuffer, strlen(clientbuffer), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        y:
        bzero(serverbuffer, 1024);  // Added bzero for server buffer
        if(timer > 0){
            if(recv(serversocket, serverbuffer, sizeof(serverbuffer), 0) < 0){
                printf("Error while receiving server's message\n");
                timer--;
                goto y;
            }

            if(strcmp(serverbuffer, "N") == 0 || strcmp(serverbuffer, "n") == 0){
                printf("\nWaiting...");
                timer--;
                goto y;
            }

            if(strcmp(serverbuffer, "Y") == 0 || strcmp(serverbuffer, "y") == 0){
                printf("\nServer acknowledged");
                timer = 0;
                frames--;
            }
        }
        else{
            printf("\nServer did not acknowledge\nResending....");
            goto x;
        }
    }
          
    close(serversocket);
    return 0;
}
