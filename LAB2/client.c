#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define N 1024
#define PORT 50100

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); 

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server!\n");

    uint32_t packages_to_receive = 0;
    if (recv(client_socket, &packages_to_receive, 32, 0) <= 0) {
        perror("Failed to receive number of packages");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Receiving %d packages...\n", packages_to_receive);

    FILE *output_file = fopen("../received_video.mp4", "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    uint8_t buffer[N];
    int bytes_received;
    for (int i = 0; i < packages_to_receive; i++) {
        bytes_received = recv(client_socket, buffer, N, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive data from client");
            break;
        }
        fwrite(buffer, 1, bytes_received, output_file);
        printf("Received package %d/%d (%d bytes)\n", i + 1, packages_to_receive, bytes_received);
    }
    printf("File reception completed.\n");
    fclose(output_file);

    close(client_socket);
    return 0;
}
