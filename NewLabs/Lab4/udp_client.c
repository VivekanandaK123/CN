#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define UDP_PORT 8081
#define BUFFER_SIZE 512

int main(void) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    const char *msg = "STATUS";
    sendto(sock_fd, msg, strlen(msg), 0,
           (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    socklen_t addr_len = sizeof(serv_addr);

    ssize_t n = recvfrom(sock_fd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&serv_addr, &addr_len);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Response:\n%s", buffer);
    } else {
        perror("Failed to receive status");
    }

    close(sock_fd);
    return 0;
}