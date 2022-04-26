CC = g++
CFLAGS = -Wall -g -O3
LDFLAGS = -pthread

all: lock

run: lock
	./lock

measure: lock.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -rf lock lock.o

.PHONY: clean
