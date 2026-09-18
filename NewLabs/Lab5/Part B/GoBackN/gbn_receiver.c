#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 1024

typedef struct {
    int seq;
    char data[SIZE];
} Packet;

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    int expected = 0;
    Packet p;

    while (1) {
        recvfrom(sock, &p, sizeof(p), 0,
                 (struct sockaddr *)&client, &len);

        printf("Received packet %d\n", p.seq);

        if (p.seq == expected) {
            printf("Delivered: %s\n", p.data);
            sendto(sock, &expected, sizeof(expected), 0,
                   (struct sockaddr *)&client, len);
            expected++;
        } else {
            printf("Out of order packet\n");

            int ack = expected - 1;
            sendto(sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, len);
        }
    }
}