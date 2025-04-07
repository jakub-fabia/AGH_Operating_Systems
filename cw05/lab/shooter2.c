#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int cnt = 0;
int cnt_sent = 0;

void handler(){
    cnt++;
}

int main(){
    pid_t child_pid = fork();
    signal(SIGUSR1, handler);
    while (1) {
        if (child_pid != 0){
            struct sigaction action;

            action.sa_flags = SA_SIGINFO;
            action.sa_sigaction = &sighand;
    
            if (sigaction(SIGUSR2, &action, NULL) == -1) {
                    perror("sigusr: sigaction");
                    return 0;
            }
            cnt_sent++;
            kill(0, SIGUSR1);
            printf("%d %d\n", cnt, cnt_sent);
        }
    }
    return 0;
}