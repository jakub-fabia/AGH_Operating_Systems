#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define QUEUE_NAME "/myqueue"

int main() {
    sleep(1); // poczekaj, aż sender utworzy i wyśle

    mqd_t mq;
    int val;
    struct mq_attr attr;

    // Atrybuty kolejki
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    // Otwórz istniejącą kolejkę (lub utwórz)
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(1);
    }
    
    if (mq_receive(mq, (char*)&val, sizeof(int), NULL) == -1) {
        perror("mq_receive");
    }

    printf("%d square is: %d\n", val, val * val);

    // Sprzątanie
    mq_close(mq);
    mq_unlink(QUEUE_NAME); // usuń kolejkę

    return 0;
}
