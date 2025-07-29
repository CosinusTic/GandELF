CC=gcc
CFLAGS=-std=c99 -Werror -Wall -Wextra
DEBUG_FLAGS=-g 
TEST_FLAGS=-O0 -fno-omit-frame-pointer

TARGET=bin/gandelf
TARGET_TEST=test/test.o
TARGET_TEST_BIN=test/test

SRC=src/main.c src/utils.c src/parse_elf.c src/pretty_print.c src/dump.c
SRC_TEST=test/test.c

TRASH=src/*.o bin

all: $(TARGET)

$(TARGET): clean test test_bin
	mkdir bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug: clean
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

test: $(SRC_TEST)
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) $(TEST_FLAGS) $< -o $(TARGET_TEST)

test_bin:
	$(CC) $(DEBUG_FLAGS) $(CFLAGS) $(TEST_FLAGS) $(SRC_TEST) -o $(TARGET_TEST_BIN)

clean:
	rm -rf $(TRASH) $(TARGET) $(TARGET_TEST) $(TARGET_TEST_BIN)

.PHONY: test

