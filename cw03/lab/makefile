CC = gcc
CFLAGS = -Wall -O2 -std=c17

readCompare: readCompare.c
	$(CC) $(CFLAGS) -o readCompare readCompare.c
	
readOneLetter: readOneLetter.c
	$(CC) $(CFLAGS) -o readOneLetter readOneLetter.c

all: readCompare readOneLetter

clean:
	rm -f readCompare readOneLetter *.o *.a *.so

.PHONY: clean