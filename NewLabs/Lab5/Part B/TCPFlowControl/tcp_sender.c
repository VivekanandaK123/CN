#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    char data[1024];

    for (int i = 0; i < 20; i++) {
        sprintf(data, "Message %d", i);

        send(sock, data, strlen(data) + 1, 0);

        printf("Sent: %s\n", data);
    }

    close(sock);
}