CC = gcc
CFLAGS = -Wall -O2

ex1: ex1.c
	$(CC) $(CFLAGS) -o ex1 ex1.c

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

catcher: catcher.c
	$(CC) $(CFLAGS) -o catcher catcher.c

all: ex1 catcher sender

clean:
	rm -f ex1 catcher sender *.o *.a *.so

.PHONY: clean