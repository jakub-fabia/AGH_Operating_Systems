#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define KEY "."

struct msgbuf
{
	long mtype;
	int mtext;
};

int main()
{
	sleep(1);

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

	if (msgrcv(msgid, &msg, sizeof(int), 1, 0) == -1)
	{
		perror("msgrcv");
		exit(1);
	}

	printf("%d square is: %d\n", msg.mtext, msg.mtext * msg.mtext);

	msgctl(msgid, IPC_RMID, NULL);
	return 0;
}
