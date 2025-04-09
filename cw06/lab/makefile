CC = gcc
CFLAGS = -Wall -O2

before1: before1.c
	$(CC) $(CFLAGS) -o before1 before1.c

before2: before2.c
	$(CC) $(CFLAGS) -o before2 before2.c

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

all: before1 before2 sender receiver

clean:
	rm -f before1 before2 sender receiver *.o *.a *.so

.PHONY: clean