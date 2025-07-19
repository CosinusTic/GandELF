CC=gcc
CFLAGS=-std=c99 -Werror -Wall -Wextra
DEBUG_FLAGS=-g 
TEST_FLAGS=-O0 -fno-omit-frame-pointer 

TARGET=bin/gandelf
TARGET_TEST=test/test

SRC=src/main.c
SRC_TEST=test/test.c

TRASH=src/*.o bin

all: $(TARGET)

$(TARGET): clean
	mkdir bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug: clean
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

test: clean
	$(CC) $(CFLAGS) $(TEST_FLAGS)  $(SRC_TEST) -o $(TARGET_TEST)

clean:
	rm -rf $(TRASH) $(TARGET) $(TARGET_TEST)

