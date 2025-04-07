#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void handler(int sig){
    printf("Ja cie ale sygnal\n");
}

int main(){
    signal(SIGUSR1, handler);
    printf("%d\n", getpid());
    pause();
    return 0;
}