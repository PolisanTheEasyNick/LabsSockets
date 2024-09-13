#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    printf("Receiving data from client...\n");
    uint8_t buffer[1024];
    int received_bytes = recv(client_socket, buffer, 1024, 0);
    printf("Received %d bytes from client: %s\n", received_bytes, buffer);

    close(client_socket);
    close(server_socket);
    return 0;
}