#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define N 1024
#define fileName "../toSend.mp4"
#define PORT 50100

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
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
    
    printf("Server listening on port %d...\n", PORT);

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    uint32_t num_packages = (file_size + N - 1) / N;

    //sending number of packages to receive
    send(client_socket, &num_packages, 32, 0);

    uint8_t file_part[N];
    int bytes_read = 0;
    for (int i = 0; i < num_packages; i++) {
        bytes_read = fread(file_part, 1, N, file);
        if (bytes_read <= 0) {
            perror("Failed to read from file");
            break;
        }
        if (send(client_socket, file_part, bytes_read, 0) == -1) {
            perror("Failed to send data to server");
            break;
        }
        printf("Sent package %d/%d (%d bytes)\n", i + 1, num_packages, bytes_read);
    }

    printf("File transmission completed.\n");

    close(client_socket);
    close(server_socket);
    return 0;
}