CC = gcc
CFLAGS = -g -Wall

emulator: emulator.c emulator.h mem.o
	$(CC) $(CFLAGS) emulator.c mem.o -o emulator

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c mem.c

clean:
	rm emulator *.o