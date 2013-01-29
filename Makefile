CC = gcc

all: vcap vcaps

vcap: vcap.c
	$(CC) -lv4l2 $^ -o $@

vcaps: vcaps.c
	$(CC) $^ -o $@

clean:
	rm vcap vcaps
