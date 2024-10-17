#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define N 1024
#define FILENAME "../toSend.mp4"
#define PORT 50100

#define REQUEST_NUMBER_OF_PACKAGES 0
#define REQUEST_PACKAGES 1

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

    printf("Server listening on port %d...\n", PORT);
    uint32_t num_packages = -1;
    FILE *file = fopen(FILENAME, "rb");
    //always listening for requests
    while(1) {
        uint8_t *buffer = malloc(1); //buffer from user will not be more than 1 byte.
        //receiving request from user
        int received = recvfrom(server_socket, buffer, 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        printf("Received %d bytes.\n", received);
        printf("Request type: %d\n", buffer[0]);
        switch(buffer[0]) {
            case REQUEST_NUMBER_OF_PACKAGES: {
                
                if (file == NULL) {
                    perror("Failed to open file");
                    exit(EXIT_FAILURE);
                }

                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                rewind(file);

                uint32_t num_packages = (file_size + N - 1) / N;
                sendto(server_socket, &num_packages, sizeof(num_packages), 0, (struct sockaddr *)&client_addr, client_addr_len);
                break;
            }
            case REQUEST_PACKAGES: {
                uint8_t file_part[N];
                int bytes_read = 0;
                uint8_t buffer[N + sizeof(uint32_t)];
                uint32_t package_id;

                for (package_id = 1; package_id <= num_packages; package_id++) {
                    bytes_read = fread(file_part, 1, N, file);
                    if (bytes_read <= 0) {
                        perror("Failed to read from file");
                        break;
                    }

                    memcpy(buffer, &package_id, sizeof(package_id));
                    memcpy(buffer + sizeof(package_id), file_part, bytes_read);
                    if (sendto(server_socket, buffer, bytes_read + sizeof(package_id), 0, 
                            (struct sockaddr *)&client_addr, client_addr_len) == -1) {
                        perror("Failed to send package");
                        break;
                    }

                    printf("Sent package %d (%d bytes)\n", package_id, bytes_read);
                }
            }
        }
        free(buffer);
    }




    printf("File transmission completed.\n");
    fclose(file);
    close(server_socket);
    return 0;
}
