#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    int min;
    int max;
    long long sum;
    double avg;
} Stats;

int main(int argc, char *argv[]){

    if(argc<2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len =sizeof(client_addr);

    // 1. Create a TCP socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configure and Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen for connections
    if(listen(server_fd, 5) < 0){
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    // 4. Accept loop for multiple client sessions
    while(1){
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if(client_fd < 0){
            perror("Accept failed");
            continue;
        }

        // Print client IP and port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("\nAccepted connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // Read N from client
        int n;
        ssize_t bytes_read = read(client_fd, &n, sizeof(int));
        if(bytes_read <= 0){
            printf("Client disconnected unexpectedly.\n");
            close(client_fd);
            continue;
        }

        if(n<=0){
            printf("Invalid count N = %d received.\n",n);
            close(client_fd);
            continue;
        }

        int *numbers = (int *)malloc(n*sizeof(int));

        // Read the N integers
        size_t total_bytes = n*sizeof(int);
        size_t received = 0;
        char *ptr = (char *)numbers;
        while(received < total_bytes){
            ssize_t r = read(client_fd, ptr+received, total_bytes - received);
            if(r<=0) break;
            received+=r;
        }

        if(received < total_bytes){
            printf("Error reading array from client.\n");
            free(numbers);
            close(client_fd);
            continue;
        }

        // Display received numbers
        printf("Received %d integers: ",n);
        for(int i=0;i<n;i++){
            printf("%d ", numbers[i]);
        }
        printf("\n");

        // Compute Statistics
        Stats stats;
        stats.min = numbers[0];
        stats.max = numbers[0];
        stats.sum = 0;
        
        for(int i=0;i<n;i++){
            if(numbers[i]<stats.min) stats.min = numbers[i];
            if(numbers[i]>stats.max) stats.max = numbers[i];
            stats.sum+=numbers[i];
        }
        stats.avg = (double)stats.sum/n;

        printf("Computed:\n Min=%d\nMax=%d\nSum=%lld\nAvg=%.2f\n",stats.min,stats.max,stats.sum,stats.avg);

        // Send computed stats back to client
        write(client_fd, &stats, sizeof(Stats));

        // Client disconnection handling
        printf("Client %s:%d disconnected.\n", client_ip, ntohs(client_addr.sin_port));

        free(numbers);
        close(client_fd);
    }

    close(server_fd);
    return 0;

}