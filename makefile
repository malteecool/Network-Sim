CC=gcc
CFLAGS=-I.
DEPS = node.h join.h hash.h hashtable.h val.h shutdown.h update.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

node: node.o join.o hash.o hashtable.o val.o shutdown.o update.o
	$(CC) -o node node.o join.o hash.o hashtable.o val.o shutdown.o update.o

clean:
	rm -f *.o 
