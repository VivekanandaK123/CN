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
    if(argc<3){
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd;
    struct sockaddr_in server_addr;

    // 1. Create Socket
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Setup Server Address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr)<=0){
        perror("Invalid server IP address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Connect to Server
    if(connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        perror("Connection to server failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // 4. Read User Input
    int n;
    printf("Enter N: ");
    if(scanf("%d",&n) != 1 || n<=0){
        fprintf(stderr, "invalid input for N\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    int *numbers = (int *)malloc(n*sizeof(int));
    printf("Enter %d integers: ", n);
    for(int i=0;i<n;i++){
        if(scanf("%d", &numbers[i])!=1){
            fprintf(stderr, "Invalid input value.\n");
            free(numbers);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    // 5. Send N and integers to server 
    write(sock_fd, &n, sizeof(int));
    write(sock_fd, numbers, n*sizeof(int));

    // 6. Receive computed stats
    Stats stats;
    ssize_t bytes_read = read(sock_fd, &stats, sizeof(Stats));
    if(bytes_read == sizeof(Stats)){
        printf("Minimum = %d\n", stats.min);
        printf("Maximum = %d\n",stats.max);
        printf("Sum     = %lld\n",stats.sum);
        printf("Average = %.2f\n",stats.avg);
    }
    else{
        printf("Failed to receive complete statistics from server.\n");
    }

    free(numbers);
    close(sock_fd);
    return 0;
}