#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 2000
#define INT_SIZE 4
#define LONG_SIZE 8

struct args
{
    int thread_number;
    char *chunk;
    int size;
};

// setup socket
int setupSocket(int i)
{
    int network_socket;
    struct sockaddr_in server_address;

    //Create network socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT + i);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //Bind the socket to the address
    int did_bind = bind(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (did_bind == -1)
    {
        printf("Error binding socket %d\n", PORT + i);
        exit(1);
    }

    // printf("Connected to %d\n", PORT + i);

    //Listen for incoming connections
    listen(network_socket, 1);
    return network_socket;
}

// Check if the file path exists
int file_exists(char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// sending file chunk to the socket
void *sendChunk(void *input)
{
    int thread_number = ((struct args *)input)->thread_number;
    char *chunk = ((struct args *)input)->chunk;
    int size = ((struct args *)input)->size;

    int port = setupSocket(thread_number);
    int client_port = accept(port, NULL, NULL);

    send(client_port, chunk, size, 0);

    return NULL;
}

// Get the size of file in bytes
long getFileSize(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

// Send data to socket
void sendData(int socket, char *data, int size)
{
    send(socket, data, size, 0);
}

int main()
{
    printf("Welcome to process A, please enter a filename in process B. \n");

    int network_socket = setupSocket(-1);
    int client_socket = accept(network_socket, NULL, NULL);

    char path[100];

    read(client_socket, path, 100);

    int file_found = file_exists(path);

    if (file_found)
    {
        printf("Retrieved Path: %s\n", path);
        printf("File succesfully found.\n \n");
        long file_size = getFileSize(path);

        printf("File size: %lu\n", file_size);

        char file_size_str[LONG_SIZE];
        sprintf(file_size_str, "%lu", file_size);

        char number_of_chunks_str[INT_SIZE];
        recv(client_socket, number_of_chunks_str, INT_SIZE, 0);

        int number_of_chunks = atoi(number_of_chunks_str);

        printf("Recieved number of chunks: %d\n", number_of_chunks);

        int extra_space_size = file_size % number_of_chunks;

        char *extra_space_size_str = malloc(INT_SIZE);
        sprintf(extra_space_size_str, "%d", extra_space_size);

        printf("Extra space size: %d\n", extra_space_size);

        send(client_socket, file_size_str, LONG_SIZE, 0);

        send(client_socket, extra_space_size_str, INT_SIZE, 0);

        int chunk_size = file_size / number_of_chunks;

        pthread_t threads[number_of_chunks + 1];

        FILE *fp = fopen(path, "r");

        size_t bytesRead = 0;

        for (int i = 0; i < number_of_chunks; i++)
        {
            char *chunk = malloc(chunk_size);
            bytesRead = fread(chunk, 1, chunk_size, fp);

            struct args *data = (struct args *)malloc(sizeof(struct args));

            data->thread_number = i;
            data->chunk = chunk;
            data->size = chunk_size;

            pthread_create(&threads[i], NULL, sendChunk, (void *)data);
        }

        int number_of_chunks_iteration = number_of_chunks;

        if (extra_space_size)
        {
            number_of_chunks_iteration = number_of_chunks + 1;
            char *extra_space = malloc(extra_space_size);
            bytesRead = fread(extra_space, 1, extra_space_size, fp);

            struct args *data = (struct args *)malloc(sizeof(struct args));

            data->thread_number = number_of_chunks;
            data->chunk = extra_space;
            data->size = extra_space_size;

            pthread_create(&threads[number_of_chunks], NULL, sendChunk, (void *)data);
        }

        for (int i = 0; i < number_of_chunks_iteration; i++)
        {
            pthread_join(threads[i], NULL);
        }

        if (extra_space_size)
        {
            pthread_join(threads[number_of_chunks], NULL);
        }

        fclose(fp);
    }

    else
    {
        printf("Could not find the file %s\n", path);
        exit(1);
    }
}
