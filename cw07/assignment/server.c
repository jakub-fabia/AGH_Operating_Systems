#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define SERVER_QUEUE_KEY 1234
#define MAX_CLIENTS 10
#define MAX_TEXT 256

typedef struct {
    long mtype;
    pid_t pid;
    key_t client_queue_key;
    char text[MAX_TEXT];
} message;

typedef struct {
    long mtype;
    int client_id;
    char text[MAX_TEXT];
} response;

typedef struct {
    int client_id;
    int queue_id;
    int active;
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;

void broadcast_message(int sender_id, const char *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].client_id != sender_id) {
            response res;
            res.mtype = 1;
            res.client_id = sender_id;
            snprintf(res.text, MAX_TEXT, "Client %d: %s", sender_id, msg);
            if (msgsnd(clients[i].queue_id, &res, sizeof(res) - sizeof(long), 0) == -1) {
                perror("msgsnd - broadcast");
            }
        }
    }
}

int main() {
    int server_queue_id = msgget(SERVER_QUEUE_KEY, IPC_CREAT | 0666);
    if (server_queue_id == -1) {
        perror("msgget - server");
        exit(EXIT_FAILURE);
    }

    printf("[Server] Started. Waiting for messages...\n");

    message msg;
    while (1) {
        ssize_t rcv_size = msgrcv(server_queue_id, &msg, sizeof(msg) - sizeof(long), 0, 0);
        if (rcv_size == -1) {
            perror("msgrcv - server");
            continue;
        }

        if (strncmp(msg.text, "INIT", 4) == 0) {
            if (client_count >= MAX_CLIENTS) {
                fprintf(stderr, "[Server] Max clients reached.\n");
                continue;
            }

            int client_queue_id = msgget(msg.client_queue_key, 0);
            if (client_queue_id == -1) {
                perror("msgget - client queue");
                continue;
            }

            int id = client_count + 1;
            clients[client_count].client_id = id;
            clients[client_count].queue_id = client_queue_id;
            clients[client_count].active = 1;
            client_count++;

            response res;
            res.mtype = 1;
            res.client_id = id;
            snprintf(res.text, MAX_TEXT, "Your client ID is %d", id);

            if (msgsnd(client_queue_id, &res, sizeof(res) - sizeof(long), 0) == -1) {
                perror("msgsnd - send client ID");
            }

            printf("[Server] Registered client %d (queue id = %d)\n", id, client_queue_id);
        } else {
            int sender_id = msg.pid;
            printf("[Server] Received from client %d: %s\n", sender_id, msg.text);
            broadcast_message(sender_id, msg.text);
        }
    }

    return 0;
}
