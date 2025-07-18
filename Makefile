CC=gcc
CFLAGS=-std=c99 -Werror -Wall
DEBUG_FLAGS=-g 

TARGET=bin/dizzy
SRC=src/main.c

all: $(TARGET)

$(TARGET): clean
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf src/*.o $(TARGET)

