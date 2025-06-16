#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define MAX_NAME_LEN 32
#define BUFFER_SIZE 1024

int client_socket;
char client_name[MAX_NAME_LEN];
int running = 1;
struct sockaddr_in server_addr;
socklen_t server_len = sizeof(server_addr);

void signal_handler(int sig) {
    printf("\nWylogowywanie z serwera...\n");
    char stop_msg[] = "QUIT";
    sendto(client_socket, stop_msg, strlen(stop_msg), 0, 
           (struct sockaddr*)&server_addr, server_len);
    running = 0;
    close(client_socket);
    exit(0);
}

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0, 
                                 (struct sockaddr*)&from_addr, &from_len);
        
        if (bytes_received <= 0) {
            if (running) {
                printf("Blad komunikacji z serwerem\n");
                running = 0;
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        if (strncmp(buffer, "ALIVE", 5) == 0) {
            char pong[] = "PING";
            sendto(client_socket, pong, strlen(pong), 0, 
                   (struct sockaddr*)&server_addr, server_len);
            continue;
        }
        
        if (strncmp(buffer, "OK:", 3) == 0 || strncmp(buffer, "ERROR:", 6) == 0 || 
            strncmp(buffer, "Aktywni klienci:", 16) == 0 || 
            strncmp(buffer, "Wiadomosc wyslana", 17) == 0 ||
            strncmp(buffer, "Klient ", 7) == 0) {
            printf("%s\n", buffer);
        } else {
            printf("%s\n", buffer);
        }
        
        fflush(stdout);
    }
    
    return NULL;
}

void print_help() {
    printf("\nDostepne komendy:\n");
    printf("LIST - wyswietl liste aktywnych klientow\n");
    printf("2ALL <wiadomosc> - wyslij wiadomosc do wszystkich\n");
    printf("2ONE <nazwa_klienta> <wiadomosc> - wyslij prywatna wiadomosc\n");
    printf("HELP - wyswietl te pomoc\n");
    printf("QUIT - zakoncz program\n\n");
}

int register_with_server() {
    char register_msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    snprintf(register_msg, BUFFER_SIZE, "REGISTER %s", client_name);
    
    if (sendto(client_socket, register_msg, strlen(register_msg), 0, 
               (struct sockaddr*)&server_addr, server_len) < 0) {
        perror("Blad wysylania zadania rejestracji");
        return -1;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Blad ustawiania timeout");
        return -1;
    }
    
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    int bytes_received = recvfrom(client_socket, response, BUFFER_SIZE - 1, 0, 
                                 (struct sockaddr*)&from_addr, &from_len);
    
    if (bytes_received <= 0) {
        printf("Brak odpowiedzi od serwera\n");
        return -1;
    }
    
    response[bytes_received] = '\0';
    printf("%s\n", response);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    if (strncmp(response, "OK:", 3) == 0) {
        return 0;
    } else {
        return -1;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Uzycie: %s <nazwa_klienta> <adres_IP> <port>\n", argv[0]);
        exit(1);
    }
    
    strncpy(client_name, argv[1], MAX_NAME_LEN - 1);
    client_name[MAX_NAME_LEN - 1] = '\0';
    
    char* server_ip = argv[2];
    int server_port = atoi(argv[3]);
    
    signal(SIGINT, signal_handler);
    
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Blad tworzenia socketu");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Nieprawidlowy adres IP\n");
        close(client_socket);
        exit(1);
    }
    
    printf("laczenie z serwerem %s:%d...\n", server_ip, server_port);
    if (register_with_server() < 0) {
        close(client_socket);
        exit(1);
    }
    
    printf("Witaj %s! Polaczono z serwerem UDP %s:%d\n", client_name, server_ip, server_port);
    print_help();
    
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, NULL);
    
    char input[BUFFER_SIZE];
    while (running) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(input, BUFFER_SIZE, stdin)) {
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "QUIT") == 0) {
            char stop_msg[] = "QUIT";
            sendto(client_socket, stop_msg, strlen(stop_msg), 0, 
                   (struct sockaddr*)&server_addr, server_len);
            break;
        }
        
        if (strcmp(input, "HELP") == 0) {
            print_help();
            continue;
        }
        
        if (sendto(client_socket, input, strlen(input), 0, 
                   (struct sockaddr*)&server_addr, server_len) < 0) {
            printf("Blad wysylania wiadomosci\n");
            break;
        }
    }
    
    running = 0;
    pthread_join(receive_thread, NULL);
    close(client_socket);
    printf("Rozlaczono z serwerem\n");
    
    return 0;
}