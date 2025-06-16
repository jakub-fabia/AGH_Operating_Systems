#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KEY "."

struct msgbuf {
    long mtype;
    int mtext;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    key_t key = ftok(KEY, 'A');
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("msgget");
        exit(1);
    }

    struct msgbuf msg;
    msg.mtype = 1;
    msg.mtext = atoi(argv[1]);

    if (msgsnd(msgid, &msg, sizeof(int), 0) == -1)
    {
        perror("msgsnd");
        exit(1);
    }

    return 0;
}
