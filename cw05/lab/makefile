CC = gcc
CFLAGS = -Wall -O2 -std=c17

main: main.c
	$(CC) $(CFLAGS) -o main main.c

reciever: reciever.c
	$(CC) $(CFLAGS) -o reciever reciever.c

reciever2: reciever2.c
	$(CC) $(CFLAGS) -o reciever2 reciever2.c

shooter: shooter.c
	$(CC) $(CFLAGS) -o shooter shooter.c

shooter2: shooter2.c
	$(CC) $(CFLAGS) -o shooter2 shooter2.c

all: main reciever reciever2 shooter shooter2

clean:
	rm -f main reciever reciever2 shooter shooter2 *.o *.a *.so

.PHONY: clean