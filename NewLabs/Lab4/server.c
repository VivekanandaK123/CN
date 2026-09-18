#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TCP_PORT 8080
#define UDP_PORT 8081
#define BUFFER_SIZE 512

typedef struct {
    char hostname[128];
    char type[16];
    char value[128];
} DnsRecord;

// Static DNS table
static DnsRecord dns_table[] = {
    {"www.example.com", "A", "192.168.1.10"},
    {"mail.example.com", "A", "192.168.1.20"},
    {"web.example.com", "CNAME", "www.example.com"},
    {"example.com", "MX", "mail.example.com"},
    {"example.com", "NS", "ns1.example.com"}
};
static const int DNS_TABLE_SIZE = sizeof(dns_table) / sizeof(dns_table[0]);

// Shared client counter protected by mutex
static int active_tcp_clients = 0;
static pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int socket_fd;
    struct sockaddr_in client_addr;
} ClientContext;

// Trims trailing newline/carriage return characters
static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

// Lookup record in DNS table
static const char *lookup_record(const char *domain, const char *type) {
    for (int i = 0; i < DNS_TABLE_SIZE; i++) {
        if (strcasecmp(dns_table[i].hostname, domain) == 0 &&
            strcasecmp(dns_table[i].type, type) == 0) {
            return dns_table[i].value;
        }
    }
    return "Record not found";
}

// Worker thread handling one TCP client
void *tcp_client_handler(void *arg) {
    ClientContext *ctx = (ClientContext *)arg;
    int client_fd = ctx->socket_fd;
    char client_ip[INET_ADDRSTRLEN];
    int client_port = ntohs(ctx->client_addr.sin_port);
    inet_ntop(AF_INET, &(ctx->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

    pthread_mutex_lock(&client_count_mutex);
    active_tcp_clients++;
    pthread_mutex_unlock(&client_count_mutex);

    printf("[TCP] Client connected: %s:%d (Active: %d)\n",
           client_ip, client_port, active_tcp_clients);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) break;

        trim_newline(buffer);
        if (strcasecmp(buffer, "EXIT") == 0) break;

        // Parse format: "<domain> <type>"
        char domain[128] = {0};
        char type[16] = {0};
        if (sscanf(buffer, "%127s %15s", domain, type) == 2) {
            const char *result = lookup_record(domain, type);
            send(client_fd, result, strlen(result), 0);
        } else {
            const char *err = "Invalid query format. Expected: <domain> <type>";
            send(client_fd, err, strlen(err), 0);
        }
    }

    close(client_fd);
    free(ctx);

    pthread_mutex_lock(&client_count_mutex);
    active_tcp_clients--;
    pthread_mutex_unlock(&client_count_mutex);

    printf("[TCP] Client disconnected: %s:%d (Active: %d)\n",
           client_ip, client_port, active_tcp_clients);

    pthread_detach(pthread_self());
    return NULL;
}

// Background thread handling UDP status requests
void *udp_status_server(void *arg) {
    (void)arg;
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP socket creation failed");
        return NULL;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("UDP bind failed");
        close(udp_fd);
        return NULL;
    }

    printf("[UDP] Status service listening on port %d...\n", UDP_PORT);

    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = recvfrom(udp_fd, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (n <= 0) continue;

        trim_newline(buffer);
        if (strcasecmp(buffer, "STATUS") == 0) {
            char response[BUFFER_SIZE];
            pthread_mutex_lock(&client_count_mutex);
            int current_clients = active_tcp_clients;
            pthread_mutex_unlock(&client_count_mutex);

            snprintf(response, sizeof(response),
                     "Server active. Connected clients: %d\n", current_clients);
            sendto(udp_fd, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, addr_len);
        }
    }

    close(udp_fd);
    return NULL;
}

int main(void) {
    // 1. Launch UDP Status thread
    pthread_t udp_tid;
    if (pthread_create(&udp_tid, NULL, udp_status_server, NULL) != 0) {
        perror("Failed to create UDP listener thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(udp_tid);

    // 2. Setup TCP master listener
    int tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_server_fd < 0) {
        perror("TCP socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(TCP_PORT);

    if (bind(tcp_server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("TCP bind failed");
        close(tcp_server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(tcp_server_fd, 10) < 0) {
        perror("TCP listen failed");
        close(tcp_server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TCP] DNS Server listening on port %d...\n", TCP_PORT);

    // 3. Accept TCP clients loop
    while (1) {
        ClientContext *ctx = malloc(sizeof(ClientContext));
        socklen_t client_len = sizeof(ctx->client_addr);
        ctx->socket_fd = accept(tcp_server_fd, (struct sockaddr *)&(ctx->client_addr), &client_len);

        if (ctx->socket_fd < 0) {
            perror("TCP accept error");
            free(ctx);
            continue;
        }

        pthread_t client_tid;
        if (pthread_create(&client_tid, NULL, tcp_client_handler, ctx) != 0) {
            perror("Failed to spawn thread for client");
            close(ctx->socket_fd);
            free(ctx);
        }
    }

    close(tcp_server_fd);
    return 0;
}