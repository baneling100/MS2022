CC = g++
CFLAGS = -Wall -g -O0 -pthread
LDFLAGS =

all: measure

run: measure
	./measure

measure: measure.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -rf measure measure.o

.PHONY: clean
