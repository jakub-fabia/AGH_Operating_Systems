#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    const char *fifo_in = "/tmp/fifo_in";
    const char *fifo_out = "/tmp/fifo_out";

    mkfifo(fifo_in, 0666);
    mkfifo(fifo_out, 0666);

    double a, b;
    printf("Input integration area [a b]: ");
    if (scanf("%lf %lf", &a, &b) != 2) {
        fprintf(stderr, "Error: Invalid entry data\n");
        return 1;
    }

    int fd_in = open(fifo_in, O_WRONLY);
    if (fd_in < 0) {
        perror("open fifo_in");
        exit(1);
    }

    write(fd_in, &a, sizeof(double));
    write(fd_in, &b, sizeof(double));
    close(fd_in);

    int fd_out = open(fifo_out, O_RDONLY);
    if (fd_out < 0) {
        perror("open fifo_out");
        exit(1);
    }

    double result;
    if (read(fd_out, &result, sizeof(double)) <= 0) {
        perror("read result");
        close(fd_out);
        exit(1);
    }

    close(fd_out);
    printf("Integration result [%.6f, %.6f] = %.17f\n", a, b, result);

    unlink(fifo_in);
    unlink(fifo_out);

    return 0;
}
