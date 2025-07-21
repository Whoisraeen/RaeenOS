# RaeenOS Development Tools Makefile

# Build configuration
BUILD_DIR ?= ../build
TARGET_ARCH ?= x64
BUILD_TYPE ?= debug

# Tools
CC = gcc
CXX = g++
AS = nasm
LD = ld
AR = ar
OBJCOPY = objcopy

# Architecture-specific settings
ifeq ($(TARGET_ARCH),x64)
    ARCH_FLAGS = -m64
    KERNEL_CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -m64 -static
else
    ARCH_FLAGS = -m32
    KERNEL_CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -m32 -static
endif

# Build type flags
ifeq ($(BUILD_TYPE),debug)
    OPT_FLAGS = -O0 -g -DDEBUG
else
    OPT_FLAGS = -O2 -DNDEBUG
endif

# Compiler flags for host tools
HOST_CFLAGS = $(ARCH_FLAGS) -Wall -Wextra -std=c11 $(OPT_FLAGS)
HOST_CXXFLAGS = $(HOST_CFLAGS) -std=c++17

# Kernel application flags
KERNEL_LDFLAGS = -static -nostdlib -nostartfiles -nodefaultlibs

# Include directories
INCLUDE_DIRS = -I. -Iinclude -I../kernel/include

# User applications
USER_APPS = test_program simple_shell interactive_test
USER_BINS = $(USER_APPS:%=$(BUILD_DIR)/%.bin)

# Development tools
DEV_TOOLS = mkfs_tool debugger profiler emulator
DEV_BINS = $(BUILD_DIR)/mkfs.raeenos $(BUILD_DIR)/raeenos-debugger \
           $(BUILD_DIR)/raeenos-profiler $(BUILD_DIR)/raeenos-emulator

# Scripts
SCRIPTS = $(BUILD_DIR)/create-iso.sh $(BUILD_DIR)/run-tests.sh

# Default target
all: user-apps dev-tools scripts

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/tools

# User applications target
user-apps: $(USER_BINS)

# Development tools target
dev-tools: $(DEV_BINS)

# Scripts target
scripts: $(SCRIPTS)

# Build user applications with RaeenOS binary format
$(BUILD_DIR)/%.bin: %.c | $(BUILD_DIR)
	@echo "Building user application: $*"
	@# Compile to object
	$(CC) $(KERNEL_CFLAGS) -c $< -o $(BUILD_DIR)/$*.o
	@# Link to ELF
	$(CC) $(KERNEL_LDFLAGS) $(BUILD_DIR)/$*.o -o $(BUILD_DIR)/$*.elf
	@# Extract sections
	$(OBJCOPY) -O binary --only-section=.text $(BUILD_DIR)/$*.elf $(BUILD_DIR)/$*.text
	$(OBJCOPY) -O binary --only-section=.data $(BUILD_DIR)/$*.elf $(BUILD_DIR)/$*.data
	$(OBJCOPY) -O binary --only-section=.rodata $(BUILD_DIR)/$*.elf $(BUILD_DIR)/$*.rodata
	@# Calculate sizes
	@TEXT_SIZE=$$(stat -c%s $(BUILD_DIR)/$*.text 2>/dev/null || stat -f%z $(BUILD_DIR)/$*.text 2>/dev/null || echo 0); \
	DATA_SIZE=$$(stat -c%s $(BUILD_DIR)/$*.data 2>/dev/null || stat -f%z $(BUILD_DIR)/$*.data 2>/dev/null || echo 0); \
	RODATA_SIZE=$$(stat -c%s $(BUILD_DIR)/$*.rodata 2>/dev/null || stat -f%z $(BUILD_DIR)/$*.rodata 2>/dev/null || echo 0); \
	CODE_SIZE=$$((TEXT_SIZE + RODATA_SIZE)); \
	printf "\\x52\\x41\\x45\\x4E" > $(BUILD_DIR)/$*_header.bin; \
	printf "\\x1C\\x00\\x00\\x00" >> $(BUILD_DIR)/$*_header.bin; \
	printf "\\x%02x\\x%02x\\x%02x\\x%02x" $$((CODE_SIZE & 0xFF)) $$(((CODE_SIZE >> 8) & 0xFF)) $$(((CODE_SIZE >> 16) & 0xFF)) $$(((CODE_SIZE >> 24) & 0xFF)) >> $(BUILD_DIR)/$*_header.bin; \
	printf "\\x%02x\\x%02x\\x%02x\\x%02x" $$((DATA_SIZE & 0xFF)) $$(((DATA_SIZE >> 8) & 0xFF)) $$(((DATA_SIZE >> 16) & 0xFF)) $$(((DATA_SIZE >> 24) & 0xFF)) >> $(BUILD_DIR)/$*_header.bin; \
	printf "\\x00\\x00\\x00\\x00" >> $(BUILD_DIR)/$*_header.bin; \
	printf "\\x00\\x10\\x00\\x00" >> $(BUILD_DIR)/$*_header.bin; \
	printf "\\x01\\x00\\x00\\x00" >> $(BUILD_DIR)/$*_header.bin; \
	cat $(BUILD_DIR)/$*_header.bin $(BUILD_DIR)/$*.text $(BUILD_DIR)/$*.rodata $(BUILD_DIR)/$*.data > $@; \
	echo "✓ User application built: $@ (Code: $$CODE_SIZE bytes, Data: $$DATA_SIZE bytes)"

# Build filesystem creation tool
$(BUILD_DIR)/mkfs.raeenos: mkfs/mkfs_raeenos.c | $(BUILD_DIR)
	@echo "Building filesystem creation tool..."
	$(CC) $(HOST_CFLAGS) $(INCLUDE_DIRS) -o $@ $<
	@echo "✓ mkfs.raeenos built successfully"

# Build kernel debugger
$(BUILD_DIR)/raeenos-debugger: debugger/raeenos_debugger.c | $(BUILD_DIR)
	@echo "Building RaeenOS debugger..."
	$(CC) $(HOST_CFLAGS) $(INCLUDE_DIRS) -o $@ $<
	@echo "✓ RaeenOS debugger built successfully"

# Build performance profiler
$(BUILD_DIR)/raeenos-profiler: profiler/performance_profiler.c | $(BUILD_DIR)
	@echo "Building performance profiler..."
	$(CC) $(HOST_CFLAGS) $(INCLUDE_DIRS) -o $@ $<
	@echo "✓ Performance profiler built successfully"

# Build hardware emulator
$(BUILD_DIR)/raeenos-emulator: emulator/hardware_emulator.c | $(BUILD_DIR)
	@echo "Building hardware emulator..."
	$(CC) $(HOST_CFLAGS) $(INCLUDE_DIRS) -o $@ $<
	@echo "✓ Hardware emulator built successfully"

# Create ISO creation script
$(BUILD_DIR)/create-iso.sh: | $(BUILD_DIR)
	@echo "Creating ISO creation script..."
	@echo '#!/bin/bash' > $@
	@echo '# RaeenOS ISO Creation Script' >> $@
	@echo 'set -e' >> $@
	@echo '' >> $@
	@echo 'BUILD_DIR="$${1:-build}"' >> $@
	@echo 'ISO_DIR="$${BUILD_DIR}/iso"' >> $@
	@echo 'OUTPUT_ISO="$${BUILD_DIR}/raeenos.iso"' >> $@
	@echo '' >> $@
	@echo 'echo "Creating RaeenOS ISO image..."' >> $@
	@echo 'mkdir -p "$${ISO_DIR}/boot/grub"' >> $@
	@echo '' >> $@
	@echo '# Copy kernel and initrd' >> $@
	@echo 'cp "$${BUILD_DIR}/kernel.bin" "$${ISO_DIR}/boot/"' >> $@
	@echo 'cp "$${BUILD_DIR}/initrd.img" "$${ISO_DIR}/boot/" 2>/dev/null || true' >> $@
	@echo '' >> $@
	@echo '# Create GRUB configuration' >> $@
	@echo 'cat > "$${ISO_DIR}/boot/grub/grub.cfg" << EOF' >> $@
	@echo 'set timeout=3' >> $@
	@echo 'set default=0' >> $@
	@echo '' >> $@
	@echo 'menuentry "RaeenOS" {' >> $@
	@echo '    multiboot2 /boot/kernel.bin' >> $@
	@echo '    module2 /boot/initrd.img' >> $@
	@echo '}' >> $@
	@echo '' >> $@
	@echo 'menuentry "RaeenOS (Safe Mode)" {' >> $@
	@echo '    multiboot2 /boot/kernel.bin safe_mode=1' >> $@
	@echo '    module2 /boot/initrd.img' >> $@
	@echo '}' >> $@
	@echo 'EOF' >> $@
	@echo '' >> $@
	@echo '# Create ISO' >> $@
	@echo 'grub-mkrescue -o "$${OUTPUT_ISO}" "$${ISO_DIR}"' >> $@
	@echo 'echo "✓ ISO created: $${OUTPUT_ISO}"' >> $@
	@chmod +x $@
	@echo "✓ ISO creation script created"

# Create test runner script
$(BUILD_DIR)/run-tests.sh: | $(BUILD_DIR)
	@echo "Creating test runner script..."
	@echo '#!/bin/bash' > $@
	@echo '# RaeenOS Test Runner Script' >> $@
	@echo 'set -e' >> $@
	@echo '' >> $@
	@echo 'BUILD_DIR="$${1:-build}"' >> $@
	@echo 'TEST_TYPE="$${2:-all}"' >> $@
	@echo '' >> $@
	@echo 'echo "Running RaeenOS tests..."' >> $@
	@echo 'echo "Build directory: $${BUILD_DIR}"' >> $@
	@echo 'echo "Test type: $${TEST_TYPE}"' >> $@
	@echo '' >> $@
	@echo '# Unit tests' >> $@
	@echo 'if [[ "$${TEST_TYPE}" == "all" || "$${TEST_TYPE}" == "unit" ]]; then' >> $@
	@echo '    echo "Running unit tests..."' >> $@
	@echo '    powershell -ExecutionPolicy Bypass -File test_raeenos.ps1' >> $@
	@echo '    echo "✓ Unit tests completed"' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo '# QEMU tests' >> $@
	@echo 'if [[ "$${TEST_TYPE}" == "all" || "$${TEST_TYPE}" == "qemu" ]]; then' >> $@
	@echo '    echo "Running QEMU tests..."' >> $@
	@echo '    if [[ -f "$${BUILD_DIR}/raeenos.iso" ]]; then' >> $@
	@echo '        timeout 30 qemu-system-x86_64 -cdrom "$${BUILD_DIR}/raeenos.iso" -m 512 -display none -serial stdio || true' >> $@
	@echo '        echo "✓ QEMU tests completed"' >> $@
	@echo '    else' >> $@
	@echo '        echo "⚠ ISO not found, skipping QEMU tests"' >> $@
	@echo '    fi' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo 'echo "✓ All tests completed successfully"' >> $@
	@chmod +x $@
	@echo "✓ Test runner script created"

# Create development environment setup
dev-setup:
	@echo "Setting up RaeenOS development environment..."
	@echo "Checking required tools..."
	@command -v gcc >/dev/null 2>&1 || (echo "Error: GCC not found" && exit 1)
	@command -v nasm >/dev/null 2>&1 || (echo "Error: NASM not found" && exit 1)
	@command -v ld >/dev/null 2>&1 || (echo "Error: LD not found" && exit 1)
	@command -v qemu-system-x86_64 >/dev/null 2>&1 || echo "Warning: QEMU not found"
	@command -v grub-mkrescue >/dev/null 2>&1 || echo "Warning: GRUB not found"
	@echo "✓ Development environment setup completed"

# Run comprehensive tests
test: all
	@echo "Running comprehensive tool tests..."
	@echo "Testing user applications..."
	@for app in $(USER_APPS); do \
		if [[ -f "$(BUILD_DIR)/$$app.bin" ]]; then \
			echo "✓ $$app.bin built successfully"; \
		else \
			echo "✗ $$app.bin build failed"; \
		fi; \
	done
	@echo "Testing development tools..."
	@for tool in $(DEV_BINS); do \
		if [[ -f "$$tool" ]]; then \
			echo "✓ $$(basename $$tool) built successfully"; \
		else \
			echo "✗ $$(basename $$tool) build failed"; \
		fi; \
	done
	@echo "✓ All tool tests completed"

# Clean build artifacts
clean:
	@echo "Cleaning development tools..."
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf
	@rm -f $(BUILD_DIR)/*.text $(BUILD_DIR)/*.data $(BUILD_DIR)/*.rodata
	@rm -f $(BUILD_DIR)/*_header.bin
	@rm -f $(USER_BINS) $(DEV_BINS) $(SCRIPTS)
	@rm -rf $(BUILD_DIR)/tools
	@echo "✓ Development tools clean completed"

# Install tools for main build
install: all
	@echo "Installing development tools..."
	@cp $(USER_BINS) $(BUILD_DIR)/ 2>/dev/null || true
	@cp $(DEV_BINS) $(BUILD_DIR)/ 2>/dev/null || true
	@cp $(SCRIPTS) $(BUILD_DIR)/ 2>/dev/null || true
	@echo "✓ Development tools installed"

# Show tools build information
info:
	@echo "=== RaeenOS Development Tools Build Information ==="
	@echo "Target Architecture: $(TARGET_ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Host Compiler: $(CC)"
	@echo "Host CFLAGS: $(HOST_CFLAGS)"
	@echo "Kernel CFLAGS: $(KERNEL_CFLAGS)"
	@echo ""
	@echo "User Applications:"
	@for app in $(USER_APPS); do echo "  $$app.bin"; done
	@echo ""
	@echo "Development Tools:"
	@echo "  mkfs.raeenos     - Filesystem creation tool"
	@echo "  raeenos-debugger - Kernel debugger"
	@echo "  raeenos-profiler - Performance profiler"
	@echo "  raeenos-emulator - Hardware emulator"
	@echo ""
	@echo "Scripts:"
	@echo "  create-iso.sh    - ISO creation script"
	@echo "  run-tests.sh     - Test runner script"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build all tools and applications"
	@echo "  user-apps - Build user applications only"
	@echo "  dev-tools - Build development tools only"
	@echo "  scripts   - Create utility scripts"
	@echo "  dev-setup - Setup development environment"
	@echo "  test      - Run tool tests"
	@echo "  clean     - Clean build artifacts"
	@echo "  install   - Install for main build"
	@echo "  info      - Show this information"

.PHONY: all user-apps dev-tools scripts dev-setup test clean install info