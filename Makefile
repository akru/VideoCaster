CC = gcc
CLIBS = -lv4l2 -ljpeg
SLIBS = -lmemcached -lpthread

all: vcap vcaps

jcomp.o: jcomp.c jcomp.h
	$(CC) -c jcomp.c

vcap.o: vcap.c jcomp.h proto.h
	$(CC) -c vcap.c

vcaps.o: vcaps.c proto.h
	$(CC) -c vcaps.c

vcap: vcap.o jcomp.o
	$(CC) $(CLIBS) $^ -o $@

vcaps: vcaps.o
	$(CC) $(SLIBS) $^ -o $@

clean:
	rm *.o vcap vcaps
