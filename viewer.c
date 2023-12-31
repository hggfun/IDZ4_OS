#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void start_client(const char* server_ip, int server_port) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("Viewer started\n");
    while(1) {
        // Send a read request to the server
        sendto(sock, "v", strlen("v"), 0, (struct sockaddr *)&serv_addr, addr_len);
        // Receive the current message from the server
        memset(buffer, 0, BUFFER_SIZE);
        valread = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        if (valread > 0) {
            printf("Viewer noticed that: %s\n", buffer);
        } else {
            printf("Viewing failed\n");
        }
    }

    // Close the socket
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Port num required");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    start_client(server_ip, server_port);

    return 0;
}
