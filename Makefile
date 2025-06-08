# === Compiler setup ===
CC = clang
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS =

DEBUG_CFLAGS = -Wall -Wextra -O0 -g -Iinclude
DEBUG_LDFLAGS =

# === Cross-compilation setup (Windows) ===
WIN_CC = clang
WIN_TARGET = x86_64-w64-windows-gnu
MINGW_ROOT = /opt/homebrew/opt/mingw-w64/toolchain-x86_64

WIN_CFLAGS = \
	--target=$(WIN_TARGET) \
	-isystem $(MINGW_ROOT)/x86_64-w64-mingw32/include \
	-D__USE_MINGW_ANSI_STDIO=1 \
	-Iinclude

WIN_LDFLAGS = \
  -L/opt/homebrew/opt/mingw-w64/toolchain-x86_64/x86_64-w64-mingw32/lib \
  -L/opt/homebrew/opt/mingw-w64/toolchain-x86_64/lib/gcc/x86_64-w64-mingw32/15.1.0 \
  -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
	
# === Project structure ===
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
WIN_BIN_DIR = $(BIN_DIR)/windows

# === Source and binary derivations ===
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
EXES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRCS))
WIN_EXES := $(patsubst $(SRC_DIR)/%.c, $(WIN_BIN_DIR)/%.exe, $(SRCS))

# === Default target ===
all: $(BIN_DIR) $(OBJ_DIR) $(EXES)

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: clean all

windows: $(WIN_BIN_DIR) $(WIN_EXES)

# === Build rules ===

# Native binaries
$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Windows cross-compiled .exe
$(WIN_BIN_DIR)/%.exe: $(SRC_DIR)/%.c
	$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) -lgcc -lgcc_eh -o $@ $<

# Directory creation
$(BIN_DIR) $(OBJ_DIR) $(WIN_BIN_DIR):
	mkdir -p $@

# Clean all builds
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean debug windows
