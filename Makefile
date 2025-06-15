PROGRAM_VERSION = 1.2.0

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
CC = clang
CFLAGS = -Wall -Wextra -pedantic -O2 -Iinclude -arch x86_64 -arch arm64
LDFLAGS = -arch x86_64 -arch arm64

DEBUG_CFLAGS = -Wall -Wextra -pedantic -O0 -g -Iinclude -arch x86_64 -arch arm64
DEBUG_LDFLAGS = -arch x86_64 -arch arm64

# === Cross-compilation setup (Windows) ===
LLVM_MINGW_ROOT = $(HOME)/toolchains/llvm-mingw
WIN_CC = $(LLVM_MINGW_ROOT)/bin/clang

# Targets per architecture
WIN_TARGET_X86_64 = x86_64-w64-windows-gnu
WIN_TARGET_I686   = i686-w64-windows-gnu
WIN_TARGET_ARM64  = aarch64-w64-windows-gnu

# CFLAGS per arch
WIN_CFLAGS_X86_64 = \
	--target=x86_64-w64-windows-gnu \
	-Iinclude

WIN_CFLAGS_I686 = \
	--target=i686-w64-windows-gnu \
	-Iinclude

WIN_CFLAGS_ARM64 = \
	--target=aarch64-w64-windows-gnu \
	-Iinclude

# LDFLAGS per arch
WIN_LDFLAGS_COMMON = -Wl,--entry=mainCRTStartup -Wl,--subsystem,console
WIN_LDFLAGS_X86_64 = $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_I686   = $(WIN_LDFLAGS_COMMON)
WIN_LDFLAGS_ARM64  = $(WIN_LDFLAGS_COMMON)

# === Source files ===
ALL_SRCS := $(wildcard $(SRC_DIR)/*.c)

# Filter platform-specific source files
MACOS_SRCS := $(filter-out $(SRC_DIR)/platform_win.c, $(ALL_SRCS))
WIN_SRCS := $(filter-out $(SRC_DIR)/platform_macos.c, $(ALL_SRCS))

# === Object files for macOS ===
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(MACOS_SRCS))

# === Default target ===

macos: $(MACOS_BIN_DIR)/typebelow

all: macos windows

.PHONY: $(VERSION_H) # Don't cache file timestamp

$(VERSION_H):
	@echo '#ifndef VERSION_H' > $(VERSION_H)
	@echo '#define VERSION_H' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#define PROGRAM_VERSION "$(PROGRAM_VERSION)"' >> $(VERSION_H)
	@echo '' >> $(VERSION_H)
	@echo '#endif' >> $(VERSION_H)

# windows target invokes builds with shared output folder:
windows: $(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_X86_64)" WIN_LDFLAGS="$(WIN_LDFLAGS_X86_64)" WIN_ARCH=x86_64 WIN_BIN_ARCH=$(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_I686)" WIN_LDFLAGS="$(WIN_LDFLAGS_I686)" WIN_ARCH=x86_32 WIN_BIN_ARCH=$(WIN_BIN_DIR)
	$(MAKE) windows-build WIN_CC=$(WIN_CC) WIN_CFLAGS="$(WIN_CFLAGS_ARM64)" WIN_LDFLAGS="$(WIN_LDFLAGS_ARM64)" WIN_ARCH=arm64 WIN_BIN_ARCH=$(WIN_BIN_DIR)

windows-build:
	@outfile="$(WIN_BIN_ARCH)/$(WIN_ARCH).exe"; \
	$(WIN_CC) $(WIN_CFLAGS) -fuse-ld=lld $(WIN_LDFLAGS) $(LIBS) -o $$outfile $(WIN_SRCS)

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: clean all

# === Testing ===

TEST_SRC = test_all.c $(wildcard tests/*.c)
TEST_BIN = bin/test_all

# Exclude src/main.c from tests
TEST_SRCS_WITH_LIB := $(filter-out src/main.c, $(SRCS))

test: $(TEST_BIN)
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(TEST_SRCS_WITH_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# === Build rules ===

# Native binaries
$(MACOS_BIN_DIR)/typebelow: $(OBJS) | $(MACOS_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Distribution copies
dist: clean all $(VERSION_H) | $(DST_DIR)
	@echo "Copying and renaming binaries to $(DST_DIR)/"
	@for file in $(MACOS_BIN_DIR)/*; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file); \
			cp -a "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-macos-universal"; \
		fi \
	done
	@for file in $(WIN_BIN_DIR)/*.exe; do \
		if [ -f "$$file" ]; then \
			base=$$(basename $$file .exe); \
			cp "$$file" "$(DST_DIR)/typebelow-$(PROGRAM_VERSION)-windows-$${base}.exe"; \
		fi \
	done

# GitHub release automation
NOTE ?= "Automated release of version $(PROGRAM_VERSION)"
release: dist
	gh release create v$(PROGRAM_VERSION) \
		--title "Release v$(PROGRAM_VERSION)" \
		--notes "$(NOTE)" \
		$(wildcard $(DST_DIR)/*)

# Directory creation
$(BIN_DIR) $(OBJ_DIR) $(WIN_BIN_DIR) $(MACOS_BIN_DIR):
	mkdir -p $@

$(DST_DIR):
	mkdir -p $@

# Clean all builds
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DST_DIR)

.PHONY: all clean debug macos windows windows-build dist release
