#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 9004
#define INT_SIZE 8

struct args
{
    int port;
    FILE *file;
    int chunk_size;
};

void createEmptyFile(char *fileName, int x)
{
    FILE *fp = fopen(fileName, "w");
    fseek(fp, x - 1, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

void fixFile(int newSize, char *fileName)
{
#ifdef _WIN32
    chsize(fileno(fopen(fileName, "r+")), newSize);
#elif _WIN64
    chsize(fileno(fopen(fileName, "r+")), newSize);
#else
#include <unistd.h>
    truncate(fileName, newSize);
#endif
}

//Save char array to file
void saveToFile(FILE *fp, char *buffer, int size, int position)
{
    fseek(fp, position, SEEK_SET);
    fwrite(buffer, 1, size, fp);
}

int sendFile(int socket, char *data)
{
    send(socket, data, sizeof(data), 0);
    return 0;
}

int handshake(int socket, int number_of_chunks)
{
    char number_of_chunks_str[INT_SIZE];
    sprintf(number_of_chunks_str, "%d", number_of_chunks);

    //Set number of chunks
    sendFile(socket, number_of_chunks_str);

    char chunk_size_str[INT_SIZE];
    recv(socket, chunk_size_str, INT_SIZE, 0);

    int chunk_size = atoi(chunk_size_str);
    // printf("%d\n", chunk_size);
    return chunk_size;
}



int main(int argc, char const *argv[])
{
    printf("Welcome to Process B.\n");

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
    if (did_connect == -1)
    {
        printf("Error connecting to server\n");
        return 1;
    }

    char path_to_inputFile[1024];
    printf("Enter path to input file: ");
    scanf("%s", path_to_inputFile);

    send(network_socket, path_to_inputFile, strlen(path_to_inputFile), 0);

    int number_of_chunks;
    printf("Enter number of threads: ");
    scanf("%d", &number_of_chunks);

    char path[100];
    printf("Enter path of output file: ");
    scanf("%s", path);

    int chunk_size = handshake(network_socket, number_of_chunks);

    char extra_space_str[INT_SIZE];
    recv(network_socket, extra_space_str, INT_SIZE, 0);
    int extra_space = atoi(extra_space_str);

    createEmptyFile(path, (chunk_size * number_of_chunks) - extra_space);
    FILE *fp = fopen(path, "r+b");

    for (int x = 0; x < number_of_chunks; x++)
    {
        char chunk_recv[chunk_size + INT_SIZE + 2];
        recv(network_socket, chunk_recv, chunk_size + INT_SIZE + 1, 0);
        char buffer[INT_SIZE];
        for (int i = chunk_size; i < chunk_size + INT_SIZE + 1; i++)
        {
            buffer[i - chunk_size] = chunk_recv[i];
        }
        //str to int
        int position = atoi(buffer);
        saveToFile(fp, chunk_recv, chunk_size, position * chunk_size);
    }

    fclose(fp);
    close(network_socket);
    shutdown(network_socket, 2);
    fixFile((chunk_size * number_of_chunks) - extra_space, path);

    return 0;
}