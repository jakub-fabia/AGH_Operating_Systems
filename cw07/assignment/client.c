#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_QUEUE_KEY 1234
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

int client_queue_id;

void cleanup(int sig) {
    if (msgctl(client_queue_id, IPC_RMID, NULL) == -1) {
        perror("msgctl - remove client queue");
    } else {
        printf("[Client] Queue removed. Exiting.\n");
    }
    exit(0);
}

void receive_messages() {
    response res;
    while (1) {
        if (msgrcv(client_queue_id, &res, sizeof(res) - sizeof(long), 0, 0) > 0) {
            printf("[Client] %s\n", res.text);
        } else {
            perror("msgrcv - client listener");
        }
    }
}

int main() {
    signal(SIGINT, cleanup);

    key_t client_key = ftok(".", getpid());
    if (client_key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    client_queue_id = msgget(client_key, IPC_CREAT | 0666);
    if (client_queue_id == -1) {
        perror("msgget - client");
        exit(EXIT_FAILURE);
    }

    int server_queue_id = msgget(SERVER_QUEUE_KEY, 0);
    if (server_queue_id == -1) {
        perror("msgget - server");
        cleanup(0);
    }

    message init_msg;
    init_msg.mtype = 1;
    init_msg.pid = getpid();
    init_msg.client_queue_key = client_key;
    strcpy(init_msg.text, "INIT");

    if (msgsnd(server_queue_id, &init_msg, sizeof(init_msg) - sizeof(long), 0) == -1) {
        perror("msgsnd - INIT");
        cleanup(0);
    }

    response res;
    if (msgrcv(client_queue_id, &res, sizeof(res) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv - receiving client ID");
        cleanup(0);
    }

    int client_id = res.client_id;
    printf("[Client] Received ID: %d\n", client_id);

    if (fork() == 0) {
        receive_messages();
        exit(0);
    }

    while (1) {
        char input[MAX_TEXT];
        if (!fgets(input, MAX_TEXT, stdin)) break;

        message msg;
        msg.mtype = 1;
        msg.pid = client_id;
        msg.client_queue_key = 0;
        strncpy(msg.text, input, MAX_TEXT);

        if (msgsnd(server_queue_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("msgsnd - message");
        }
    }

    cleanup(0);
    return 0;
}
