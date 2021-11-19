#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#include <netinet/in.h>
#define PORT 9004
#define MAX_SIZE 1024
#define INT_SIZE 4
// #define LARGE_INT_SIZE 4

int getPosition(char *str,int size){
    char position[INT_SIZE];
    for (int i= size - 4; i < size; i++){
        position[i-size+4] = str[i];
    }
    int position_int = atoi(position);
    return position_int;
}

int sendFile(int socket,char *data){
    send(socket, data, sizeof(data), 0);
    return 0;
}

char* getText(char *data,int size){
    char *text = malloc(size);
    memcpy(text, data, size);
   return text;
    }



// char *getFile(int socket,int chunk_size){
//     char *chunk_recieved = malloc(chunk_size);
//     recv(socket, chunk_recieved, chunk_size, 0);
//     return chunk_recieved;
// }
int copyData(char* a, char*b,int pos){
    for(int i=0;i<strlen(b);i++){
        a[i+pos] = b[i];
    }
    int final_pos = pos + strlen(b);
    return final_pos;
}

int handshake(int socket,int number_of_chunks){
    
    
    send(socket, &number_of_chunks, sizeof(number_of_chunks), 0);

    char chunk_size[4];
    recv(socket, chunk_size, sizeof(chunk_size), 0);
    recv(socket, chunk_size, sizeof(chunk_size), 0);

    printf("Chunk size: %s\n",chunk_size);


    return 0;
}

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

    int number_of_chunks = 8;
    int chunk_size = handshake(network_socket,number_of_chunks);
    
    int current_chunk = 0;



    // while (current_chunk < number_of_chunks){
    //     char chunk_recv[chunk_size];
    //     recv(network_socket, chunk_recv, chunk_size, 0);   
    //     current_chunk += 1;
    // }

 



      
    close(network_socket);
    return 0;
}