#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TCP_PORT 8080
#define UDP_PORT 8081

typedef struct {
    char host[128], type[16], value[128];
} Record;

Record records[] = {
    {"www.example.com",  "A",     "192.168.1.10"},
    {"mail.example.com", "A",     "192.168.1.20"},
    {"web.example.com",  "CNAME", "www.example.com"},
    {"example.com",      "MX",    "mail.example.com"},
    {"example.com",      "NS",    "ns1.example.com"}
};

int clients = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char *lookup(char *host, char *type) {
    for (int i = 0; i < 5; i++)
        if (!strcasecmp(host, records[i].host) &&
            !strcasecmp(type, records[i].type))
            return records[i].value;

    return "Record not found";
}

void *tcp_client(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(fd, (struct sockaddr *)&addr, &len);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));

    pthread_mutex_lock(&lock);
    clients++;
    pthread_mutex_unlock(&lock);

    printf("TCP Client connected: %s:%d\n", ip, ntohs(addr.sin_port));

    char buf[512], host[128], type[16];

    while (recv(fd, buf, sizeof(buf) - 1, 0) > 0) {
        if (!strcasecmp(buf, "EXIT")) break;

        if (sscanf(buf, "%127s %15s", host, type) == 2) {
            char *result = lookup(host, type);
            send(fd, result, strlen(result) + 1, 0);
        }
    }

    pthread_mutex_lock(&lock);
    clients--;
    pthread_mutex_unlock(&lock);

    close(fd);
    return NULL;
}

void *udp_server(void *arg) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(UDP_PORT);

    bind(fd, (struct sockaddr *)&server, sizeof(server));

    char buf[100];
    socklen_t len = sizeof(client);

    while (1) {
        int n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&client, &len);

        buf[n] = '\0';

        if (!strcasecmp(buf, "STATUS")) {
            char response[100];

            pthread_mutex_lock(&lock);
            int nclients = clients;
            pthread_mutex_unlock(&lock);

            sprintf(response, "Server active. Connected clients: %d",
                    nclients);

            sendto(fd, response, strlen(response) + 1, 0,
                   (struct sockaddr *)&client, len);
        }
    }
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, udp_server, NULL);
    pthread_detach(tid);

    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(TCP_PORT);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 10);

    while (1) {
        int *client = malloc(sizeof(int));
        *client = accept(server, NULL, NULL);

        pthread_t tid;
        pthread_create(&tid, NULL, tcp_client, client);
        pthread_detach(tid);
    }
}