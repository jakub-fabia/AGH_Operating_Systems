CC = gcc
CFLAGS = -Wall -O2

zad1: zad1.c
	$(CC) $(CFLAGS) -o zad1 zad1.c

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

all: zad1 sender receiver

clean:
	rm -f zad1 sender receiver *.o *.a *.so

.PHONY: clean