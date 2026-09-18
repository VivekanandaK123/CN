#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define TCP_PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    char host[128], type[16], query[200], response[512];

    while (1) {
        printf("Enter domain: ");
        scanf("%127s", host);

        if (!strcasecmp(host, "EXIT")) {
            send(sock, "EXIT", 5, 0);
            break;
        }

        printf("Enter type: ");
        scanf("%15s", type);

        sprintf(query, "%s %s", host, type);

        send(sock, query, strlen(query) + 1, 0);

        int n = recv(sock, response, sizeof(response) - 1, 0);
        response[n] = '\0';

        printf("Response: %s\n", response);
    }

    close(sock);
}