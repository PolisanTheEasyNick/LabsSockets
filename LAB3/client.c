#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define N 1024
#define PORT 50100

#define REQUEST_NUMBER_OF_PACKAGES 0
#define REQUEST_PACKAGES 1

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    printf("Creating client socket...\n");
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    printf("Created client socket!\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    printf("Setting up server address...\n");
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Creating message...\n");
    uint8_t *message = malloc(1);
    message[0] = REQUEST_NUMBER_OF_PACKAGES;

    printf("Requesting number of file packages...\n");
    //requesting number of file packages
    sendto(client_socket, message, 1, 0, (struct sockaddr *)&server_addr, addr_len);
    printf("Sent request to server!");
    //receiving number of file packages
    uint32_t num_packages = -1; 
    recvfrom(client_socket, &num_packages, sizeof(num_packages), 0, (struct sockaddr *)&server_addr, &addr_len);

    printf("Receiving %d packages...\n", num_packages);

    FILE *output_file = fopen("received_video.mp4", "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    //requesting file packages
    message[0] = REQUEST_PACKAGES;
    sendto(client_socket, message, 1, 0, (struct sockaddr *)&server_addr, addr_len);

    for (int i = 0; i < num_packages; i++) {
        uint8_t buffer[N + sizeof(uint32_t)];
        int bytes_received = recvfrom(client_socket, buffer, sizeof(buffer), 0, 
                                  (struct sockaddr *)&server_addr, &addr_len);
        if (bytes_received <= 0) {
            perror("Failed to receive data");
            break;
        }

        uint32_t package_id;
        memcpy(&package_id, buffer, sizeof(package_id));
        fwrite(buffer + sizeof(package_id), 1, bytes_received - sizeof(package_id), output_file);

        printf("Received package #%d (%d bytes)\n", package_id, bytes_received);
    }

    printf("File reception completed.\n");
    fclose(output_file);
    close(client_socket);
    free(message);
    return 0;
}