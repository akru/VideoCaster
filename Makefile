CC = gcc
CLIBS = -lv4l2 -ljpeg
SLIBS = -lmemcached -lpthread

all: vcast vserv

jcomp.o: jcomp.c jcomp.h
	$(CC) -c jcomp.c

vcast.o: vcast.c jcomp.h proto.h
	$(CC) -c vcast.c

vserv.o: vserv.c proto.h
	$(CC) -c vserv.c

vcast: vcast.o jcomp.o
	$(CC) $(CLIBS) $^ -o $@

vserv: vserv.o
	$(CC) $(SLIBS) $^ -o $@

clean:
	rm *.o vcap vcaps
