CC=gcc
CFLAGS=-I.
DEPS = stdio.h uinstd.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

forkmake: fork_example.o
	$(CC) -o fork_exe fork_example.o -I.
clean:
	rm *~ *.o
