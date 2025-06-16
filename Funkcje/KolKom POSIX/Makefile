CFLAGS = -Wall
CC = gcc

receiver: 
	gcc receiver.c -o receiver -lrt

sender: 
	gcc sender.c -o sender -lrt

clean:
	rm -f sender.o sender receiver.o receiver squareFIFO

test:  sender receiver 
	@echo -e "\nTESTING\n"
	ipcs -q
	./sender 22 &
	ipcs -q
	./receiver
	ipcs -q
	./sender 9 &
	ipcs -q
	./receiver
	ipcs -q
	./sender -13 &
	ipcs -q
	./receiver
	ipcs -q
	./sender cos &
	ipcs -q
	./receiver
	ipcs -q
all: sender receiver
