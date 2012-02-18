CC=gcc
CFLAGS=-Wall -std=c99 -pedantic -g

all: test_buffer

test_buffer: test_buffer.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ ../src/*.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o

FRC:
.SUFFIXES: .c
