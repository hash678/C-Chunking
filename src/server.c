#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define PORT 9003
#define INT_SIZE 8

struct args
{
    int socket;
    char *chunk;
    int size;
};

void *sendFile(void *input)
{
    int socket = ((struct args *)input)->socket;
    char *chunk = ((struct args *)input)->chunk;
    int size = ((struct args *)input)->size;

    send(socket, chunk, size, 0);
    return 0;
}

int sendData(int socket, char *data, int size)
{
    send(socket, data, size, 0);
    return 0;
}

int setupSocket()
{
    int network_socket;
    struct sockaddr_in server_address;

    //Create network socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //Bind the socket to the address
    int did_bind = bind(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (did_bind == -1)
    {
        printf("Error binding socket\n");
        exit(1);
    }

    //Listen for incoming connections
    listen(network_socket, 1);
    return network_socket;
}

long getFileSize(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

char *loadFile(FILE *file, int size,int position)
{

    if (file == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    //int to str
    char string_str[INT_SIZE];
    sprintf(string_str, "%d", position);

    char *string = malloc(size+INT_SIZE);
    fread(string, 1, size, file);

    printf("Pos: %d\n", position);

    for (int x = size; x < size+INT_SIZE; x++)
    {
        string[x] = string_str[x-size];
    }
    return string;
}

int main()
{

    //Setup Basic Socket
    int network_socket = setupSocket();
    int client_socket = accept(network_socket, NULL, NULL);

    char rec_buffer[INT_SIZE];
    recv(client_socket, rec_buffer, sizeof(rec_buffer), 0);

    int number_of_chunks = atoi(rec_buffer);

    long file_size = getFileSize("./sample/test.png");
    printf("File Size: %lu\n", file_size);
    int chunk_size = (file_size / number_of_chunks);
    chunk_size = chunk_size == 0 ? 1 : chunk_size + 1;

    printf("Chunk Size: %d\n", chunk_size);

    FILE *f = fopen("./sample/test.png", "r");
    printf("File Opened");

    //Sending the chunk size
    char chunk_size_str[INT_SIZE];
    sprintf(chunk_size_str, "%d", chunk_size);
    sendData(client_socket, chunk_size_str, INT_SIZE);

    pthread_t threads[number_of_chunks];

    for (int x = 0; x < number_of_chunks; x++)
    {
        char *chunk = loadFile(f, chunk_size,x);
        // sendFile(client_socket, chunk,chunk_size);
        // free(chunk);

        struct args *data = (struct args *)malloc(sizeof(struct args));
        data->socket = client_socket;
        data->chunk = chunk;
        data->size = chunk_size+INT_SIZE;

        pthread_t tid;
        pthread_create(&tid, NULL, sendFile, (void *)data);
        threads[x] = tid;
        //pthread_join(tid, NULL);
    }

    for (int x = 0; x < number_of_chunks; x++)
    {
        pthread_join(threads[x], NULL);
    }

    fclose(f);

    close(network_socket);

    return 0;
}