CC = gcc
CFLAGS = -Wall -O2 -std=c17

ex1: ex1.c
	$(CC) $(CFLAGS) -o ex1 ex1.c

ex2: ex2.c
	$(CC) $(CFLAGS) -o ex2 ex2.c

all: ex1 ex2

clean:
	rm -f ex2 ex1 *.o *.a *.so

.PHONY: clean