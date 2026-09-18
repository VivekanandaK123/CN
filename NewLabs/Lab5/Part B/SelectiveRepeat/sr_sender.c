#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define SIZE 1024
#define WINDOW 4
#define TIMEOUT 2

typedef struct {
    int seq;
    char data[SIZE];
} Packet;

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in receiver;
    socklen_t len = sizeof(receiver);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &receiver.sin_addr);

    struct timeval tv = {TIMEOUT, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int n;
    printf("Enter number of packets: ");
    scanf("%d", &n);

    Packet p[n];
    int ack[n];

    for (int i = 0; i < n; i++) {
        p[i].seq = i;
        ack[i] = 0;

        printf("Enter message %d: ", i + 1);
        scanf("%s", p[i].data);
    }

    int base = 0;

    while (base < n) {

        int end = base + WINDOW;
        if (end > n) end = n;

        for (int i = base; i < end; i++) {
            if (!ack[i]) {
                printf("Sending packet %d\n", i);

                sendto(sock, &p[i], sizeof(p[i]), 0,
                       (struct sockaddr *)&receiver, len);
            }
        }

        int a;

        while (recvfrom(sock, &a, sizeof(a), 0,
                        (struct sockaddr *)&receiver, &len) > 0) {

            printf("ACK %d received\n", a);
            ack[a] = 1;

            while (base < n && ack[base])
                base++;

            if (base >= end)
                break;
        }

        for (int i = base; i < end; i++) {
            if (!ack[i])
                printf("Packet %d will be retransmitted\n", i);
        }
    }

    close(sock);
}