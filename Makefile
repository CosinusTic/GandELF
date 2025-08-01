CC = gcc
CFLAGS = -std=c99 -Werror -Wall -Wextra
DEBUG_FLAGS = -g
TEST_FLAGS = -O0 -fno-omit-frame-pointer

SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

TARGET = $(BIN_DIR)/gandelf
TARGET_TEST = $(TEST_DIR)/test
OBJ = $(SRC:.c=.o)

SRC = $(SRC_DIR)/main.c $(SRC_DIR)/utils.c $(SRC_DIR)/parse_elf.c $(SRC_DIR)/pretty_print.c
OBJS = $(SRC:.c=.o)
TEST_SRC = $(TEST_DIR)/test.c

.PHONY: all debug test clean

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

test: CFLAGS += $(DEBUG_FLAGS) $(TEST_FLAGS)
test: $(TEST_SRC)
	$(CC) $(CFLAGS) $^ -o $(TARGET_TEST)

clean:
	rm -rf $(BIN_DIR) $(TARGET_TEST) $(SRC_DIR)/*.o

