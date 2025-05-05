#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

double f(double x) {
    return 4.0 / (x * x + 1.0);
}

double rectangle_area(double a, double b, double dx) {
    double sum = 0.0;
    for (double x = a; x < b; x += dx) {
        sum += f(x) * dx;
    }
    return sum;
}

int main() {
    const char *fifo_in = "/tmp/fifo_in";
    const char *fifo_out = "/tmp/fifo_out";

    mkfifo(fifo_in, 0666);
    mkfifo(fifo_out, 0666);

    int fd_in = open(fifo_in, O_RDONLY);
    if (fd_in < 0) {
        perror("open fifo_in");
        exit(1);
    }

    double a, b;
    if (read(fd_in, &a, sizeof(double)) <= 0 ||
        read(fd_in, &b, sizeof(double)) <= 0) {
        perror("read a or b");
        close(fd_in);
        exit(1);
    }
    close(fd_in);

    double dx = 0.000001;
    double result = rectangle_area(a, b, dx);

    int fd_out = open(fifo_out, O_WRONLY);
    if (fd_out < 0) {
        perror("open fifo_out");
        exit(1);
    }

    write(fd_out, &result, sizeof(double));
    close(fd_out);

    return 0;
}
