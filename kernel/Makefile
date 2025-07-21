# RaeenOS Kernel Makefile

# Build configuration
BUILD_DIR ?= ../build
TARGET_ARCH ?= x64
BUILD_TYPE ?= debug

# Tools
CC = gcc
CXX = g++
AS = nasm
LD = ld
OBJCOPY = objcopy
AR = ar

# Architecture-specific settings
ifeq ($(TARGET_ARCH),x64)
    ARCH_FLAGS = -m64 -mcmodel=kernel
    NASM_FLAGS = -f elf64 -D X64_MODE
    LD_FLAGS = -m elf_x86_64
    ARCH_DIR = arch/x86_64
else
    ARCH_FLAGS = -m32
    NASM_FLAGS = -f elf32 -D X86_MODE
    LD_FLAGS = -m elf_i386
    ARCH_DIR = arch/x86
endif

# Build type flags
ifeq ($(BUILD_TYPE),debug)
    OPT_FLAGS = -O0 -g -DDEBUG
    DEBUG_FLAGS = -ggdb3 -gdwarf-4
else
    OPT_FLAGS = -O2 -DNDEBUG
    DEBUG_FLAGS = 
endif

# Compiler flags
BASE_CFLAGS = $(ARCH_FLAGS) -ffreestanding -fno-pie -no-pie -fno-stack-protector \
              -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone \
              -Wall -Wextra -Werror -std=c11 $(OPT_FLAGS) $(DEBUG_FLAGS)

# AI and performance optimization flags
AI_FLAGS = -mavx2 -mfma -msse4.2 -DAI_ACCELERATION_ENABLED
PERF_FLAGS = -march=native -mtune=native -ffast-math -funroll-loops

# Security hardening flags
SECURITY_FLAGS = -fstack-clash-protection -fcf-protection=full \
                 -D_FORTIFY_SOURCE=2 -fPIE -Wformat -Wformat-security

# Final CFLAGS
CFLAGS ?= $(BASE_CFLAGS) $(AI_FLAGS) $(PERF_FLAGS) $(SECURITY_FLAGS)
CXXFLAGS ?= $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti
ASFLAGS ?= $(NASM_FLAGS)

# Linker flags
LDFLAGS ?= $(LD_FLAGS) -T kernel.ld -nostdlib -z noexecstack -z relro -z now

# Include directories
INCLUDE_DIRS = -I. -Icore/include -Iinclude -I$(ARCH_DIR) \
               -Imemory/include -Iprocess/include -Ifilesystem/include \
               -Idrivers/include -Inetwork/include -Igui/include \
               -Isecurity/include -Iperformance/include -Iapp/include \
               -Iusers/include -Iai/include -Iclustering/include \
               -Ivirtualization/include -Iquantum/include -Ixr/include \
               -Iblockchain/include -Igaming/include -Icreative/include \
               -Ienterprise/include -Iux/include -Icompatibility/include \
               -Ireliability/include -Iinit/include -Ishell/include \
               -Idiagnostics/include

# Source files discovery
C_SOURCES = $(shell find . -name "*.c" -not -path "./$(BUILD_DIR)/*" -not -path "./obj/*")
CXX_SOURCES = $(shell find . -name "*.cpp" -not -path "./$(BUILD_DIR)/*" -not -path "./obj/*")
ASM_SOURCES = $(shell find . -name "*.S" -not -path "./$(BUILD_DIR)/*" -not -path "./obj/*")
NASM_SOURCES = $(shell find . -name "*.asm" -not -path "./$(BUILD_DIR)/*" -not -path "./obj/*")

# Object files
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/kernel/%.o)
CXX_OBJECTS = $(CXX_SOURCES:%.cpp=$(BUILD_DIR)/kernel/%.o)
ASM_OBJECTS = $(ASM_SOURCES:%.S=$(BUILD_DIR)/kernel/%.o)
NASM_OBJECTS = $(NASM_SOURCES:%.asm=$(BUILD_DIR)/kernel/%.o)

ALL_OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS) $(ASM_OBJECTS) $(NASM_OBJECTS)

# Target kernel binary
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

# Default target
all: $(KERNEL_BIN)

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/kernel/core
	@mkdir -p $(BUILD_DIR)/kernel/memory
	@mkdir -p $(BUILD_DIR)/kernel/process
	@mkdir -p $(BUILD_DIR)/kernel/filesystem
	@mkdir -p $(BUILD_DIR)/kernel/drivers
	@mkdir -p $(BUILD_DIR)/kernel/network
	@mkdir -p $(BUILD_DIR)/kernel/gui
	@mkdir -p $(BUILD_DIR)/kernel/security
	@mkdir -p $(BUILD_DIR)/kernel/ai
	@mkdir -p $(BUILD_DIR)/kernel/$(ARCH_DIR)

# Compile C source files
$(BUILD_DIR)/kernel/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile C++ source files
$(BUILD_DIR)/kernel/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile GAS assembly files
$(BUILD_DIR)/kernel/%.o: %.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling GAS: $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile NASM assembly files
$(BUILD_DIR)/kernel/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling NASM: $<"
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel ELF
$(KERNEL_ELF): $(ALL_OBJECTS)
	@echo "Linking kernel ELF..."
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "✓ Kernel ELF linked successfully"

# Create kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Creating kernel binary..."
	$(OBJCOPY) -O binary $< $@
	@echo "✓ Kernel binary created: $@"
	@echo "Kernel size: $$(stat -c%s $@) bytes"

# Generate kernel symbols
symbols: $(KERNEL_ELF)
	@echo "Generating kernel symbols..."
	objdump -t $(KERNEL_ELF) > $(BUILD_DIR)/kernel.symbols
	nm $(KERNEL_ELF) > $(BUILD_DIR)/kernel.nm
	@echo "✓ Kernel symbols generated"

# Disassemble kernel
disasm: $(KERNEL_ELF)
	@echo "Disassembling kernel..."
	objdump -d $(KERNEL_ELF) > $(BUILD_DIR)/kernel.disasm
	@echo "✓ Kernel disassembly created"

# Analyze kernel
analyze: $(KERNEL_BIN) symbols disasm
	@echo "Analyzing kernel..."
	@echo "=== Kernel Analysis ==="
	@echo "Binary size: $$(stat -c%s $(KERNEL_BIN)) bytes"
	@echo "ELF size: $$(stat -c%s $(KERNEL_ELF)) bytes"
	@echo "Sections:"
	@objdump -h $(KERNEL_ELF)
	@echo ""
	@echo "Entry point:"
	@objdump -f $(KERNEL_ELF) | grep "start address"
	@echo ""
	@echo "Dependencies:"
	@objdump -p $(KERNEL_ELF) | grep NEEDED || echo "No external dependencies (good!)"

# Clean build artifacts
clean:
	@echo "Cleaning kernel artifacts..."
	@rm -rf $(BUILD_DIR)/kernel
	@rm -f $(KERNEL_BIN) $(KERNEL_ELF)
	@rm -f $(BUILD_DIR)/kernel.symbols $(BUILD_DIR)/kernel.nm $(BUILD_DIR)/kernel.disasm
	@echo "✓ Kernel clean completed"

# Install kernel for main build
install: $(KERNEL_BIN)
	@echo "Installing kernel..."
	@cp $(KERNEL_BIN) $(BUILD_DIR)/
	@echo "✓ Kernel installed"

# Test kernel with QEMU
test: $(KERNEL_BIN)
	@echo "Testing kernel in QEMU..."
	qemu-system-x86_64 -kernel $(KERNEL_BIN) -m 512 -display gtk \
		-serial stdio -monitor telnet:127.0.0.1:1234,server,nowait

# Debug kernel with QEMU and GDB
debug: $(KERNEL_ELF)
	@echo "Starting kernel debug session..."
	@echo "Connect GDB to localhost:1234"
	@echo "Use 'target remote localhost:1234' in GDB"
	qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512 -s -S \
		-serial stdio -monitor stdio

# Performance test
perf-test: $(KERNEL_BIN)
	@echo "Running kernel performance tests..."
	qemu-system-x86_64 -kernel $(KERNEL_BIN) -m 2G -smp 4 \
		-enable-kvm -cpu host -display none -serial stdio

# Show kernel build information
info:
	@echo "=== RaeenOS Kernel Build Information ==="
	@echo "Target Architecture: $(TARGET_ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo ""
	@echo "Source files:"
	@echo "  C sources: $(words $(C_SOURCES))"
	@echo "  C++ sources: $(words $(CXX_SOURCES))"
	@echo "  GAS assembly: $(words $(ASM_SOURCES))"
	@echo "  NASM assembly: $(words $(NASM_SOURCES))"
	@echo "  Total objects: $(words $(ALL_OBJECTS))"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build kernel binary"
	@echo "  symbols   - Generate symbol tables"
	@echo "  disasm    - Create disassembly"
	@echo "  analyze   - Analyze kernel binary"
	@echo "  clean     - Clean build artifacts"
	@echo "  install   - Install for main build"
	@echo "  test      - Test with QEMU"
	@echo "  debug     - Debug with QEMU + GDB"
	@echo "  perf-test - Performance testing"
	@echo "  info      - Show this information"

.PHONY: all symbols disasm analyze clean install test debug perf-test info