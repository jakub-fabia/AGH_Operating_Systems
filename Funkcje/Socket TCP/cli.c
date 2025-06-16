#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

#define L_PYTAN 10
#define PORT 12345

int main(void){
    int status, gniazdo, i;
    struct sockaddr_in ser;
    char buf[200];
    char pytanie[] = "abccbahhhh";

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo < 0) { perror("socket"); return 1; }

    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser.sin_port = htons(PORT);

    status = connect(gniazdo, (struct sockaddr*)&ser, sizeof(ser));
    if (status < 0) { printf("blad 01\n"); return 1; }

    for (i = 0; i < L_PYTAN; i++){
        status = write(gniazdo, pytanie + i, 1);
        if (status < 0) { perror("write"); break; }

        status = read(gniazdo, buf, sizeof(buf));
        if (status > 0) {
            buf[status] = '\0';
            printf("%s ", buf);
        }
    }

    printf("\n");
    close(gniazdo);
    printf("KONIEC DZIALANIA KLIENTA\n");
    return 0;
}
