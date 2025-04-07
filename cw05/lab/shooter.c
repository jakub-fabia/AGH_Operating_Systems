#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    int value = atoi(argv[1]);
    kill(value, SIGUSR1);
    return 0;
}