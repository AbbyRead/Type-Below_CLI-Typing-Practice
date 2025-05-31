# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -O2 -Iinclude

# Project structure
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
EXES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRCS))

# Default rule
all: $(BIN_DIR) $(OBJ_DIR) $(EXES)

# Compile each source to its own executable
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $<

# Compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
