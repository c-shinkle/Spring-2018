CC=gcc
CFLAGS=-I.
DEPS = stdio.h pthread.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: q1.o
	$(CC) -o q1 q1.o -I. -lpthread
clean:
	rm *~ *.o
