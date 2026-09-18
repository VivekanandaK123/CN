#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    int client = accept(server, NULL, NULL);

    char data[1024];

    while (recv(client, data, sizeof(data), 0) > 0) {
        printf("Received: %s\n", data);
        sleep(1);
    }

    close(client);
    close(server);
}