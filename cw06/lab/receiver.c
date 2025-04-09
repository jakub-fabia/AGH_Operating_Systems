#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/my_fifo"

int main() {
    mkfifo(FIFO_PATH, 0666);
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    char buffer[128];
    ssize_t bytes_read = read(fifo_fd, buffer, 128);
    printf("%s\n", buffer);
    close(fifo_fd);
    return 0;
}