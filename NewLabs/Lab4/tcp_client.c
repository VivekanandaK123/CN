#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define TCP_PORT 8080
#define BUFFER_SIZE 512

static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

int main(void) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection to TCP server failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to DNS Lookup Server. Enter 'EXIT' at domain prompt to quit.\n\n");

    char domain[128], type[16], payload[BUFFER_SIZE], response[BUFFER_SIZE];

    while (1) {
        printf("Enter domain: ");
        if (!fgets(domain, sizeof(domain), stdin)) break;
        trim_newline(domain);

        if (strcasecmp(domain, "EXIT") == 0) {
            send(sock_fd, "EXIT", 4, 0);
            break;
        }

        printf("Enter type: ");
        if (!fgets(type, sizeof(type), stdin)) break;
        trim_newline(type);

        // Serialize query as "<domain> <type>"
        snprintf(payload, sizeof(payload), "%s %s", domain, type);
        send(sock_fd, payload, strlen(payload), 0);

        memset(response, 0, sizeof(response));
        ssize_t bytes = recv(sock_fd, response, sizeof(response) - 1, 0);
        if (bytes <= 0) {
            printf("Server closed connection.\n");
            break;
        }

        printf("Server response:\n%s\n\n", response);
    }

    close(sock_fd);
    return 0;
}