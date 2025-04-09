#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int pipefd[2];
    pipe(pipefd);

    if (fork() == 0) {
        close(pipefd[1]);
        sleep(1);
        char c;
        while (read(pipefd[0], &c, 1) > 0) {
            printf("%c", c);
        }
        printf("END\n");
    } else {
        close(pipefd[0]);
        write(pipefd[1], "ABC", 3);
    }
    return 0;
}