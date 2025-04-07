#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int changes = 0;
int current_type = 0;
int timer_running = 0;

void int_handler(){
    printf("CRTL+C received!\n");
}

void sig_handler(int sig, siginfo_t *info, void *context){
    if (sig == SIGUSR1) {
        int val = info -> si_value.sival_int;
        pid_t sender_pid = info -> si_pid;

        printf("SIGUSR1 from %d with value: %d\n", sender_pid, val);
        
        if (val != current_type) {
            current_type = val;
            changes++;
            timer_running = current_type == 2;
        }

        switch (current_type) {
            case 1: {   printf("Type changes ammount: %d\n", changes);     break;}
            case 3: {   signal(SIGINT, SIG_IGN);                           break;}
            case 4: {   signal(SIGINT, int_handler);                       break;}
            case 5: {   exit(0);                                           break;}
            default:{   break;}
        }

        kill(sender_pid, SIGUSR1);
    }

}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_handler;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);

    int pid = getpid();
    while (1) {
        printf("PID catcher: %d\n", pid);
        int timer = 0;
        while(timer_running) {
            printf("%d\n", timer++);
            sleep(1);
        }
        pause();
    }
    return 0;
}