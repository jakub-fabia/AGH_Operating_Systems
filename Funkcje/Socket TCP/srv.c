#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

#define L_SLOW 8
#define PORT 12345

int main(void){
    int status, gniazdo, dlugosc, gniazdo2, lbajtow, i;
    struct sockaddr_in ser, cli;
    char buf[200];
    char pytanie[L_SLOW] = "abcdefgh";
    char odpowiedz[L_SLOW][10] = {"alfa", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel"};

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo < 0) { perror("socket"); return 1; }

    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser.sin_port = htons(PORT);
    status = bind(gniazdo, (struct sockaddr*)&ser, sizeof(ser));
    if (status < 0) { perror("bind"); return 1; }

    status = listen(gniazdo, 10);
    if (status < 0) { perror("listen"); return 1; }

    while (1){
        dlugosc = sizeof(cli);
        gniazdo2 = accept(gniazdo, (struct sockaddr*)&cli, (socklen_t*)&dlugosc);
        if (gniazdo2 == -1) { printf("blad 03\n"); return 1; }

        lbajtow = 1;
        while (lbajtow > 0){
            lbajtow = read(gniazdo2, buf, 1);
            if (lbajtow > 0){
                for (i = 0; i < L_SLOW && pytanie[i] != buf[0]; i++);
                if (i < L_SLOW)
                    write(gniazdo2, odpowiedz[i], strlen(odpowiedz[i]));
            }
        }
        close(gniazdo2);
    }

    close(gniazdo);
    printf("KONIEC DZIALANIA SERWERA\n");
    return 0;
}
