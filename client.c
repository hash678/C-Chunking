#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>
#define PORT 9002
#define MAX_SIZE 1024

int main(){
 
    //Create network socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Create the address to connect to
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    //Set the IP address to 0.0.0.0 i.e Localhost
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //Connect to the server
    int did_connect = connect(network_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //Check if connection successful
    if (did_connect == -1){
        printf("Error connecting to server\n");
        return 1;
    }

    char server_response[MAX_SIZE];
    recv(network_socket, &server_response, sizeof(server_response), 0);

    printf("%s\n", server_response);

    //Close the socket
    close(network_socket);
    return 0;
}