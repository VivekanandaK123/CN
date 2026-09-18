#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SIZE 1024
#define TIMEOUT 2

typedef struct {
    int seq;
    char data[SIZE];
} Packet;

typedef struct {
    int ack;
} ACK;

int main() {
    int sock, port, n, seq = 0;
    char ip[50];

    struct sockaddr_in receiver;
    socklen_t len = sizeof(receiver);

    printf("Enter Receiver IP: ");
    scanf("%s", ip);

    printf("Enter Receiver Port: ");
    scanf("%d", &port);

    printf("Enter Number of Messages: ");
    scanf("%d", &n);
    getchar();

    char msg[n][SIZE];

    for (int i = 0; i < n; i++) {
        printf("Enter Message %d: ", i + 1);
        fgets(msg[i], SIZE, stdin);
        msg[i][strcspn(msg[i], "\n")] = 0;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval tv = {TIMEOUT, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(port);
    inet_pton(AF_INET, ip, &receiver.sin_addr);

    for (int i = 0; i < n; i++) {

        Packet p = {seq};
        strcpy(p.data, msg[i]);

        while (1) {
            printf("Sending Seq %d: %s\n", seq, p.data);

            sendto(sock, &p, sizeof(p), 0,
                   (struct sockaddr *)&receiver, len);

            ACK a;

            if (recvfrom(sock, &a, sizeof(a), 0,
                         (struct sockaddr *)&receiver, &len) < 0) {

                printf("Timeout → Retransmitting Seq %d\n", seq);
                continue;
            }

            printf("ACK %d received\n", a.ack);

            if (a.ack == seq)
                break;
        }

        seq = 1 - seq;
    }

    close(sock);
}