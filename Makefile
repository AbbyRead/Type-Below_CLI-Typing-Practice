# === Compiler setup ===
CC = clang
CFLAGS = -Wall -Wextra -O2 -Iinclude -arch x86_64 -arch arm64
LDFLAGS = -arch x86_64 -arch arm64

DEBUG_CFLAGS = -Wall -Wextra -O0 -g -Iinclude -arch x86_64 -arch arm64
DEBUG_LDFLAGS = -arch x86_64 -arch arm64

# === Cross-compilation setup (Windows) ===
# Default cross-compiler for Windows targets
WIN_CC = clang
# ARM64 Windows build will use llvm-mingw's toolchain directly
WIN_CC_ARM64 = $(HOME)/toolchains/llvm-mingw/bin/aarch64-w64-mingw32-clang

# Toolchain roots for each architecture
MINGW_ROOT_X86_64 = /opt/homebrew/opt/mingw-w64/toolchain-x86_64
MINGW_ROOT_I686   = /opt/homebrew/opt/mingw-w64/toolchain-i686
MINGW_ROOT_ARM64  = $(HOME)/toolchains/llvm-mingw

# Targets per architecture
WIN_TARGET_X86_64 = x86_64-w64-windows-gnu
WIN_TARGET_I686   = i686-w64-windows-gnu
WIN_TARGET_ARM64  = aarch64-w64-windows-gnu

# CFLAGS per arch
WIN_CFLAGS_X86_64 = \
	--target=$(WIN_TARGET_X86_64) \
	-isystem $(MINGW_ROOT_X86_64)/x86_64-w64-mingw32/include \
	-D__USE_MINGW_ANSI_STDIO=1 \
	-Iinclude

WIN_CFLAGS_I686 = \
	--target=$(WIN_TARGET_I686) \
	-isystem $(MINGW_ROOT_I686)/i686-w64-mingw32/include \
	-D__USE_MINGW_ANSI_STDIO=1 \
	-Iinclude

WIN_CFLAGS_ARM64 = \
	--target=$(WIN_TARGET_ARM64) \
	-Iinclude

# LDFLAGS per arch
WIN_LDFLAGS_X86_64 = \
	-L$(MINGW_ROOT_X86_64)/x86_64-w64-mingw32/lib \
	-L$(MINGW_ROOT_X86_64)/lib/gcc/x86_64-w64-mingw32/15.1.0 \
	-Wl,--entry=mainCRTStartup -Wl,--subsystem,console

WIN_LDFLAGS_I686 = \
	-L$(MINGW_ROOT_I686)/i686-w64-mingw32/lib \
	-L$(MINGW_ROOT_I686)/lib/gcc/i686-w64-mingw32/15.1.0 \
	-Wl,--entry=mainCRTStartup -Wl,--subsystem,console

WIN_LDFLAGS_ARM64 = \
	-Wl,--entry=mainCRTStartup -Wl,--subsystem,console

# === Project structure ===
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
MACOS_BIN_DIR = $(BIN_DIR)/macos
WIN_BIN_DIR = $(BIN_DIR)/windows

# === Source and binary derivations ===
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Directories for each Windows arch output
# WIN_BIN_X86_64 = $(WIN_BIN_DIR)/x86_64
# WIN_BIN_ARM64  = $(WIN_BIN_DIR)/arm64
# WIN_BIN_I686   = $(WIN_BIN_DIR)/win32

# === Default target ===
all: macos windows

macos: $(MACOS_BIN_DIR) $(OBJ_DIR) $(patsubst $(SRC_DIR)/%.c, $(MACOS_BIN_DIR)/%, $(SRCS))

# windows target invokes builds with shared output folder:
windows: $(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_X86_64)" WIN_LDFLAGS="$(WIN_LDFLAGS_X86_64)" WIN_TARGET="$(WIN_TARGET_X86_64)" WIN_ARCH=x86_64 WIN_BIN_ARCH=$(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_I686)" WIN_LDFLAGS="$(WIN_LDFLAGS_I686)" WIN_TARGET="$(WIN_TARGET_I686)" WIN_ARCH=win32 WIN_BIN_ARCH=$(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC_ARM64) WIN_CFLAGS="$(WIN_CFLAGS_ARM64)" WIN_LDFLAGS="$(WIN_LDFLAGS_ARM64)" WIN_TARGET="$(WIN_TARGET_ARM64)" WIN_ARCH=arm64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-build: $(SRCS)
	@for src in $^; do \
		name=$$(basename $$src .c); \
		outfile="$(WIN_BIN_ARCH)/$${name}-$(WIN_ARCH).exe"; \
		echo "Building $$outfile for $(WIN_TARGET)"; \
		if [ "$(WIN_TARGET)" = "aarch64-w64-windows-gnu" ]; then \
			$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) -o $$outfile $$src; \
		else \
			$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) -lgcc -lgcc_eh -o $$outfile $$src; \
		fi \
	done

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: clean all

# === Build rules ===

# Native binaries
$(MACOS_BIN_DIR)/%: $(OBJ_DIR)/%.o | $(MACOS_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory creation
$(BIN_DIR) $(OBJ_DIR) $(WIN_BIN_DIR) $(MACOS_BIN_DIR) $(WIN_BIN_X86_64) $(WIN_BIN_I686) $(WIN_BIN_ARM64):
	mkdir -p $@

# Clean all builds
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean debug windows windows-build
