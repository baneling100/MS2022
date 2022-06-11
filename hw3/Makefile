CC = g++
CXXFLAGS = -Wall -g -O0
OBJS = main.o barrier.o
TARGET = test
LDFLAGS = -pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) 16 10

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: clean
