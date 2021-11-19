#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#include <netinet/in.h>
#define PORT 9003
#define MAX_SIZE 1024
#define INT_SIZE 4
// #define LARGE_INT_SIZE 4

//Save char array to file
void save_file(char *filename, char *buffer, int size)
{
    FILE *fp = fopen(filename, "w");
    fwrite(buffer, 1, size, fp);
    fclose(fp);
}

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


    //print a
    return final_pos;
}

int handshake(int socket,int number_of_chunks){
    char number_of_chunks_str[INT_SIZE];
    sprintf(number_of_chunks_str, "%d", number_of_chunks);


    //Set number of chunks
    sendFile(socket, number_of_chunks_str);

    char  chunk_size_str[4];
    recv(socket, chunk_size_str, INT_SIZE, 0);

    int chunk_size = atoi(chunk_size_str);
    printf("%d\n", chunk_size);
    return chunk_size;
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


    char data[MAX_SIZE];

    int pos = 0;

    while (current_chunk < number_of_chunks){
        char chunk_recv[chunk_size+1];
        recv(network_socket, chunk_recv, chunk_size, 0);      

        chunk_recv[chunk_size] = '\0';
        // printf("%s\n", chunk_recv);
        current_chunk += 1;

        //append chunk_recv to data
        pos = copyData(data,chunk_recv,pos);
    }
    data[pos+1] = '\0';

    printf("%s", data);
    save_file("./sample/rcv.txt", data, strlen(data));

 



      
    close(network_socket);
    return 0;
}