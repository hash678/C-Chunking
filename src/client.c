#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define PORT 1000
#define INT_SIZE 32

struct args
{
    int port;
    FILE *file;
    int chunk_size;
};

void createEmptyFile(char *fileName, int x)
{
    printf("File of size %d \n", x);
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

int getPosition(char *str, int size)
{
    char position[INT_SIZE];
    for (int i = size - INT_SIZE; i < size; i++)
    {
        position[i - size + INT_SIZE] = str[i];
    }
    int position_int = atoi(position);
    return position_int;
}

int sendFile(int socket, char *data)
{
    send(socket, data, sizeof(data), 0);
    return 0;
}

// pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *recievePacket(void *input)
{
    // pthread_mutex_lock(&m);

    int port = ((struct args *)input)->port;

    int network_socket;
    struct sockaddr_in packet_server_address;

    //Create network socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    packet_server_address.sin_family = AF_INET;
    packet_server_address.sin_port = htons(port);
    packet_server_address.sin_addr.s_addr = INADDR_ANY;

    // printf("Connecting to socket on port %d \n",port);

    int did_connect = -1;
    did_connect = connect(network_socket, (struct sockaddr *)&packet_server_address, sizeof(packet_server_address));

    // printf("Connected on port: %d \n", port);

    FILE *fp = ((struct args *)input)->file;
    int chunk_size = ((struct args *)input)->chunk_size;

    char chunk_recv[chunk_size + INT_SIZE + 2];
    recv(network_socket, chunk_recv, chunk_size + INT_SIZE + 1, 0);

    char buffer[INT_SIZE];
    for (int i = chunk_size; i < chunk_size + INT_SIZE + 1; i++)
    {
        buffer[i - chunk_size] = chunk_recv[i];
    }
    int position = atoi(buffer);
    saveToFile(fp, chunk_recv, chunk_size, position * chunk_size);

    // pthread_mutex_unlock(&m);

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
    printf("%d\n", chunk_size);
    return chunk_size;
}

int main()
{

    int number_of_chunks = 400;
    char *path = "./sample/ok1.mp4";

    //Create network socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Create the address to connect to
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //Connect to the server
    int did_connect = connect(network_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //Check if connection successful
    if (did_connect == -1)
    {
        printf("Error connecting to server\n");
        return 1;
    }

    int chunk_size = handshake(network_socket, number_of_chunks);

    char extra_space_str[INT_SIZE];
    recv(network_socket, extra_space_str, INT_SIZE, 0);
    int extra_space = atoi(extra_space_str);

    int file_size = (chunk_size * number_of_chunks) - extra_space;
    createEmptyFile(path, file_size);
    FILE *fp = fopen(path, "r+b");

    pthread_t threads[number_of_chunks];

    for (int x = 0; x < number_of_chunks; x++)
    {
        pthread_t thread;
        struct args *args = malloc(sizeof(struct args));
        args->port = PORT + x + 1;
        args->file = fp;
        args->chunk_size = chunk_size;
        pthread_create(&thread, NULL, recievePacket, args);
        threads[x] = thread;
        pthread_join(threads[x], NULL);
    }

    fclose(fp);
    close(network_socket);
    shutdown(network_socket, 2);
    fixFile((chunk_size * number_of_chunks) - extra_space, path);

    return 0;
}
