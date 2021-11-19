#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX_SIZE 1024
#define PORT 9003
#define INT_SIZE 4



int sendFile(int socket,char *data,int size){
    send(socket, data, size, 0);
    return 0;
}

int setupSocket(){
    int network_socket;
    struct sockaddr_in server_address;

    //Create network socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;


    //Bind the socket to the address
    int did_bind = bind(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    if (did_bind == -1) {
        printf("Error binding socket\n");
        exit(1);
    }

    //Listen for incoming connections
    listen(network_socket, 1);
    return network_socket;
}

long getFileSize(char *fileName){
    FILE *file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

char* loadFile(FILE *file, int size){
  
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    // fseek(file, 0, SEEK_END);
    // long fsize = ftell(file);

    // printf("File size: %ld\n", fsize);

    // fseek(file, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(size);
    fread(string, 1, size, file);
    return string;
}

// char * createChunk(char *data, int chunkSize,int position, int chunk_pos){
//     char *chunk = malloc(chunkSize);

//     for (int i = 0; i < chunkSize; i++) {
//         chunk[i] = data[position + i];
//     }

//     return chunk;
// }





int main() {

    //Setup Basic Socket
    int network_socket = setupSocket();
    int client_socket = accept(network_socket, NULL, NULL);
    

    char  rec_buffer[4];
    recv(client_socket, rec_buffer, sizeof(rec_buffer), 0);

    int number_of_chunks = atoi(rec_buffer);




    long file_size = getFileSize("./sample/test.png");
    printf("File Size: %lu\n", file_size);
    int chunk_size = (file_size / number_of_chunks);
    chunk_size = chunk_size == 0 ? 1 : chunk_size + 1;
    
    FILE *f = fopen("./sample/test.png", "r");


    //Sending the chunk size
    char chunk_size_str[4];
    sprintf(chunk_size_str, "%d", chunk_size);
    sendFile(client_socket, chunk_size_str, INT_SIZE);


    for (int x=0;x<number_of_chunks;x++){
        char *chunk = loadFile(f,chunk_size);
        sendFile(client_socket, chunk,chunk_size);
        free(chunk);
    }

    fclose(f);

    


    close(network_socket);

   return 0;
}