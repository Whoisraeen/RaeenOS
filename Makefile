# RaeenOS Build System
# Copyright (c) 2025 RaeenOS Project

.PHONY: all clean bootloader kernel ui tools install test debug run iso qemu-test validate

# Build configuration
TARGET_ARCH ?= x64
BUILD_TYPE ?= debug
CC = gcc
CXX = g++
AS = nasm
LD = ld
OBJCOPY = objcopy
GRUB_MKRESCUE = grub-mkrescue

# Directories
BUILD_DIR = build
BOOTLOADER_DIR = bootloader
KERNEL_DIR = kernel
UI_DIR = ui
TOOLS_DIR = tools
ISO_DIR = $(BUILD_DIR)/iso
INITRD_DIR = $(BUILD_DIR)/initrd

# Version information
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION_STRING = "$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"

# Build flags with AI optimization and security hardening
CFLAGS = -std=c11 -Wall -Wextra -Werror -nostdlib -fno-builtin -fno-stack-protector
CFLAGS += -ffreestanding -fno-pie -fno-pic -mno-red-zone -mcmodel=kernel
CFLAGS += -DVERSION_STRING=$(VERSION_STRING) -DTARGET_$(TARGET_ARCH)
CFLAGS += -fstack-clash-protection -fcf-protection=full -fno-omit-frame-pointer

CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -nostdlib -fno-builtin -fno-stack-protector
CXXFLAGS += -fno-exceptions -fno-rtti -ffreestanding -fno-pie -fno-pic -mno-red-zone
CXXFLAGS += -DVERSION_STRING=$(VERSION_STRING) -DTARGET_$(TARGET_ARCH)

ASFLAGS = -f elf64
LDFLAGS = -nostdlib -z noexecstack -z relro -z now

# AI acceleration flags
CFLAGS += -mavx2 -mfma -msse4.2 -DAI_ACCELERATION_ENABLED
CXXFLAGS += -mavx2 -mfma -msse4.2 -DAI_ACCELERATION_ENABLED

ifeq ($(BUILD_TYPE),debug)
    CFLAGS += -g -O0 -DDEBUG -DKERNEL_DEBUG
    CXXFLAGS += -g -O0 -DDEBUG -DKERNEL_DEBUG
else
    CFLAGS += -O2 -DNDEBUG -DKERNEL_RELEASE
    CXXFLAGS += -O2 -DNDEBUG -DKERNEL_RELEASE
endif

ifeq ($(TARGET_ARCH),x86)
    CFLAGS += -m32
    CXXFLAGS += -m32
    ASFLAGS = -f elf32
    LDFLAGS += -m elf_i386
else
    CFLAGS += -m64
    CXXFLAGS += -m64
    LDFLAGS += -m elf_x86_64
endif

# Default target - Build complete OS
all: validate-deps $(BUILD_DIR) bootloader kernel initrd iso
	@echo "=== RaeenOS Build Complete ==="
	@echo "Version: $(VERSION_STRING)"
	@echo "Architecture: $(TARGET_ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "ISO: $(BUILD_DIR)/raeenos.iso"
	@echo "Ready for testing with 'make run' or 'make qemu-test'"

# Validate build dependencies
validate-deps:
	@echo "Validating build dependencies..."
	@command -v $(CC) >/dev/null 2>&1 || (echo "Error: $(CC) not found" && exit 1)
	@command -v $(AS) >/dev/null 2>&1 || (echo "Error: $(AS) not found" && exit 1)
	@command -v $(LD) >/dev/null 2>&1 || (echo "Error: $(LD) not found" && exit 1)
	@command -v $(GRUB_MKRESCUE) >/dev/null 2>&1 || (echo "Error: $(GRUB_MKRESCUE) not found" && exit 1)
	@echo "✓ All dependencies validated"

# Create build directory structure
$(BUILD_DIR):
	@echo "Creating build directory structure..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ISO_DIR)/boot/grub
	@mkdir -p $(INITRD_DIR)
	@mkdir -p $(BUILD_DIR)/apps
	@mkdir -p $(BUILD_DIR)/lib
	@mkdir -p $(BUILD_DIR)/bin
	@mkdir -p $(BUILD_DIR)/sbin
	@mkdir -p $(BUILD_DIR)/etc
	@mkdir -p $(BUILD_DIR)/usr/{bin,sbin,lib,share}
	@mkdir -p $(BUILD_DIR)/var
	@mkdir -p $(BUILD_DIR)/tmp
	@mkdir -p $(BUILD_DIR)/{proc,sys,dev,home}

# Build bootloader with multiboot support
bootloader: $(BUILD_DIR)
	@echo "Building RaeenOS bootloader..."
	$(MAKE) -C $(BOOTLOADER_DIR) BUILD_DIR=../$(BUILD_DIR) TARGET_ARCH=$(TARGET_ARCH)
	@echo "✓ Bootloader built successfully"

# Build kernel with all subsystems
kernel: $(BUILD_DIR)
	@echo "Building RaeenOS kernel..."
	$(MAKE) -C $(KERNEL_DIR) BUILD_DIR=../$(BUILD_DIR) TARGET_ARCH=$(TARGET_ARCH) BUILD_TYPE=$(BUILD_TYPE) \
		CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" ASFLAGS="$(ASFLAGS)" LDFLAGS="$(LDFLAGS)"
	@echo "✓ Kernel built successfully"

# Build UI framework
ui: $(BUILD_DIR)
	@echo "Building RaeenOS UI framework..."
	$(MAKE) -C $(UI_DIR) BUILD_DIR=../$(BUILD_DIR) TARGET_ARCH=$(TARGET_ARCH) BUILD_TYPE=$(BUILD_TYPE)
	@echo "✓ UI framework built successfully"

# Build development tools
tools: $(BUILD_DIR)
	@echo "Building development tools..."
	$(MAKE) -C $(TOOLS_DIR) BUILD_DIR=../$(BUILD_DIR)
	@echo "✓ Development tools built successfully"

# Create initial RAM disk
initrd: $(BUILD_DIR)
	@echo "Creating initial RAM disk..."
	@cp -r apps/* $(INITRD_DIR)/ 2>/dev/null || true
	@echo "#!/bin/sh" > $(INITRD_DIR)/init
	@echo "echo 'RaeenOS Initial RAM Disk loaded successfully'" >> $(INITRD_DIR)/init
	@echo "exec /bin/sh" >> $(INITRD_DIR)/init
	@chmod +x $(INITRD_DIR)/init
	@cd $(INITRD_DIR) && find . | cpio -o -H newc | gzip > ../initrd.img
	@echo "✓ Initial RAM disk created"

# Create bootable ISO image
iso: bootloader kernel initrd
	@echo "Creating bootable ISO image..."
	@cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	@cp $(BUILD_DIR)/initrd.img $(ISO_DIR)/boot/
	@echo 'set timeout=3' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "RaeenOS $(VERSION_STRING) - $(TARGET_ARCH)" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/kernel.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    module2 /boot/initrd.img' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "RaeenOS $(VERSION_STRING) - Safe Mode" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/kernel.bin safe_mode=1' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    module2 /boot/initrd.img' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(BUILD_DIR)/raeenos.iso $(ISO_DIR)
	@echo "✓ Bootable ISO created: $(BUILD_DIR)/raeenos.iso"

# Comprehensive testing suite
test: iso
	@echo "Running RaeenOS test suite..."
	@powershell -ExecutionPolicy Bypass -File test_raeenos.ps1
	@echo "✓ Test suite completed"

# Validate codebase
validate: 
	@echo "Validating RaeenOS codebase..."
	@powershell -ExecutionPolicy Bypass -File test_raeenos.ps1
	@echo "✓ Codebase validation completed"

# QEMU testing with comprehensive options
qemu-test: iso
	@echo "Starting RaeenOS in QEMU test environment..."
	@echo "QEMU Options: 4GB RAM, KVM acceleration, VGA graphics, network"
	qemu-system-x86_64 \
		-cdrom $(BUILD_DIR)/raeenos.iso \
		-m 4G \
		-cpu qemu64,+avx,+avx2,+sse4.2 \
		-smp 4 \
		-device VGA \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial stdio \
		-monitor telnet:127.0.0.1:1234,server,nowait \
		-enable-kvm 2>/dev/null || \
	qemu-system-x86_64 \
		-cdrom $(BUILD_DIR)/raeenos.iso \
		-m 4G \
		-cpu qemu64,+avx,+avx2,+sse4.2 \
		-smp 4 \
		-device VGA \
		-netdev user,id=net0 \
		-device e1000,netdev=net0 \
		-serial stdio \
		-monitor telnet:127.0.0.1:1234,server,nowait

# Quick QEMU run
run: iso
	@echo "Running RaeenOS in QEMU..."
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/raeenos.iso -m 2G -enable-kvm 2>/dev/null || \
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/raeenos.iso -m 2G

# Debug with QEMU and GDB
debug: iso
	@echo "Starting RaeenOS debug session..."
	@echo "Connect GDB to localhost:1234"
	qemu-system-x86_64 \
		-cdrom $(BUILD_DIR)/raeenos.iso \
		-m 2G \
		-s -S \
		-monitor stdio

# Performance testing
perf-test: iso
	@echo "Running performance tests..."
	qemu-system-x86_64 \
		-cdrom $(BUILD_DIR)/raeenos.iso \
		-m 8G \
		-cpu host \
		-smp 8 \
		-enable-kvm \
		-device VGA \
		-serial file:$(BUILD_DIR)/perf-test.log \
		-monitor telnet:127.0.0.1:1234,server,nowait

# Clean all build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(BOOTLOADER_DIR) clean 2>/dev/null || true
	@$(MAKE) -C $(KERNEL_DIR) clean 2>/dev/null || true
	@$(MAKE) -C $(UI_DIR) clean 2>/dev/null || true
	@$(MAKE) -C $(TOOLS_DIR) clean 2>/dev/null || true
	@echo "✓ Clean completed"

# Install to system (development)
install: all
	@echo "Installing RaeenOS for development..."
	@echo "This would install RaeenOS components to the system"
	@echo "Not implemented for safety - use 'make run' for testing"

# Show build information
info:
	@echo "=== RaeenOS Build Information ==="
	@echo "Version: $(VERSION_STRING)"
	@echo "Target Architecture: $(TARGET_ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Compiler: $(CC)"
	@echo "Assembler: $(AS)"
	@echo "Linker: $(LD)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build complete OS with ISO"
	@echo "  bootloader - Build bootloader only"
	@echo "  kernel     - Build kernel only"
	@echo "  ui         - Build UI framework only"
	@echo "  tools      - Build development tools"
	@echo "  iso        - Create bootable ISO image"
	@echo "  test       - Run comprehensive test suite"
	@echo "  validate   - Validate codebase"
	@echo "  run        - Run RaeenOS in QEMU"
	@echo "  qemu-test  - Run with full QEMU testing options"
	@echo "  debug      - Start debug session with GDB"
	@echo "  perf-test  - Run performance tests"
	@echo "  clean      - Clean all build artifacts"
	@echo "  info       - Show this information"

# Help target
help: info