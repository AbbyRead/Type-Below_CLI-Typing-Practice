PROGRAM_VERSION ?= 1.2.0

# === Project structure ===
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DST_DIR = dst
INCLUDE_DIR = include
MACOS_BIN_DIR = $(BIN_DIR)/macos
WIN_BIN_DIR = $(BIN_DIR)/windows
VERSION_H = $(INCLUDE_DIR)/version.h

# === Compiler setup ===
CC ?= clang
CFLAGS = -Wall -Wextra -pedantic -O2 -Iinclude -arch x86_64 -arch arm64
LDFLAGS = -arch x86_64 -arch arm64

DEBUG_CFLAGS = -Wall -Wextra -pedantic -O0 -g -Iinclude -arch x86_64 -arch arm64
DEBUG_LDFLAGS = -arch x86_64 -arch arm64

# === Cross-compilation setup (Windows) ===
LLVM_MINGW_ROOT ?= $(HOME)/toolchains/llvm-mingw
WIN_CC ?= $(LLVM_MINGW_ROOT)/bin/clang

# Warn if LLVM_MINGW_ROOT doesn't look right
ifeq ("$(wildcard $(LLVM_MINGW_ROOT)/bin/clang)","")
$(warning LLVM_MINGW not found at $(LLVM_MINGW_ROOT); Windows builds may fail)
endif

# Targets per architecture
WIN_TARGET_X86_64 = x86_64-w64-windows-gnu
WIN_TARGET_I686   = i686-w64-windows-gnu
WIN_TARGET_ARM64  = aarch64-w64-windows-gnu

# CFLAGS per arch
WIN_CFLAGS_X86_64 = --target=$(WIN_TARGET_X86_64) -Iinclude
WIN_CFLAGS_I686   = --target=$(WIN_TARGET_I686) -Iinclude
WIN_CFLAGS_ARM64  = --target=$(WIN_TARGET_ARM64) -Iinclude

# LDFLAGS per arch
WIN_LDFLAGS_COMMON = -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_LDFLAGS_X86_64 = $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_I686   = $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_ARM64  = $(WIN_LDFLAGS_COMMON)

# === Source files ===
ALL_SRCS := $(wildcard $(SRC_DIR)/*.c)

# Filter platform-specific source files by convention:
# User code should include platform files via #ifdef in a single entry point,
# so here we just exclude platform files from platform-specific builds.

MACOS_SRCS := $(filter-out $(SRC_DIR)/platform_win.c, $(ALL_SRCS))
WIN_SRCS := $(filter-out $(SRC_DIR)/platform_macos.c, $(ALL_SRCS))

# === Object files for macOS ===
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(MACOS_SRCS))

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

# === Default target ===
.DEFAULT_GOAL := $(DEFAULT_TARGET)

# === Main Targets ===

macos: $(MACOS_BIN_DIR)/typebelow
macos-universal: $(MACOS_BIN_DIR)/universal

all: macos windows

# === Version header ===
FORCE:

$(VERSION_H): FORCE
	@echo '#ifndef VERSION_H' > $(VERSION_H)
	@echo '#define VERSION_H' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#define PROGRAM_VERSION "$(PROGRAM_VERSION)"' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#endif' >> $(VERSION_H)

# === Windows targets: split for parallel builds ===
windows: windows-x86_64 windows-i686 windows-arm64

windows-x86_64:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_X86_64)" WIN_LDFLAGS="$(WIN_LDFLAGS_X86_64)" WIN_ARCH=x86_64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-i686:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_I686)" WIN_LDFLAGS="$(WIN_LDFLAGS_I686)" WIN_ARCH=x86_32 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-arm64:
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_ARM64)" WIN_LDFLAGS="$(WIN_LDFLAGS_ARM64)" WIN_ARCH=arm64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-build:
	@outfile="$(WIN_BIN_ARCH)/$(WIN_ARCH).exe"; \
	echo "Building $$outfile"; \
	$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) -o $$outfile $(WIN_SRCS)

# === Debug targets ===
debug-macos: CFLAGS = $(DEBUG_CFLAGS)
debug-macos: LDFLAGS = $(DEBUG_LDFLAGS)
debug-macos: clean macos

debug-windows: clean
	$(MAKE) windows DEBUG=1

# Add DEBUG flag support for windows-build
ifeq ($(DEBUG),1)
WIN_CFLAGS := $(WIN_CFLAGS) -O0 -g
endif

# === Testing ===
TEST_SRC = test_all.c $(wildcard tests/*.c)
TEST_BIN = $(BIN_DIR)/test_all

test: $(TEST_BIN)
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(filter-out src/main.c, $(ALL_SRCS)) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# === Build rules ===

# macOS native binary
$(MACOS_BIN_DIR)/typebelow: $(OBJS) | $(MACOS_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# === Distribution ===
dist: clean all $(VERSION_H) | $(DST_DIR)
	@echo "Copying and renaming binaries to $(DST_DIR)/"
	@for file in $(MACOS_BIN_DIR)/*; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file); \
			cp -a "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-macos-$$base"; \
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
	gh release create v$(PROGRAM_VERSION) \
		--title "Release v$(PROGRAM_VERSION)" \
		--notes "$(NOTE)" \
		$(wildcard $(DST_DIR)/*)

# === Check for required tools ===
check-tools:
	@command -v clang >/dev/null || (echo "clang not found. Please install clang." && exit 1)
	@command -v gh >/dev/null || (echo "GitHub CLI 'gh' not found. Please install it." && exit 1)

# === Directory creation ===
$(BIN_DIR) $(OBJ_DIR) $(WIN_BIN_DIR) $(MACOS_BIN_DIR):
	mkdir -p $@

$(DST_DIR):
	mkdir -p $@

# === Clean ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DST_DIR)

# === Help ===
help:
	@echo "Available targets:"
	@echo "  make                   Build native binary ($(DEFAULT_TARGET))"
	@echo "  make all               Build macOS + Windows binaries"
	@echo "  make macos             Build macOS binary"
	@echo "  make windows           Build all Windows binaries"
	@echo "  make debug-macos       Build debug macOS binary"
	@echo "  make debug-windows     Build debug Windows binaries"
	@echo "  make dist              Copy binaries to dst/ for packaging"
	@echo "  make release           Create GitHub release with binaries"
	@echo "  make clean             Remove all build artifacts"
	@echo "  make test              Build and run tests"
	@echo "  make help              Show this help message"

.PHONY: all clean debug-macos debug-windows macos windows windows-x86_64 windows-i686 windows-arm64 windows-build dist release test check-tools help FORCE
