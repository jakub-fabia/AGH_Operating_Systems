#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void sig_handler(){
    printf("Received SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s :pid\n", argv[0]);
        return 1;
    }
    pid_t pid = atol(argv[1]);
    int val;
    signal(SIGUSR1, sig_handler);
    printf("Type a value 1-5:\n");

    while (scanf("%d", &val) == 1) {
        if (val < 1 || val > 5) {
            continue;
        }
        union sigval sig_value;
        sig_value.sival_int = val;

        sigqueue(pid, SIGUSR1, sig_value);
        printf("Sent SIGUSR1 with value %d\n", val);
        pause();
    }
    return 0;
}