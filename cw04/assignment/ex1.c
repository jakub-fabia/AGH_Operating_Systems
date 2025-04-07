#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int child() {
    printf("Child PID: %d, Parent PID: %d\n", (int)getpid(), (int)getppid());
    exit(0);
 }

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong amount of arguments!\n");
        return 1;
    }
    
    int value = atoi(argv[1]);
    pid_t child_pid;

    for (int i = 0; i < value; i++) {
        child_pid = fork();

        if (child_pid == 0) {
            child();
        }
    }

    for (int i = 0; i < value; i++) {
        wait(NULL);
    }

    printf("Parent PID: %d\n", (int)getpid());
    return 0;
}