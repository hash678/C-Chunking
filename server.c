#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 


#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_SIZE 1024
#define PORT 9002



int main() {

    char sample_text[MAX_SIZE] = "Hello brother";

    int network_socket;
    struct sockaddr_in server_address;

    //Create network socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //Bind the socket to the address
    bind(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    //Listen for incoming connections
    listen(network_socket, 5);

    //Accept an incoming connection
    int client_socket = accept(network_socket, NULL, NULL);
    
    //Send the text to the client
    send(client_socket, sample_text, MAX_SIZE, 0);

    //Close the socket
    close(network_socket);


   return 0;
}