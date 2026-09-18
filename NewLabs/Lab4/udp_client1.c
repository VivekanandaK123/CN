#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define UDP_PORT 8081

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    char *msg = "STATUS";
    char response[100];
    socklen_t len = sizeof(server);

    sendto(sock, msg, strlen(msg) + 1, 0,
           (struct sockaddr *)&server, len);

    int n = recvfrom(sock, response, sizeof(response) - 1, 0,
                     (struct sockaddr *)&server, &len);

    response[n] = '\0';

    printf("Response: %s\n", response);

    close(sock);
}