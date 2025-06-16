#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUEUE_NAME "/myqueue"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    int val = atoi(argv[1]);

    mqd_t mq;
    struct mq_attr attr;

    // Atrybuty kolejki
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    // Utwórz lub otwórz kolejkę
    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        return 1;
    }

    // Wyślij wartość
    if (mq_send(mq, (char*)&val, sizeof(int), 0) == -1) {
        perror("mq_send");
        return 1;
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    return 0;
}
