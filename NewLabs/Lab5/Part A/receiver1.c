#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIZE 1024

typedef struct {
    int seq;
    char data[SIZE];
} Packet;

typedef struct {
    int ack;
} ACK;

int main() {
    int sock, port, expected = 0, count = 0;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    printf("Enter Receiver Port: ");
    scanf("%d", &port);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Receiver listening on port %d\n", port);

    Packet p;
    ACK a;

    while (1) {
        recvfrom(sock, &p, sizeof(p), 0,
                 (struct sockaddr *)&client, &len);

        printf("Received: Seq %d, %s\n", p.seq, p.data);

        if (p.seq == expected) {
            printf("Delivered\n");
            expected = 1 - expected;
        } else {
            printf("Duplicate\n");
        }

        count++;

        if (count % 3 == 0) {
            printf("ACK %d lost\n", p.seq);
            continue;
        }

        a.ack = p.seq;

        sendto(sock, &a, sizeof(a), 0,
               (struct sockaddr *)&client, len);
    }
}