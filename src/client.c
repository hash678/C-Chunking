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
    int size;
    int thread_number;
};

int setupSocket(int i)
{
    //Create network socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Create the address to connect to
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT + i);

    //Set the IP address to 0.0.0.0 i.e Localhost
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //Connect to the server
    int did_connect = connect(network_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //Check if connection successful
    if (did_connect == -1)
    {
        printf("Error connecting to socket %d\n", PORT + i);
        exit(1);
    }

    return network_socket;
}

void createEmptyFile(char *fileName, int x)
{
    FILE *fp = fopen(fileName, "w");
    fseek(fp, x - 1, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

//Save char array to file
void saveToFile(FILE *fp, char *buffer, int size, int position)
{
    fseek(fp, position, SEEK_SET);
    fwrite(buffer, 1, size, fp);
}

void *recieveChunk(void *input)
{
    // int port = ((struct args *)input)->port;
    int size = ((struct args *)input)->size;
    int thread_number = ((struct args *)input)->thread_number;

    int port = setupSocket(thread_number);

    char *chunk = malloc(size);

    read(port, chunk, size);

    return (void *)chunk;
}

long handshake(int socket, int number_of_chunks)
{
    char number_of_chunks_str[INT_SIZE];
    sprintf(number_of_chunks_str, "%d", number_of_chunks);

    //Set number of chunks
    send(socket, number_of_chunks_str, INT_SIZE, 0);

    char file_size_str[LONG_SIZE];
    recv(socket, file_size_str, LONG_SIZE, 0);

    long file_size = atoll(file_size_str);

    return file_size;
}

int main()
{
    printf("Welcome to Process B.\n");

    char path_to_inputFile[100] = "./sample/input.txt";
    // printf("Enter path to input file: ");
    // scanf("%s", path_to_inputFile);

    char path_to_outputFile[100] = "./sample/output.txt";
    // printf("Enter path to input file: ");
    // scanf("%s", path_to_inputFile);

    int network_socket = setupSocket(-1);

    send(network_socket, path_to_inputFile, sizeof(path_to_inputFile), 0);

    int number_of_chunks = 10;
    // printf("Enter number of threads: ");
    // scanf("%d", &number_of_chunks);

    long file_size = handshake(network_socket, number_of_chunks);

    printf("File size: %lu\n", file_size);

    char *extra_space_size_str = malloc(INT_SIZE);
    recv(network_socket, extra_space_size_str, INT_SIZE, 0);

    int extra_space_size = atoi(extra_space_size_str);
    printf("Extra space size: %d\n", extra_space_size);

    int chunk_size = file_size / number_of_chunks;

    printf("Chunk Size: %d\n", chunk_size);

    createEmptyFile(path_to_outputFile, (chunk_size * number_of_chunks) - extra_space_size);
    FILE *fp = fopen(path_to_outputFile, "r+b");

    pthread_t threads[number_of_chunks + 1];

    for (int i = 0; i < number_of_chunks; i++)
    {
        struct args *data = (struct args *)malloc(sizeof(struct args));

        data->size = chunk_size;
        data->thread_number = i;
        pthread_create(&threads[i], NULL, recieveChunk, (void *)data);
    }

    if (extra_space_size)
    {
        struct args *data = (struct args *)malloc(sizeof(struct args));

        data->size = extra_space_size;
        data->thread_number = number_of_chunks;
        pthread_create(&threads[number_of_chunks], NULL, recieveChunk, (void *)data);
    }

    for (int i = 0; i < number_of_chunks; i++)
    {
        void *chunk;
        pthread_join(threads[i], &chunk);
        saveToFile(fp, chunk, chunk_size, i * chunk_size);
    }

    if (extra_space_size)
    {
        void *extra_space;
        pthread_join(threads[number_of_chunks], &extra_space);
        saveToFile(fp, extra_space, extra_space_size, file_size - extra_space_size);
    }

    fclose(fp);

    return 0;
}