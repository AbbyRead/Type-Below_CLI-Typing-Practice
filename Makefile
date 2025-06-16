PROGRAM_VERSION ?= E0.0.0

# === Project Structure ===
SRC_DIR       := src
OBJ_DIR       := obj
BIN_DIR       := bin
DST_DIR       := dst
INCLUDE_DIR   := include
MACOS_BIN_DIR := $(BIN_DIR)/macos
WIN_BIN_DIR   := $(BIN_DIR)/windows

VERSION_H     := $(INCLUDE_DIR)/version.h

.PHONY: all debug-macos windows-x86_64 version_header clean

# === Compiler Setup (macOS only) ===
CC        ?= clang
CPPFLAGS  := -I$(INCLUDE_DIR)
CFLAGS    := -Wall -Wextra -pedantic -O2 -arch x86_64 -arch arm64
LDFLAGS   :=

DEBUG_CFLAGS := -Wall -Wextra -pedantic -O0 -g -arch x86_64 -arch arm64
DEBUG_LDFLAGS := -arch x86_64 -arch arm64

# === Cross-compilation Setup (Windows x86_64 only) ===
LLVM_MINGW_ROOT ?= $(HOME)/toolchains/llvm-mingw
WIN_CC         ?= $(LLVM_MINGW_ROOT)/bin/clang

ifeq ("$(wildcard $(LLVM_MINGW_ROOT)/bin/clang)","")
  $(warning LLVM_MINGW not found at $(LLVM_MINGW_ROOT); Windows builds may fail)
endif

# === Cross-compilation Setup (Windows additional targets) ===

# x86_64 (64-bit)
WIN_TARGET_X86_64 := x86_64-w64-windows-gnu
WIN_SYSROOT_X86_64 := $(LLVM_MINGW_ROOT)/x86_64-w64-mingw32
WIN_CFLAGS_X86_64 := --target=$(WIN_TARGET_X86_64) --sysroot=$(WIN_SYSROOT_X86_64)
WIN_LDFLAGS_X86_64 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console

# i686 (32-bit)
WIN_TARGET_I686 := i686-w64-windows-gnu
WIN_SYSROOT_I686 := $(LLVM_MINGW_ROOT)/i686-w64-mingw32
WIN_CFLAGS_I686 := --target=$(WIN_TARGET_I686) --sysroot=$(WIN_SYSROOT_I686)
WIN_LDFLAGS_I686 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console

# ARM64
WIN_TARGET_ARM64 := aarch64-w64-windows-gnu
WIN_SYSROOT_ARM64 := $(LLVM_MINGW_ROOT)/aarch64-w64-mingw32
WIN_CFLAGS_ARM64 := --target=$(WIN_TARGET_ARM64) --sysroot=$(WIN_SYSROOT_ARM64)
WIN_LDFLAGS_ARM64 := -fuse-ld=lld -Wl,--entry=mainCRTStartup -Wl,--subsystem,console

# === Source Files ===
ALL_SRC := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(filter-out $(SRC_DIR)/main.c, $(ALL_SRC))

MACOS_SRCS := $(SRCS) $(SRC_DIR)/platform/os_mac.c $(SRC_DIR)/main.c
WIN_SRCS   := $(SRCS) $(SRC_DIR)/platform/os_win.c $(SRC_DIR)/main.c

# === Object Files ===

# macOS objects
MACOS_OBJ_DIR := $(OBJ_DIR)/macos
MACOS_OBJS := $(patsubst $(SRC_DIR)/%.c,$(MACOS_OBJ_DIR)/%.o,$(MACOS_SRCS))

# Windows objects
WIN_OBJ_DIR_x86_64 := $(OBJ_DIR)/win-x86_64
WIN_OBJ_DIR_i686   := $(OBJ_DIR)/win-i686
WIN_OBJ_DIR_arm64  := $(OBJ_DIR)/win-arm64

WIN_OBJS_x86_64 := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_x86_64)/%.o,$(WIN_SRCS))
WIN_OBJS_i686  := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_i686)/%.o,$(WIN_SRCS))
WIN_OBJS_arm64 := $(patsubst $(SRC_DIR)/%.c,$(WIN_OBJ_DIR_arm64)/%.o,$(WIN_SRCS))

# === Build Targets ===
all: macos windows

# macOS build
macos: $(MACOS_OBJ_DIR) version_header $(MACOS_BIN_DIR) $(MACOS_BIN_DIR)/typebelow

debug-macos: CFLAGS := $(DEBUG_CFLAGS)
debug-macos: LDFLAGS := $(DEBUG_LDFLAGS)
debug-macos: clean macos

$(MACOS_BIN_DIR)/typebelow: $(MACOS_OBJS) | $(MACOS_BIN_DIR)
	@echo "Linking macOS binary $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Windows architectures
windows: windows-x86_64 windows-arm64 windows-i686

# Windows x86_64 build
windows-x86_64: version_header $(WIN_OBJ_DIR_x86_64) $(WIN_BIN_DIR)
	@echo "Building Windows x86_64 binary"
	$(MAKE) $(WIN_OBJS_x86_64)
	$(WIN_CC) $(WIN_CFLAGS_X86_64) $(WIN_OBJS_x86_64) $(WIN_LDFLAGS_X86_64) -o $(WIN_BIN_DIR)/x86_64.exe

# Windows x86_32 build
windows-i686: version_header $(WIN_BIN_DIR)
	@mkdir -p $(WIN_OBJ_DIR_i686)
	@mkdir -p $(WIN_BIN_DIR)
	$(MAKE) $(WIN_OBJS_i686)
	@echo "Linking Windows x86 (i686) binary"
	$(WIN_CC) -fuse-ld=lld $(WIN_CFLAGS_I686) $(WIN_OBJS_i686) $(WIN_LDFLAGS_I686) -o $(WIN_BIN_DIR)/i686.exe

# Windows Arm64 build
windows-arm64: version_header $(WIN_BIN_DIR)
	@mkdir -p $(WIN_OBJ_DIR_arm64)
	@mkdir -p $(WIN_BIN_DIR)
	$(MAKE) $(WIN_OBJS_arm64)
	@echo "Linking Windows ARM64 binary"
	$(WIN_CC) -fuse-ld=lld $(WIN_CFLAGS_ARM64) $(WIN_OBJS_arm64) $(WIN_LDFLAGS_ARM64) -o $(WIN_BIN_DIR)/arm64.exe

# Object rules
$(MACOS_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(WIN_OBJ_DIR_x86_64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_X86_64) -c $< -o $@

$(WIN_OBJ_DIR_i686)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_I686) -c $< -o $@

$(WIN_OBJ_DIR_arm64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(WIN_CC) $(CPPFLAGS) $(WIN_CFLAGS_ARM64) -c $< -o $@

# Directory creation
$(MACOS_BIN_DIR):
	@mkdir -p $@

$(WIN_BIN_DIR):
	@mkdir -p $@

$(MACOS_OBJ_DIR):
	@mkdir -p $@

$(WIN_OBJ_DIR_x86_64):
	@mkdir -p $@

# Version header
version_header: | $(INCLUDE_DIR)
	@echo "Generating version header $(VERSION_H)"
	@echo '#ifndef VERSION_H' > $(VERSION_H)
	@echo '#define VERSION_H' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#define PROGRAM_VERSION "$(PROGRAM_VERSION)"' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#endif' >> $(VERSION_H)

# Clean
clean:
	@echo "Cleaning build artifacts"
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DST_DIR)
