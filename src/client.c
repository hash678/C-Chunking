#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#include <netinet/in.h>
#define PORT 9003
#define INT_SIZE 8
// #define LARGE_INT_SIZE 4


void createEmptyFile(char *fileName, int x) {
    FILE *fp = fopen(fileName, "w");
    fseek(fp, x-1 , SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

//Save char array to file
void saveToFile(FILE *fp, char *buffer, int size, int position)
{
    fseek(fp, position, SEEK_SET);
    fwrite(buffer, 1, size, fp);
}

int getPosition(char *str,int size){
    char position[INT_SIZE];
    for (int i= size - INT_SIZE; i < size; i++){
        position[i-size+INT_SIZE] = str[i];
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


char *cleanChunk(char *chunk, int size){
    char *text = malloc(size);
    memcpy(text, chunk, size);
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

    char  chunk_size_str[INT_SIZE];
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

    int number_of_chunks = 128;
    int chunk_size = handshake(network_socket,number_of_chunks);
    

    char extra_space_str[INT_SIZE];
    recv(network_socket, extra_space_str, INT_SIZE, 0);      
    int extra_space = atoi(extra_space_str);

    int current_chunk = 0;


    createEmptyFile("./sample/test1.png",(chunk_size*number_of_chunks)-extra_space);
    FILE *fp = fopen("./sample/test1.png", "r+b");


    while (current_chunk < number_of_chunks){
        char chunk_recv[chunk_size+INT_SIZE+2];
        recv(network_socket, chunk_recv, chunk_size+INT_SIZE+1, 0);      
        current_chunk += 1;


        char buffer[INT_SIZE];
        for (int i=chunk_size;i<chunk_size+INT_SIZE+1;i++){
            buffer[i-chunk_size] = chunk_recv[i];
        }
     
        //str to int
        int position = atoi(buffer);

        
        saveToFile(fp, chunk_recv, chunk_size ,position*chunk_size);

    }

    fclose(fp);


    //truncate
    truncate("./sample/test1.png", (chunk_size*number_of_chunks)-extra_space);
    fclose(fp);

      
    close(network_socket);
    return 0;
}
