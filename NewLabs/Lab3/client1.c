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
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    int n;
    scanf("%d", &n);

    int *a = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++)
        scanf("%d", &a[i]);

    send(sock, &n, sizeof(n), 0);
    send(sock, a, n * sizeof(int), 0);

    Stats s;
    recv(sock, &s, sizeof(s), 0);

    printf("Minimum = %d\n", s.min);
    printf("Maximum = %d\n", s.max);
    printf("Sum = %lld\n", s.sum);
    printf("Average = %.2f\n", s.avg);

    free(a);
    close(sock);
}