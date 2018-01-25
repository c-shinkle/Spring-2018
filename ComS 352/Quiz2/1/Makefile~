CC=gcc
CFLAGS=-I.
DEPS = stdio.h pthread.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: ex1.o
	$(CC) -o ex1 ex1.o -I. -lpthread
clean:
	rm *~ *.o
