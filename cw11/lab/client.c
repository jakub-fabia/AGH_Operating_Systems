#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#define UNIX_PATH_MAX 108

int main() {
    int fd = -1;
    if((fd = socket(AF_UNIX, SOCK_DGRAM,0)) == -1) {
        perror("Error crating socket");
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';

    if(connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("Error connecting to server");
    }

    char buf[20];
    int to_send = sprintf(buf, "HELLO From: %u", getpid());

    if(write(fd,buf, to_send+1) == -1) {
        perror("Error sending a message");
    }

    shutdown(fd, SHUT_RDWR);
    return 0;
}