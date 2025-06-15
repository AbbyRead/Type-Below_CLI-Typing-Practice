# Makefile for cross-platform build: macOS (universal), Linux, Windows (x86_64, i686, arm64)
# Includes debug builds, auto dependency generation, distribution, testing, and release automation.

PROGRAM_VERSION ?= E0.0.0

# === Project Structure ===
SRC_DIR       := src
OBJ_DIR       := obj
BIN_DIR       := bin
DST_DIR       := dst
INCLUDE_DIR   := include

MACOS_BIN_DIR := $(BIN_DIR)/macos
LINUX_BIN_DIR := $(BIN_DIR)/linux
WIN_BIN_DIR   := $(BIN_DIR)/windows

VERSION_H     := $(INCLUDE_DIR)/version.h

# === Compiler Setup ===
CC        ?= clang
CFLAGS    := -Wall -Wextra -pedantic -O2 -I$(INCLUDE_DIR) -arch x86_64 -arch arm64 -MMD -MF
LDFLAGS   := -arch x86_64 -arch arm64

DEBUG_CFLAGS := -Wall -Wextra -pedantic -O0 -g -I$(INCLUDE_DIR) -arch x86_64 -arch arm64 -MMD -MF
DEBUG_LDFLAGS := -arch x86_64 -arch arm64

# === Cross-compilation Setup (Windows) ===
LLVM_MINGW_ROOT ?= $(HOME)/toolchains/llvm-mingw
WIN_CC         ?= $(LLVM_MINGW_ROOT)/bin/clang

ifeq ("$(wildcard $(LLVM_MINGW_ROOT)/bin/clang)","")
$(warning LLVM_MINGW not found at $(LLVM_MINGW_ROOT); Windows builds may fail)
endif

WIN_TARGET_X86_64 := x86_64-w64-windows-gnu
WIN_TARGET_I686   := i686-w64-windows-gnu
WIN_TARGET_ARM64  := aarch64-w64-windows-gnu

WIN_CFLAGS_X86_64 := --target=$(WIN_TARGET_X86_64) -I$(INCLUDE_DIR)
WIN_CFLAGS_I686   := --target=$(WIN_TARGET_I686) -I$(INCLUDE_DIR)
WIN_CFLAGS_ARM64  := --target=$(WIN_TARGET_ARM64) -I$(INCLUDE_DIR)

WIN_LDFLAGS_COMMON := -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_LDFLAGS_X86_64 := $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_I686   := $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_ARM64  := $(WIN_LDFLAGS_COMMON)

# === Source Files ===
ALL_SRCS := $(wildcard $(SRC_DIR)/*.c)

# Filter platform-specific source files:
MACOS_SRCS := $(filter-out $(SRC_DIR)/platform_win.c, $(ALL_SRCS))
WIN_SRCS   := $(filter-out $(SRC_DIR)/platform_macos.c, $(ALL_SRCS))
LINUX_SRCS := $(filter-out $(SRC_DIR)/platform_win.c $(SRC_DIR)/platform_macos.c, $(ALL_SRCS))

# === Object Files ===
MACOS_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/macos_%.o,$(MACOS_SRCS))
WIN_OBJS   := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/win_%.o,$(WIN_SRCS))
LINUX_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/linux_%.o,$(LINUX_SRCS))

# Dependency files for automatic header tracking
MACOS_DEPS := $(MACOS_OBJS:.o=.d)
WIN_DEPS   := $(WIN_OBJS:.o=.d)
LINUX_DEPS := $(LINUX_OBJS:.o=.d)

# === Detect Native OS ===
ifeq ($(origin OS_OVERRIDE), undefined)
  UNAME_S := $(shell uname -s)
else
  UNAME_S := $(OS_OVERRIDE)
endif

ifeq ($(UNAME_S),Darwin)
DEFAULT_TARGET := macos
else ifeq ($(UNAME_S),Linux)
DEFAULT_TARGET := linux
else ifeq ($(OS),Windows_NT)
DEFAULT_TARGET := windows
else
$(error Unsupported OS: $(UNAME_S))
endif

.DEFAULT_GOAL := $(DEFAULT_TARGET)

# === Compiler Flags for Debugging Windows ===
ifeq ($(DEBUG),1)
WIN_CFLAGS_X86_64 := $(WIN_CFLAGS_X86_64) -O0 -g
WIN_CFLAGS_I686   := $(WIN_CFLAGS_I686)   -O0 -g
WIN_CFLAGS_ARM64  := $(WIN_CFLAGS_ARM64)  -O0 -g
endif

# === Directory creation (grouped to reduce shell invocations) ===
.PHONY: dirs
dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(MACOS_BIN_DIR) $(WIN_BIN_DIR) $(LINUX_BIN_DIR) $(DST_DIR)

# === Version Header ===
.PHONY: version_header
version_header: | $(INCLUDE_DIR)
	@echo "Generating version header $(VERSION_H)"
	@echo '#ifndef VERSION_H' > $(VERSION_H)
	@echo '#define VERSION_H' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#define PROGRAM_VERSION "$(PROGRAM_VERSION)"' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#endif' >> $(VERSION_H)

# === Build Targets ===
.PHONY: all macos linux windows windows-x86_64 windows-i686 windows-arm64 debug-macos debug-linux debug-windows dist release clean test help

all: macos linux windows

# --- macOS ---
macos: dirs version_header $(MACOS_BIN_DIR)/typebelow

debug-macos: CFLAGS := $(DEBUG_CFLAGS)
debug-macos: LDFLAGS := $(DEBUG_LDFLAGS)
debug-macos: clean macos

$(MACOS_BIN_DIR)/typebelow: $(MACOS_OBJS) | $(MACOS_BIN_DIR)
	@echo "Linking macOS binary $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# --- Linux ---
linux: dirs version_header $(LINUX_BIN_DIR)/typebelow

debug-linux: CFLAGS := -Wall -Wextra -pedantic -O0 -g -I$(INCLUDE_DIR) -MMD -MF
debug-linux: clean linux

$(LINUX_BIN_DIR)/typebelow: $(LINUX_OBJS) | $(LINUX_BIN_DIR)
	@echo "Linking Linux binary $@"
	$(CC) $(CFLAGS) -o $@ $^

# --- Windows ---
windows: windows-x86_64 windows-i686 windows-arm64

windows-x86_64:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_X86_64)" WIN_LDFLAGS="$(WIN_LDFLAGS_X86_64)" WIN_ARCH=x86_64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-i686:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_I686)" WIN_LDFLAGS="$(WIN_LDFLAGS_I686)" WIN_ARCH=x86_32 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-arm64:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_ARM64)" WIN_LDFLAGS="$(WIN_LDFLAGS_ARM64)" WIN_ARCH=arm64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

debug-windows: clean
	$(MAKE) windows DEBUG=1

.PHONY: windows-build
windows-build: dirs
	@outfile="$(WIN_BIN_ARCH)/$(WIN_ARCH).exe"; \
	echo "Building $$outfile"; \
	$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) -o $$outfile $(WIN_SRCS)

# === Object file compilation rules with auto dependencies ===
# macOS objects
$(OBJ_DIR)/macos_%.o: $(SRC_DIR)/%.c | dirs
	@echo "Compiling $< for macOS"
	$(CC) $(CFLAGS) -c $< -o $@
-include $(OBJ_DIR)/macos_%.d

# Linux objects
$(OBJ_DIR)/linux_%.o: $(SRC_DIR)/%.c | dirs
	@echo "Compiling $< for Linux"
	$(CC) $(CFLAGS) -c $< -o $@
-include $(OBJ_DIR)/linux_%.d

# Windows objects
$(OBJ_DIR)/win_%.o: $(SRC_DIR)/%.c | dirs
	@echo "Compiling $< for Windows"
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@
-include $(OBJ_DIR)/win_%.d

# === Distribution ===
dist: clean all | dirs
	@echo "Copying binaries to $(DST_DIR)/"
	@for file in $(MACOS_BIN_DIR)/*; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file); \
			cp -a "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-macos-$$base"; \
		fi \
	done
	@for file in $(LINUX_BIN_DIR)/*; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file); \
			cp -a "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-linux-$$base"; \
		fi \
	done
	@for file in $(WIN_BIN_DIR)/*.exe; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file .exe); \
			cp "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-windows-$$base.exe"; \
		fi \
	done

# === GitHub release automation ===
NOTE ?= "Automated release of version $(PROGRAM_VERSION)"
release: check-tools dist
	@echo "Creating GitHub release v$(PROGRAM_VERSION)"
	@gh release create v$(PROGRAM_VERSION) \
		--title "Release v$(PROGRAM_VERSION)" \
		--notes "$(NOTE)" \
		$(wildcard $(DST_DIR)/*)

# === Check required tools ===
.PHONY: check-tools
check-tools:
	@command -v clang >/dev/null || (echo "clang not found. Please install clang." && exit 1)
	@command -v lld >/dev/null || (echo "lld linker not found. Please install lld." && exit 1)
	@command -v gh >/dev/null || (echo "GitHub CLI 'gh' not found. Please install it." && exit 1)

# === Testing ===
TEST_SRC := test_all.c $(wildcard tests/*.c)
TEST_BIN := $(BIN_DIR)/test_all

test: dirs $(TEST_BIN)
	@echo "Running tests"
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(filter-out src/main.c, $(ALL_SRCS)) | dirs
	@echo "Compiling test binary"
	$(CC) $(CFLAGS) -o $@ $^

# === Clean ===
.PHONY: clean
clean:
	@echo "Cleaning build artifacts"
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DST_DIR)

# === Help ===
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make                   Build native binary ($(DEFAULT_TARGET))"
	@echo "  make all               Build all binaries (macOS, Linux, Windows)"
	@echo "  make macos             Build macOS universal binary"
	@echo "  make linux             Build Linux binary"
	@echo "  make windows           Build all Windows binaries"
	@echo "  make windows-x86_64    Build Windows x86_64 binary"
	@echo "  make windows-i686      Build Windows i686 binary"
	@echo "  make windows-arm64     Build Windows ARM64 binary"
	@echo "  make debug-macos       Build macOS debug binary"
	@echo "  make debug-linux       Build Linux debug binary"
	@echo "  make debug-windows     Build all Windows debug binaries"
	@echo "  make dist              Prepare distribution package in $(DST_DIR)/"
	@echo "  make release           Create GitHub release (requires 'gh' CLI)"
	@echo "  make test              Build and run tests"
	@echo "  make clean             Remove all build artifacts"
	@echo "  make help              Show this help message"

