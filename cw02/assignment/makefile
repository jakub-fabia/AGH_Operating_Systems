CC = gcc
CFLAGS = -Wall -O2 -std=c17

OBJECT = collatz.o
STATIC_LIB = libcollatz.a
SHARED_LIB = libcollatz.so

$(OBJECT): collatz.c
	$(CC) -c collatz.c

$(STATIC_LIB): $(OBJECT)
	ar rcs $(STATIC_LIB) $(OBJECT)

$(SHARED_LIB): $(OBJECT)
	$(CC) -shared -fPIC -o $(SHARED_LIB) $(OBJECT)

client_static: client.c $(STATIC_LIB)
	$(CC) $(CFLAGS) -o client_static client.c -L. -lcollatz

client_shared: client.c $(SHARED_LIB)
	$(CC) $(CFLAGS) -o client_shared client.c -L. -lcollatz

client_dynamic: client.c $(SHARED_LIB)
	$(CC) $(CFLAGS) -DDYNAMIC -o client_dynamic client.c -ldl

all: client_dynamic client_shared client_static

clean:
	rm -f client_static client_dynamic client_shared *.o *.a *.so

.PHONY: clean