#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

double f(double x) {
    return 4.0 / (x * x + 1.0);
}

double rectangle_area(double start, double end, double dx) {
    double sum = 0.0;
    for (double x = start; x < end; x += dx) {
        sum += f(x) * dx;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dx> <n>\n", argv[0]);
        return 1;
    }

    double dx = atof(argv[1]);
    int max_k = atoi(argv[2]);

    for (int k = 1; k <= max_k; ++k) {
        int pipes[k][2];
        pid_t pids[k];
        struct timeval start, end;

        gettimeofday(&start, NULL);

        double step = 1.0 / k;

        for (int i = 0; i < k; ++i) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(1);
            }

            pids[i] = fork();
            if (pids[i] < 0) {
                perror("fork");
                exit(1);
            } else if (pids[i] == 0) {
                close(pipes[i][0]);
                double a = i * step;
                double b = (i + 1) * step;
                double result = rectangle_area(a, b, dx);
                write(pipes[i][1], &result, sizeof(double));
                close(pipes[i][1]);
                exit(0);
            } else {
                close(pipes[i][1]);
            }
        }

        double total = 0.0;
        for (int i = 0; i < k; ++i) {
            double partial = 0.0;
            read(pipes[i][0], &partial, sizeof(double));
            close(pipes[i][0]);
            total += partial;
            waitpid(pids[i], NULL, 0);
        }

        gettimeofday(&end, NULL);
        double time_taken = (end.tv_sec - start.tv_sec) + 
                            (end.tv_usec - start.tv_usec) / 1e6;

        printf("k = %d, result = %.12f, time = %.4f s\n", k, total, time_taken);
    }

    return 0;
}
