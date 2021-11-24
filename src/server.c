#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 9004
#define INT_SIZE 8

struct args
{
    int socket;
    char *chunk;
    int size;
};

// Check if the file path exists
int file_exists(char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// pthread_mutex_t lock;

// send a chunk of the file to the sokcet
void *sendFile(void *input)
{
    // pthread_mutex_lock(&lock);
    int socket = ((struct args *)input)->socket;
    char *chunk = ((struct args *)input)->chunk;
    int size = ((struct args *)input)->size;

    send(socket, chunk, size, 0);

    // pthread_mutex_unlock(&lock);

    return 0;
}

long getFileSize(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

// send data to the sokcet
int sendData(int socket, char *data, int size)
{
    send(socket, data, size, 0);
    return 0;
}

char *loadFile(FILE *file, int size, int position)
{
    if (file == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    //int to str
    char string_str[INT_SIZE];
    sprintf(string_str, "%d", position);

    char *string = malloc(size + INT_SIZE);
    fread(string, 1, size, file);

    for (int x = size; x < size + INT_SIZE; x++)
    {
        string[x] = string_str[x - size];
    }
    return string;
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

int main(int argc, char const *argv[])
{
    printf("Wekcome to Process A. Please Enter a file name in Process B.\n");

    int network_socket = setupSocket();
    int client_socket = accept(network_socket, NULL, NULL);

    char path[1024];

    read(client_socket, path, 1024);

    if (file_exists(path))
    {
        printf("Retrieved Path: %s\n", path);
        printf("File succesfully found.\n");

        char rec_buffer[INT_SIZE];
        recv(client_socket, rec_buffer, sizeof(rec_buffer), 0);

        int number_of_chunks = atoi(rec_buffer);

        long file_size = getFileSize(path);
        printf("File Size: %lu\n", file_size);
        int chunk_size = (file_size / number_of_chunks);
        chunk_size = chunk_size == 0 ? 1 : chunk_size + 1;

        printf("Chunk Size: %d\n", chunk_size);

        // if (chunk_size > 64 * 1024)
        // {
        //     printf("Chunk Size is too big\n");
        //     exit(1);
        // }

        FILE *f = fopen(path, "r");

        printf("File Opened.\n \n");

        int extra_space = (chunk_size * number_of_chunks) - file_size;
        printf("Extra Space: %d\n", extra_space);
        printf("Number of Chunks: %d\n", number_of_chunks);
        printf("chunk_size x number_of_chunks: %d\n", chunk_size * number_of_chunks);

        //Sending the chunk size
        char chunk_size_str[INT_SIZE];
        sprintf(chunk_size_str, "%d", chunk_size);
        sendData(client_socket, chunk_size_str, INT_SIZE);

        //Sending the extra space
        char extra_space_str[INT_SIZE];
        sprintf(extra_space_str, "%d", extra_space);
        sendData(client_socket, extra_space_str, INT_SIZE);

        pthread_t threads[number_of_chunks];

        for (int x = 0; x < number_of_chunks; x++)
        {
            char *chunk = loadFile(f, chunk_size, x);

            char extra = (extra_space != 0 && (number_of_chunks == x + 1)) ? '1' : '0';

            chunk[chunk_size + INT_SIZE] = extra;

            struct args *data = (struct args *)malloc(sizeof(struct args));
            data->socket = client_socket;
            data->chunk = chunk;
            data->size = chunk_size + INT_SIZE + 1;

            pthread_t tid;
            pthread_create(&tid, NULL, sendFile, (void *)data);
            threads[x] = tid;
            pthread_join(threads[x], NULL);
        }

        printf("File succesfully sent to process B. ");
        fclose(f);
        close(network_socket);
    }

    else
    {
        printf("Retrieved Path: %s\n", path);
        printf("File not found.\n");
        exit(1);
    }

    close(network_socket);
    shutdown(network_socket, 2);

    return 0;
}
