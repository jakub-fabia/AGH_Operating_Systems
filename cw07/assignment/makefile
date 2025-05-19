CC = gcc
CFLAGS = -Wall -O2

server: server.c
	$(CC) -o server server.c

client: client.c
	$(CC) -o client client.c

all: client server

clean:
	rm -f client server *.o *.a *.so

.PHONY: clean