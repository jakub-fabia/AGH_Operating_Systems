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
#define TIMEOUT_SECONDS 60

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
    char name[MAX_NAME_LEN];
    int active;
    time_t last_ping;
    int registered;
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
            sendto(server_socket, message, strlen(message), 0, 
                   (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_client(char* message, int client_id) {
    pthread_mutex_lock(&clients_mutex);
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id].active) {
        sendto(server_socket, message, strlen(message), 0, 
               (struct sockaddr*)&clients[client_id].addr, clients[client_id].addr_len);
    }
    pthread_mutex_unlock(&clients_mutex);
}

int find_client_by_addr(struct sockaddr_in* addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

int find_client_by_name(char* name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int register_client(char* name, struct sockaddr_in* addr, socklen_t addr_len) {
    pthread_mutex_lock(&clients_mutex);
    
    if (find_client_by_name(name) != -1) {
        pthread_mutex_unlock(&clients_mutex);
        return -2;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].addr = *addr;
            clients[i].addr_len = addr_len;
            strncpy(clients[i].name, name, MAX_NAME_LEN - 1);
            clients[i].name[MAX_NAME_LEN - 1] = '\0';
            clients[i].active = 1;
            clients[i].registered = 1;
            clients[i].last_ping = time(NULL);
            client_count++;
            
            printf("Nowy klient: %s (%s:%d). Aktywnych klientow: %d\n", 
                   name, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), client_count);
            
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

void remove_client(int client_id) {
    pthread_mutex_lock(&clients_mutex);
    if (client_id >= 0 && client_id < MAX_CLIENTS && clients[client_id].active) {
        printf("Klient %s rozlaczony. Aktywnych klientow: %d\n", 
               clients[client_id].name, client_count - 1);
        
        clients[client_id].active = 0;
        clients[client_id].registered = 0;
        memset(clients[client_id].name, 0, MAX_NAME_LEN);
        client_count--;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* ping_clients(void* arg) {
    while (1) {
        sleep(30);
        
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                if (now - clients[i].last_ping > TIMEOUT_SECONDS) {
                    printf("Klient %s nie odpowiada - usuwam\n", clients[i].name);
                    clients[i].active = 0;
                    clients[i].registered = 0;
                    client_count--;
                } else {
                    char ping_msg[] = "ALIVE";
                    sendto(server_socket, ping_msg, strlen(ping_msg), 0, 
                           (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
                }
            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void handle_message(char* buffer, struct sockaddr_in* client_addr, socklen_t addr_len) {
    char response[BUFFER_SIZE];
    int client_id = find_client_by_addr(client_addr);
    
    if (strncmp(buffer, "REGISTER ", 9) == 0) {
        char* client_name = buffer + 9;
        
        int result = register_client(client_name, client_addr, addr_len);
        if (result >= 0) {
            strcpy(response, "OK: Zarejestrowano pomyslnie");
        } else if (result == -2) {
            strcpy(response, "ERROR: Nazwa juz jest uzywana");
        } else {
            strcpy(response, "ERROR: Serwer pelny");
        }
        
        sendto(server_socket, response, strlen(response), 0, 
               (struct sockaddr*)client_addr, addr_len);
        return;
    }
    
    if (client_id == -1) {
        strcpy(response, "ERROR: Niezarejestrowany klient");
        sendto(server_socket, response, strlen(response), 0, 
               (struct sockaddr*)client_addr, addr_len);
        return;
    }
    
    clients[client_id].last_ping = time(NULL);
    
    printf("Otrzymano od %s: %s\n", clients[client_id].name, buffer);
    
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
        
        sendto(server_socket, response, strlen(response), 0, 
               (struct sockaddr*)client_addr, addr_len);
    }
    else if (strncmp(buffer, "2ALL ", 5) == 0) {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        
        snprintf(response, BUFFER_SIZE, "[%s] %s: %s", 
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
                
                snprintf(response, BUFFER_SIZE, "[%s] %s (prywatnie): %s", 
                        timestamp, clients[client_id].name, message);
                
                send_to_client(response, target_id);
                
                snprintf(response, BUFFER_SIZE, "Wiadomosc wyslana do %s", target_name);
                sendto(server_socket, response, strlen(response), 0, 
                       (struct sockaddr*)client_addr, addr_len);
            } else {
                snprintf(response, BUFFER_SIZE, "Klient %s nie zostal znaleziony", target_name);
                sendto(server_socket, response, strlen(response), 0, 
                       (struct sockaddr*)client_addr, addr_len);
            }
        }
    }
    else if (strncmp(buffer, "QUIT", 4) == 0) {
        strcpy(response, "OK: Do widzenia");
        sendto(server_socket, response, strlen(response), 0, 
               (struct sockaddr*)client_addr, addr_len);
        remove_client(client_id);
    }
    else if (strncmp(buffer, "PING", 4) == 0) {
        strcpy(response, "PONG");
        sendto(server_socket, response, strlen(response), 0, 
               (struct sockaddr*)client_addr, addr_len);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uzycie: %s <port>\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    signal(SIGINT, signal_handler);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        clients[i].registered = 0;
    }
    
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
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
    
    printf("Serwer UDP uruchomiony na porcie %d\n", port);
    
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0, 
                                     (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received < 0) {
            perror("Blad odbierania danych");
            continue;
        }
        
        buffer[bytes_received] = '\0';
        handle_message(buffer, &client_addr, client_len);
    }
    
    close(server_socket);
    return 0;
}