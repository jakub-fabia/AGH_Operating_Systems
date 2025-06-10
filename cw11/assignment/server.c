#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define MAX_CLIENTS 50
#define MAX_NAME_LEN 32
#define MAX_MSG_LEN 512
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char name[MAX_NAME_LEN];
    struct sockaddr_in addr;
    int active;
    time_t last_ping;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;

void signal_handler(int sig) {
    printf("\nZamykanie serwera...\n");
    close(server_socket);
    exit(0);
}

void broadcast_message(char* message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != sender_id) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_client(char* message, int client_id) {
    pthread_mutex_lock(&clients_mutex);
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id].active) {
        send(clients[client_id].socket, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&clients_mutex);
}

int find_client_by_name(char* name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_client(int id) {
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS && clients[id].active) {
        close(clients[id].socket);
        clients[id].active = 0;
        memset(clients[id].name, 0, MAX_NAME_LEN);
        client_count--;
        printf("Klient %s rozlaczony. Aktywnych klientow: %d\n", clients[id].name, client_count);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    int client_id = *((int*)arg);
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int bytes_received;
    
    while (clients[client_id].active) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(clients[client_id].socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("Otrzymano od %s: %s\n", clients[client_id].name, buffer);
        
        
        clients[client_id].last_ping = time(NULL);
        
        if (strncmp(buffer, "LIST", 4) == 0) {
            memset(response, 0, BUFFER_SIZE);
            strcat(response, "Aktywni klienci:\n");
            
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    char client_info[64];
                    snprintf(client_info, sizeof(client_info), "- %s\n", clients[i].name);
                    strcat(response, client_info);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            
            send(clients[client_id].socket, response, strlen(response), 0);
        }
        else if (strncmp(buffer, "2ALL ", 5) == 0) {
            time_t now = time(NULL);
            struct tm* tm_info = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
            
            snprintf(response, BUFFER_SIZE, "[%s] %s: %s\n", 
                    timestamp, clients[client_id].name, buffer + 5);
            
            broadcast_message(response, client_id);
        }
        else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char* space_pos = strchr(buffer + 5, ' ');
            if (space_pos != NULL) {
                *space_pos = '\0';
                char* target_name = buffer + 5;
                char* message = space_pos + 1;
                
                int target_id = find_client_by_name(target_name);
                if (target_id != -1) {
                    time_t now = time(NULL);
                    struct tm* tm_info = localtime(&now);
                    char timestamp[20];
                    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
                    
                    snprintf(response, BUFFER_SIZE, "[%s] %s (prywatnie): %s\n", 
                            timestamp, clients[client_id].name, message);
                    
                    send_to_client(response, target_id);
                    
                    snprintf(response, BUFFER_SIZE, "Wiadomosc wysłana do %s\n", target_name);
                    send(clients[client_id].socket, response, strlen(response), 0);
                } else {
                    snprintf(response, BUFFER_SIZE, "Klient %s nie zostal znaleziony\n", target_name);
                    send(clients[client_id].socket, response, strlen(response), 0);
                }
            }
        }
        else if (strncmp(buffer, "STOP", 4) == 0) {
            break;
        }
        else if (strncmp(buffer, "PING", 4) == 0) {
            send(clients[client_id].socket, "PONG\n", 5, 0);
        }
    }
    
    remove_client(client_id);
    return NULL;
}

void* ping_clients(void* arg) {
    while (1) {
        sleep(30); 
        
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                
                if (now - clients[i].last_ping > 60) {
                    printf("Klient %s nie odpowiada - usuwam\n", clients[i].name);
                    clients[i].active = 0;
                    close(clients[i].socket);
                    client_count--;
                } else {
                    
                    send(clients[i].socket, "ALIVE\n", 6, 0);
                }
            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uzycie: %s <port>\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    
    signal(SIGINT, signal_handler);
    
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }
    
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Blad tworzenia socketu");
        exit(1);
    }
    
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Blad bindowania");
        close(server_socket);
        exit(1);
    }
    
    
    if (listen(server_socket, 5) < 0) {
        perror("Blad nasluchiwania");
        close(server_socket);
        exit(1);
    }
    
    printf("Serwer uruchomiony na porcie %d\n", port);
    
    
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);
    
    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Blad akceptowania polaczenia");
            continue;
        }
        
        
        char client_name[MAX_NAME_LEN];
        int bytes_received = recv(client_socket, client_name, MAX_NAME_LEN - 1, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            continue;
        }
        client_name[bytes_received] = '\0';
        
        
        if (find_client_by_name(client_name) != -1) {
            char error_msg[] = "ERROR: Nazwa juz jest uzywana\n";
            send(client_socket, error_msg, strlen(error_msg), 0);
            close(client_socket);
            continue;
        }
        
        
        int client_id = -1;
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                client_id = i;
                clients[i].socket = client_socket;
                strncpy(clients[i].name, client_name, MAX_NAME_LEN - 1);
                clients[i].name[MAX_NAME_LEN - 1] = '\0';
                clients[i].addr = client_addr;
                clients[i].active = 1;
                clients[i].last_ping = time(NULL);
                client_count++;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        
        if (client_id == -1) {
            char error_msg[] = "ERROR: Serwer pelny\n";
            send(client_socket, error_msg, strlen(error_msg), 0);
            close(client_socket);
            continue;
        }
        
        printf("Nowy klient: %s (ID: %d). Aktywnych klientow: %d\n", 
               client_name, client_id, client_count);
        
        
        char welcome_msg[] = "OK: Polaczono z serwerem\n";
        send(client_socket, welcome_msg, strlen(welcome_msg), 0);
        
        
        pthread_t client_thread;
        int* client_id_ptr = malloc(sizeof(int));
        *client_id_ptr = client_id;
        pthread_create(&client_thread, NULL, handle_client, client_id_ptr);
        pthread_detach(client_thread);
    }
    
    close(server_socket);
    return 0;
}