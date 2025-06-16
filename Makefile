PROGRAM_VERSION ?= E0.0.0
.DEFAULT_GOAL := macos-arm64

# === Project Structure ===
SRC_DIR       := src
OBJ_DIR       := obj
BIN_DIR       := bin
INCLUDE_DIR   := include
MACOS_BIN_DIR := $(BIN_DIR)/macos
WIN_BIN_DIR   := $(BIN_DIR)/windows
VERSION_H     := $(INCLUDE_DIR)/version.h

# Version header generation for release numbering
version_header: | $(INCLUDE_DIR)
	@echo "Generating version header $(VERSION_H)"
	@echo '#ifndef VERSION_H' > $(VERSION_H)
	@echo '#define VERSION_H' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#define PROGRAM_VERSION "$(PROGRAM_VERSION)"' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#endif' >> $(VERSION_H)

.PHONY: version_header all macos macos-arm64 macos-x86_64 macos-dist \
        debug-macos debug-macos-arm64 debug-macos-x86_64 debug-macos-dist \
        windows-all windows-x86_64 windows-arm64 windows-i686 clean clean-macos clean-windows

clean: clean-macos clean-windows

# === Compiler Setup (macOS only) ===
CC        ?= clang
CPPFLAGS  := -I$(INCLUDE_DIR)
CFLAGS    := -Wall -Wextra -pedantic -O2 -arch x86_64 -arch arm64
LDFLAGS   :=

DEBUG_CFLAGS := -Wall -Wextra -pedantic -O0 -g -arch x86_64 -arch arm64
DEBUG_LDFLAGS := -arch x86_64 -arch arm64

# === Source Files ===
ALL_SRC := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(filter-out $(SRC_DIR)/main.c, $(ALL_SRC))

MACOS_SRCS := $(SRCS) $(SRC_DIR)/platform/os_mac.c $(SRC_DIR)/main.c
WIN_SRCS   := $(SRCS) $(SRC_DIR)/platform/os_win.c $(SRC_DIR)/main.c

# === macOS ===

all: macos-all windows-all

macos-all: macos-arm64 macos-x86_64 macos-universal

macos-universal: version_header $(MACOS_BIN_DIR)/typebelow

debug-macos: CFLAGS := $(DEBUG_CFLAGS)
debug-macos: LDFLAGS := $(DEBUG_LDFLAGS)

# macOS per-arch build flags
MACOS_CFLAGS_arm64   := -Wall -Wextra -pedantic -O2 -arch arm64
MACOS_LDFLAGS_arm64  := -arch arm64

MACOS_CFLAGS_x86_64  := -Wall -Wextra -pedantic -O2 -arch x86_64
MACOS_LDFLAGS_x86_64 := -arch x86_64

DEBUG_CFLAGS_arm64   := -Wall -Wextra -pedantic -O0 -g -arch arm64
DEBUG_LDFLAGS_arm64  := -arch arm64

DEBUG_CFLAGS_x86_64  := -Wall -Wextra -pedantic -O0 -g -arch x86_64
DEBUG_LDFLAGS_x86_64 := -arch x86_64

MACOS_OBJ_DIR_arm64  := $(OBJ_DIR)/macos-arm64
MACOS_OBJ_DIR_x86_64 := $(OBJ_DIR)/macos-x86_64

MACOS_OBJS_arm64  := $(patsubst $(SRC_DIR)/%.c,$(MACOS_OBJ_DIR_arm64)/%.o,$(MACOS_SRCS))
MACOS_OBJS_x86_64 := $(patsubst $(SRC_DIR)/%.c,$(MACOS_OBJ_DIR_x86_64)/%.o,$(MACOS_SRCS))

macos-arm64: version_header $(MACOS_BIN_DIR) $(MACOS_OBJ_DIR_arm64)
	@mkdir -p $(dir $@)
	@echo "Linking macOS arm64 binary"
	$(CC) $(MACOS_CFLAGS_arm64) $(MACOS_LDFLAGS_arm64) -o $(MACOS_BIN_DIR)/typebelow-arm64 $(MACOS_OBJS_arm64)

macos-x86_64: version_header $(MACOS_BIN_DIR) $(MACOS_OBJ_DIR_x86_64)
	@mkdir -p $(dir $@)
	@echo "Linking macOS x86_64 binary"
	$(CC) $(MACOS_CFLAGS_x86_64) $(MACOS_LDFLAGS_x86_64) -o $(MACOS_BIN_DIR)/typebelow-x86_64 $(MACOS_OBJS_x86_64)

debug-macos-arm64: MACOS_CFLAGS_arm64 := $(DEBUG_CFLAGS_arm64)
debug-macos-arm64: MACOS_LDFLAGS_arm64 := $(DEBUG_LDFLAGS_arm64)
debug-macos-arm64: clean-macos macos-arm64

debug-macos-x86_64: MACOS_CFLAGS_x86_64 := $(DEBUG_CFLAGS_x86_64)
debug-macos-x86_64: MACOS_LDFLAGS_x86_64 := $(DEBUG_LDFLAGS_x86_64)
debug-macos-x86_64: clean-macos macos-x86_64

debug-macos-dist: debug-macos-arm64 debug-macos-x86_64
	@echo "Creating debug macOS universal binary using lipo"
	@lipo -create \
		$(MACOS_BIN_DIR)/typebelow-arm64 \
		$(MACOS_BIN_DIR)/typebelow-x86_64 \
		-output $(MACOS_BIN_DIR)/typebelow

$(MACOS_OBJ_DIR_arm64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(MACOS_CFLAGS_arm64) -c $< -o $@

$(MACOS_OBJ_DIR_x86_64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(MACOS_CFLAGS_x86_64) -c $< -o $@

$(MACOS_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(MACOS_BIN_DIR)/typebelow: $(MACOS_OBJS) | $(MACOS_BIN_DIR)
	@mkdir -p $(dir $@)
	@echo "Linking macOS binary $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

debug-macos: clean macos

clean-macos:
	@echo "Cleaning macOS build artifacts"
	rm -rf $(MACOS_OBJ_DIR_arm64) $(MACOS_OBJ_DIR_arm64) $(MACOS_BIN_DIR)

# === Cross-compilation to Windows ===

# Windows architectures
windows-all: windows-x86_64 windows-arm64 windows-i686

LLVM_MINGW_ROOT ?= $(HOME)/toolchains/llvm-mingw
WIN_CC         ?= $(LLVM_MINGW_ROOT)/bin/clang

ifeq ("$(wildcard $(LLVM_MINGW_ROOT)/bin/clang)","")
  $(warning LLVM_MINGW not found at $(LLVM_MINGW_ROOT); Windows builds may fail)
endif

# x86_64 (64-bit)
WIN_TARGET_x86_64 := x86_64-w64-windows-gnu
WIN_SYSROOT_x86_64 := $(LLVM_MINGW_ROOT)/x86_64-w64-mingw32
WIN_CFLAGS_x86_64 := --target=$(WIN_TARGET_x86_64) --sysroot=$(WIN_SYSROOT_x86_64)
WIN_LDFLAGS_x86_64 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_OBJ_DIR_x86_64 := $(OBJ_DIR)/win-x86_64
WIN_OBJS_x86_64 := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_x86_64)/%.o,$(WIN_SRCS))

# i686 (32-bit)
WIN_TARGET_i686 := i686-w64-windows-gnu
WIN_SYSROOT_i686 := $(LLVM_MINGW_ROOT)/i686-w64-mingw32
WIN_CFLAGS_i686 := --target=$(WIN_TARGET_i686) --sysroot=$(WIN_SYSROOT_i686)
WIN_LDFLAGS_i686 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_OBJ_DIR_i686   := $(OBJ_DIR)/win-i686
WIN_OBJS_i686  := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_i686)/%.o,$(WIN_SRCS))

# ARM64
WIN_TARGET_ARM64 := aarch64-w64-windows-gnu
WIN_SYSROOT_ARM64 := $(LLVM_MINGW_ROOT)/aarch64-w64-mingw32
WIN_CFLAGS_ARM64 := --target=$(WIN_TARGET_ARM64) --sysroot=$(WIN_SYSROOT_ARM64)
WIN_LDFLAGS_ARM64 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_OBJ_DIR_arm64  := $(OBJ_DIR)/win-arm64
WIN_OBJS_arm64 := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_arm64)/%.o,$(WIN_SRCS))

# Windows x86_64 build
windows-x86_64: version_header $(WIN_OBJ_DIR_x86_64) $(WIN_BIN_DIR)
	@echo "Building Windows x86_64 binary"
	$(MAKE) $(WIN_OBJS_x86_64)
	$(WIN_CC) $(WIN_CFLAGS_x86_64) $(WIN_OBJS_x86_64) $(WIN_LDFLAGS_x86_64) -o $(WIN_BIN_DIR)/x86_64.exe

# Windows x86_32 build
windows-i686: version_header $(WIN_BIN_DIR)
	@mkdir -p $(WIN_OBJ_DIR_i686)
	@mkdir -p $(WIN_BIN_DIR)
	$(MAKE) $(WIN_OBJS_i686)
	@echo "Linking Windows x86 (i686) binary"
	$(WIN_CC) -fuse-ld=lld $(WIN_CFLAGS_i686) $(WIN_OBJS_i686) $(WIN_LDFLAGS_i686) -o $(WIN_BIN_DIR)/i686.exe

# Windows Arm64 build
windows-arm64: version_header $(WIN_BIN_DIR)
	@mkdir -p $(WIN_OBJ_DIR_arm64)
	@mkdir -p $(WIN_BIN_DIR)
	$(MAKE) $(WIN_OBJS_arm64)
	@echo "Linking Windows ARM64 binary"
	$(WIN_CC) -fuse-ld=lld $(WIN_CFLAGS_ARM64) $(WIN_OBJS_arm64) $(WIN_LDFLAGS_ARM64) -o $(WIN_BIN_DIR)/arm64.exe

# Object rules
$(WIN_OBJ_DIR_x86_64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_x86_64) -c $< -o $@

$(WIN_OBJ_DIR_i686)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_i686) -c $< -o $@

$(WIN_OBJ_DIR_arm64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_ARM64) -c $< -o $@

clean-windows:
	@echo "Cleaning build artifacts"
	rm -rf $(WIN_OBJ_DIR_x86_64) $(WIN_OBJ_DIR_i686) $(WIN_OBJ_DIR_arm64) $(WIN_BIN_DIR)

