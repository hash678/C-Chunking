#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX_SIZE 1024




int main() {

    int mainSocket,newSocket;



    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;


    
    mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    //Set the family of the address
    serverAddr.sin_family = AF_INET;
    //Set the port
    serverAddr.sin_port = htons(8080);
    //Set the IP address
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    //Bind socket to server address
    bind(mainSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));


    if(listen(mainSocket,1)==0)
        printf("Listening\n");
    else
        printf("Error\n");

      /*---- Accept call creates a new socket for the incoming connection ----*/
    addr_size = sizeof serverStorage;
    newSocket = accept(mainSocket, (struct sockaddr *) &serverStorage, &addr_size);

        /*---- Send message to the socket of the incoming connection ----*/

        
        FILE *f;
        char buffer[MAX_SIZE];
        f = fopen("sample.txt", "rb");
        if (f){fread(buffer, MAX_SIZE, 1, f);}

        

    strcpy(buffer,"Hello World\n");
    send(newSocket,buffer,13,0);

    close(newSocket);
    close(mainSocket);

   return 0;
}