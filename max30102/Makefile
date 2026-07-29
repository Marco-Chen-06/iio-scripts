CC      = gcc
CFLAGS += -g -O0 -Wall -Wextra -std=gnu11
TARGET  = max30102_read
OBJS    = main.o max30102_read.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c max30102_read.h
	$(CC) $(CFLAGS) -c main.c

max30102_read.o: max30102_read.c max30102_read.h
	$(CC) $(CFLAGS) -c max30102_read.c

clean:
	rm -f $(TARGET) *.o

.PHONY: clean