#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void handler() {
    printf("To jest komunikat z handler na SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <none/ignore/handler/mask>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    }
    else if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, handler);
    }
    else if (strcmp(argv[1], "mask") == 0) {
        sigset_t new_mask, old_mask;
        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1); 
        sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    }
    else if (strcmp(argv[1], "none") != 0) {
        printf("Usage: %s <none/ignore/handler/mask>\n", argv[0]);
        return 1;
    }

    kill(getpid(), SIGUSR1);

    if (strcmp(argv[1], "mask") == 0) {
        sigset_t pending_signals;
        sigpending(&pending_signals);
        if (sigismember(&pending_signals, SIGUSR1)) {
            printf("Sygnał SIGUSR1 jest oczekujący.\n");
        } else {
            printf("Sygnał SIGUSR1 nie jest oczekujący.\n");
        }
    }
    return 0;
}