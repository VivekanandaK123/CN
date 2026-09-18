#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Packet structures matching lab specs
typedef struct {
    int seq_num;
    char data[BUFFER_SIZE];
} DataPacket;

typedef struct {
    int ack_num;
} AckPacket;

int main() {
    int sockfd;
    int port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("Enter Receiver Port Number: ");
    if (scanf("%d", &port) != 1) return 1;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("\nReceiver listening on port %d...\n\n", port);

    int expected_seq = 0;
    int ack_counter = 0;
    DataPacket recv_pkt;
    AckPacket ack_pkt;

    while (1) {
        ssize_t bytes = recvfrom(sockfd, &recv_pkt, sizeof(DataPacket), 0,
                                 (struct sockaddr *)&client_addr, &addr_len);
        if (bytes <= 0) continue;

        printf("--------------------------------------------------\n");
        printf("[RECEIVED] Packet with Seq No: %d | Data: \"%s\"\n", recv_pkt.seq_num, recv_pkt.data);

        if (recv_pkt.seq_num == expected_seq) {
            printf("[DELIVERY] Correct packet. Delivered to application.\n");
            expected_seq = 1 - expected_seq;
        } else {
            printf("[DUPLICATE] Duplicate packet detected! Ignored delivery.\n");
        }

        // Simulate ACK loss: discard every 3rd ACK to reliably show 2-3 retransmissions
        ack_counter++;
        if (ack_counter % 3 == 0) {
            printf("[SIMULATED LOSS] Deliberately dropping ACK %d (Ack Count: %d)\n",
                   recv_pkt.seq_num, ack_counter);
            continue; 
        }

        // Send ACK back for the packet just received
        ack_pkt.ack_num = recv_pkt.seq_num;
        sendto(sockfd, &ack_pkt, sizeof(AckPacket), 0,
               (struct sockaddr *)&client_addr, addr_len);
        printf("[SENT] ACK %d sent to sender.\n", ack_pkt.ack_num);
    }

    close(sockfd);
    return 0;
}