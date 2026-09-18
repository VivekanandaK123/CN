#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 2

typedef struct {
    int seq_num;
    char data[BUFFER_SIZE];
} DataPacket;

typedef struct {
    int ack_num;
} AckPacket;

int main() {
    int sockfd;
    char ip[50];
    int port, num_messages;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    printf("Enter Receiver IP Address: ");
    if (scanf("%s", ip) != 1) return 1;

    printf("Enter Receiver Port: ");
    if (scanf("%d", &port) != 1) return 1;

    printf("Enter Number of Messages: ");
    if (scanf("%d", &num_messages) != 1) return 1;
    getchar();

    char messages[num_messages][BUFFER_SIZE];
    for (int i = 0; i < num_messages; i++) {
        printf("Enter Message %d: ", i + 1);
        fgets(messages[i], BUFFER_SIZE, stdin);
        messages[i][strcspn(messages[i], "\n")] = 0; 
    }

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure socket receive timeout
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    int current_seq = 0;
    DataPacket send_pkt;
    AckPacket ack_pkt;

    printf("\nStarting Reliable Data Transfer...\n");

    for (int i = 0; i < num_messages; i++) {
        send_pkt.seq_num = current_seq;
        strncpy(send_pkt.data, messages[i], BUFFER_SIZE);

        int ack_received = 0;
        int attempts = 0;

        while (!ack_received) {
            attempts++;
            if (attempts == 1) {
                printf("--------------------------------------------------\n");
                printf("[TRANSMIT] Sending Packet (Seq: %d, Data: \"%s\")\n",
                       send_pkt.seq_num, send_pkt.data);
            } else {
                printf("[RETRANSMIT] Resending Packet (Seq: %d, Data: \"%s\")\n",
                       send_pkt.seq_num, send_pkt.data);
            }

            // Send packet
            sendto(sockfd, &send_pkt, sizeof(DataPacket), 0,
                   (struct sockaddr *)&server_addr, addr_len);

            // Wait for ACK
            ssize_t bytes = recvfrom(sockfd, &ack_pkt, sizeof(AckPacket), 0,
                                     (struct sockaddr *)&server_addr, &addr_len);

            if (bytes < 0) {
                // Timeout event occurred
                printf("[TIMEOUT] Timeout waiting for ACK %d! Triggering retransmission.\n", current_seq);
            } else {
                printf("[ACK RECEIVED] Received ACK %d\n", ack_pkt.ack_num);
                if (ack_pkt.ack_num == current_seq) {
                    ack_received = 1;
                    current_seq = 1 - current_seq; // Alternate sequence: 0 -> 1 -> 0
                }
            }
        }
    }

    printf("--------------------------------------------------\n");
    printf("All messages sent and acknowledged successfully.\n");

    close(sockfd);
    return 0;
}