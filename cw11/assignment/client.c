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

void signal_handler(int sig) {
    printf("\nWylogowywanie z serwera...\n");
    char stop_msg[] = "STOP";
    send(client_socket, stop_msg, strlen(stop_msg), 0);
    running = 0;
    close(client_socket);
    exit(0);
}

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (running) {
                printf("Polaczenie z serwerem zostalo przerwane\n");
                running = 0;
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        
        if (strncmp(buffer, "ALIVE", 5) == 0) {
            char pong[] = "PING";
            send(client_socket, pong, strlen(pong), 0);
            continue;
        }
        
        printf("%s", buffer);
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
    printf("QUIT - zakończ program\n\n");
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
    
    struct sockaddr_in server_addr;
    
    
    signal(SIGINT, signal_handler);
    
    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
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
    
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Blad polaczenia z serwerem");
        close(client_socket);
        exit(1);
    }
    
    
    send(client_socket, client_name, strlen(client_name), 0);
    
    
    char response[BUFFER_SIZE];
    int bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Blad komunikacji z serwerem\n");
        close(client_socket);
        exit(1);
    }
    
    response[bytes_received] = '\0';
    printf("%s", response);
    
    if (strncmp(response, "ERROR", 5) == 0) {
        close(client_socket);
        exit(1);
    }
    
    printf("Witaj %s! Polaczono z serwerem %s:%d\n", client_name, server_ip, server_port);
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
            char stop_msg[] = "STOP";
            send(client_socket, stop_msg, strlen(stop_msg), 0);
            break;
        }
        
        if (strcmp(input, "HELP") == 0) {
            print_help();
            continue;
        }
        
        
        if (send(client_socket, input, strlen(input), 0) < 0) {
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