CFLAGS=-g -Wall -pipe

all: bidilink

bidilink: bidilink.o std.o exec.o stream.o client-tty.o server-tty.o client-tcp.o server-tcp.o client-unix.o server-unix.o
	$(CC) $^ -o $@

clean:
	rm -f *.o bidilink

.PHONY: all clean
