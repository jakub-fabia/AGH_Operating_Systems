CC = gcc
CFLAGS = -Wall -O2

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lpthread

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lpthread

all: server client

clean:
	rm -f server client *.o *.a *.so

.PHONY: clean