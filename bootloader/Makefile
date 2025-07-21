# RaeenOS Bootloader Makefile

# Build configuration
BUILD_DIR ?= ../build
TARGET_ARCH ?= x64

# Tools
NASM = nasm
DD = dd
OBJCOPY = objcopy

# Architecture-specific flags
ifeq ($(TARGET_ARCH),x64)
    NASM_FLAGS = -f bin -D X64_MODE
    MULTIBOOT_FLAGS = -D MULTIBOOT2
else
    NASM_FLAGS = -f bin -D X86_MODE
    MULTIBOOT_FLAGS = -D MULTIBOOT1
endif

# Files
BOOT_ASM = boot.asm
STAGE1_ASM = stage1/stage1.asm
STAGE2_ASM = stage2/stage2.asm
MULTIBOOT_C = multiboot.c

BOOT_BIN = $(BUILD_DIR)/boot.bin
STAGE1_BIN = $(BUILD_DIR)/stage1.bin
STAGE2_BIN = $(BUILD_DIR)/stage2.bin
MULTIBOOT_OBJ = $(BUILD_DIR)/multiboot.o
BOOTLOADER_BIN = $(BUILD_DIR)/bootloader.bin

# Targets
all: $(BOOTLOADER_BIN)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build main boot sector
$(BOOT_BIN): $(BOOT_ASM) | $(BUILD_DIR)
	@echo "Building boot sector..."
	$(NASM) $(NASM_FLAGS) $(MULTIBOOT_FLAGS) -o $@ $<

# Build stage 1 bootloader
$(STAGE1_BIN): $(STAGE1_ASM) | $(BUILD_DIR)
	@echo "Building stage 1 bootloader..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Build stage 2 bootloader
$(STAGE2_BIN): $(STAGE2_ASM) | $(BUILD_DIR)
	@echo "Building stage 2 bootloader..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Build multiboot support
$(MULTIBOOT_OBJ): $(MULTIBOOT_C) | $(BUILD_DIR)
	@echo "Building multiboot support..."
	gcc -m64 -c -nostdlib -nostdinc -fno-builtin -o $@ $<

# Combine bootloader components
$(BOOTLOADER_BIN): $(BOOT_BIN) $(STAGE1_BIN) $(STAGE2_BIN) $(MULTIBOOT_OBJ) | $(BUILD_DIR)
	@echo "Combining bootloader components..."
	@# Create a 1MB bootloader image
	@dd if=/dev/zero of=$@ bs=1M count=1 2>/dev/null
	@# Copy boot sector to first 512 bytes
	@dd if=$(BOOT_BIN) of=$@ conv=notrunc 2>/dev/null
	@# Copy stage 1 to sector 2
	@dd if=$(STAGE1_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	@# Copy stage 2 to sector 3
	@dd if=$(STAGE2_BIN) of=$@ bs=512 seek=2 conv=notrunc 2>/dev/null
	@echo "✓ Bootloader built successfully"

# Clean build artifacts
clean:
	@echo "Cleaning bootloader artifacts..."
	@rm -f $(BUILD_DIR)/boot.bin $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin
	@rm -f $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/bootloader.bin
	@echo "✓ Bootloader clean completed"

# Test bootloader standalone
test: $(BOOTLOADER_BIN)
	@echo "Testing bootloader in QEMU..."
	qemu-system-x86_64 -drive format=raw,file=$< -m 256 -display gtk

# Install bootloader for main build
install: $(BOOTLOADER_BIN)
	@echo "Installing bootloader..."
	@cp $(BOOTLOADER_BIN) $(BUILD_DIR)/
	@echo "✓ Bootloader installed"

# Debug bootloader
debug: $(BOOTLOADER_BIN)
	@echo "Starting bootloader debug session..."
	qemu-system-x86_64 -drive format=raw,file=$< -m 256 -s -S -display gtk

# Show bootloader information
info:
	@echo "=== RaeenOS Bootloader Build Information ==="
	@echo "Target Architecture: $(TARGET_ARCH)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "NASM Flags: $(NASM_FLAGS)"
	@echo "Components:"
	@echo "  - Boot sector (512 bytes)"
	@echo "  - Stage 1 bootloader"
	@echo "  - Stage 2 bootloader"
	@echo "  - Multiboot support"
	@echo ""
	@echo "Available targets:"
	@echo "  all     - Build complete bootloader"
	@echo "  clean   - Clean build artifacts"
	@echo "  test    - Test in QEMU"
	@echo "  debug   - Debug with QEMU + GDB"
	@echo "  install - Install for main build"
	@echo "  info    - Show this information"

.PHONY: all clean test debug install info