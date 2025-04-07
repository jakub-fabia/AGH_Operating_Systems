#include <signal.h>
#include <stdio.h>

int main() {
    signal(SIGUSR1, SIG_IGN);
    raise(SIGUSR1);
    raise(SIGUSR2);
    printf("asd\n");
    return 0;
}