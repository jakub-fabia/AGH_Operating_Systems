CC = gcc
CFLAGS = -Wall -O2 -std=c17

flipper: flipper.c
	$(CC) $(CFLAGS) -o flipper flipper.c

all: flipper

clean:
	rm -f flipper readOneLetter *.o *.a *.so

.PHONY: clean