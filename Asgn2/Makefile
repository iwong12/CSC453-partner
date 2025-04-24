CC = gcc
CFLAGS = -Wall -g -fpic

.PHONY: clean

rr.o: rr_scheduler.c
	$(CC) $(CFLAGS) -c rr_scheduler.c -o rr.o

queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c -o queue.o

lwp.o: lwp.c
	$(CC) $(CFLAGS) -c lwp.c -o lwp.o

magic64.o: magic64.S
	$(CC) $(CFLAGS) -c magic64.S -o magic64.o

liblwp.so: lwp.o rr.o queue.o magic64.o
	$(CC) $(CFLAGS) -shared -fPIC -o liblwp.so lwp.o rr.o queue.o magic64.o

clean:
	rm -f *.o *.so -r

