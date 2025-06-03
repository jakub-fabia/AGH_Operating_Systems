CC = gcc
CFLAGS = -Wall -O2

client: client.c
	$(CC) -o client client.c

server: server.c
	$(CC) -o server server.c

all: server client

clean:
	rm -f server client *.o *.a *.so

.PHONY: clean