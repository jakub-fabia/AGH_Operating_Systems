#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/my_fifo"
#define MESSAGE "Hello from sender!"

int main() {
    mkfifo(FIFO_PATH, 0666);
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    write(fifo_fd, MESSAGE, strlen(MESSAGE) + 1);
    close(fifo_fd);
    return 0;
}