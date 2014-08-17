CC=gcc
CFLAGS=-Wall -Ofast -mtune=native -fstack-protector-all -lX11 -lXtst 
BINDIR=/usr/bin

googler2: IR_remote.c 
	$(CC) $(CFLAGS) -g -c *.c 
	$(CC) $(CFLAGS) -o IR_remote *.o 
	rm *.o

clean:
	rm IR_remote
