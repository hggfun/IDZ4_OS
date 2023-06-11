#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int database = 0;
int* viewer;

void start_server(const char* host, int port) {
    int server_fd;
    struct sockaddr_in address, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", host, port);

    // Handle incoming requests
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive the message from the client
        recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        
        // Lock the mutex
        pthread_mutex_lock(&mutex);

        if (buffer[0] != 'r') {
            database = atoi(buffer);
            if (viewer != NULL) {
                sendto(server_fd, "writer is writing", strlen("writer is writing"), 0, (struct sockaddr *)viewer, client_addr_len);
            }
        } else {
            if (viewer != NULL) {
                sendto(server_fd, "reader is reading", strlen("reader is reading"), 0, (struct sockaddr *)viewer, client_addr_len);
            }
        }
        // Send the current database value to the client
        snprintf(buffer, BUFFER_SIZE, "%d", database);
        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, client_addr_len);

        // Unlock the mutex
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    start_server(host, port);

    return 0;
}
