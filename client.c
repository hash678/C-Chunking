#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#define PORT 8080

int main(){
 
    //Create network socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Create the address to connect to
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("");

    return 0;
}