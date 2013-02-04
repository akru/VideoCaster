CC = gcc
CLIBS = -lv4l2 -ljpeg

all: vcap vcaps

jcomp.o: jcomp.c jcomp.h
	$(CC) -c jcomp.c

vcap.o: vcap.c jcomp.h
	$(CC) -c vcap.c

vcap: vcap.o jcomp.o
	$(CC) $(CLIBS) $^ -o $@

vcaps: vcaps.c
	$(CC) $^ -o $@

clean:
	rm *.o vcap vcaps
