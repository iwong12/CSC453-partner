CC = gcc
CFLAGS = -Wall -g -fpic

.PHONY: clean

lwp.o: lwp.c
	$(CC) $(CFLAGS) -c lwp.c -o lwp.o

liblwp.so: lwp.o
	$(CC) $(CFLAGS) -shared -fPIC -o libmalloc.so malloc.o

clean:
	rm -f *.o *.so *.a -r lib*

