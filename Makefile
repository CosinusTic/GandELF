CC=gcc
CFLAGS=-std=c99 -Werror -Wall -Wextra
DEBUG_FLAGS=-g 
TEST_FLAGS=-O0 -fno-omit-frame-pointer

TARGET=bin/gandelf
TARGET_TEST=test/test.o

SRC=src/main.c src/utils.c src/parse_elf.c src/pretty_print.c
SRC_TEST=test/test.c

TRASH=src/*.o bin

all: $(TARGET)

$(TARGET): clean test
	mkdir bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug: clean
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

test: $(SRC_TEST)
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) $(TEST_FLAGS) $< -o $(TARGET_TEST)

clean:
	rm -rf $(TRASH) $(TARGET) $(TARGET_TEST)

.PHONY: test

