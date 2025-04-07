#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int cnt = 0;

void handler(){
    cnt++;
    printf("%d\n", cnt);
}

int main(){
    printf("%d\n", getpid());
    while (1) {
        signal(SIGUSR1, handler);
        pause();
    }
    return 0;
}