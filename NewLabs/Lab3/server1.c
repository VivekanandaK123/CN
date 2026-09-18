#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
    int min, max;
    long long sum;
    double avg;
} Stats;

int main(int argc, char *argv[]) {
    int server_fd, client_fd, port = atoi(argv[1]);
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 5);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client, &len);

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
        printf("Client %s connected\n", ip);
        
        int n;
        recv(client_fd, &n, sizeof(n), 0);

        int *a = malloc(n * sizeof(int));
        recv(client_fd, a, n * sizeof(int), 0);

        Stats s = {a[0], a[0], 0, 0};

        for (int i = 0; i < n; i++) {
            if (a[i] < s.min) s.min = a[i];
            if (a[i] > s.max) s.max = a[i];
            s.sum += a[i];
        }

        s.avg = (double)s.sum / n;

        send(client_fd, &s, sizeof(s), 0);

        free(a);
        close(client_fd);
    }
}