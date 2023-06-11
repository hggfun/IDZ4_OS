#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_VIEWERS 10

int database = 0;
struct sockaddr_in viewer_addresses[MAX_VIEWERS];
int viewer_count = 0;

void send_to_all_viewers(int server_fd, const char* message) {
    for (int i = 0; i < viewer_count; ++i) {
        sendto(server_fd, message, strlen(message), 0, (struct sockaddr *)&viewer_addresses[i], sizeof(viewer_addresses[i]));
    }
}

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

        if (buffer[0] == 'v') { // Viewer's message
            if (viewer_count < MAX_VIEWERS) {
                viewer_addresses[viewer_count++] = client_addr;
                printf("New viewer registered\n");
            }
        } else if (buffer[0] != 'r') {
            database = atoi(buffer);
            send_to_all_viewers(server_fd, "writer is writing");
        } else {
            send_to_all_viewers(server_fd, "reader is reading");
        }

        // Send the current database value to the client
        snprintf(buffer, BUFFER_SIZE, "%d", database);
        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, client_addr_len);
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

