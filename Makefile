# RaeenOS Build System - Modern, Portable Makefile
# Supports cross-compilation, multiple targets, and automated dependency management

# Build Configuration
OS := $(shell uname -s)
ARCH := $(shell uname -m)
BUILD_TYPE ?= release
TARGET_ARCH ?= x86_64
CROSS_COMPILE ?= 
VERBOSE ?= 0

# Toolchain Configuration
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS := $(CROSS_COMPILE)nasm
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy
STRIP := $(CROSS_COMPILE)strip

# Build Directories
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ISO_DIR := $(BUILD_DIR)/iso
DOC_DIR := $(BUILD_DIR)/docs
TEST_DIR := $(BUILD_DIR)/tests

# Source Directories
KERNEL_DIR := kernel
BOOTLOADER_DIR := bootloader
DRIVERS_DIR := $(KERNEL_DIR)/drivers
GUI_DIR := $(KERNEL_DIR)/gui
APPS_DIR := $(KERNEL_DIR)/apps
TOOLS_DIR := tools
DOCS_DIR := docs

# Compiler Flags
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic
ASFLAGS := -f elf64
LDFLAGS := -nostdlib -nostartfiles -nodefaultlibs

# Architecture-specific flags
ifeq ($(TARGET_ARCH),x86_64)
    CFLAGS += -m64 -march=x86-64 -mtune=generic
    CXXFLAGS += -m64 -march=x86-64 -mtune=generic
    ASFLAGS += -f elf64
    LDFLAGS += -m elf_x86_64
else ifeq ($(TARGET_ARCH),i386)
    CFLAGS += -m32 -march=i386 -mtune=generic
    CXXFLAGS += -m32 -march=i386 -mtune=generic
    ASFLAGS += -f elf32
    LDFLAGS += -m elf_i386
else ifeq ($(TARGET_ARCH),aarch64)
    CFLAGS += -march=armv8-a -mtune=generic
    CXXFLAGS += -march=armv8-a -mtune=generic
    ASFLAGS += -f elf64
    LDFLAGS += -m aarch64linux
endif

# Build type flags
ifeq ($(BUILD_TYPE),debug)
    CFLAGS += -g -O0 -DDEBUG
    CXXFLAGS += -g -O0 -DDEBUG
    ASFLAGS += -g
else ifeq ($(BUILD_TYPE),release)
    CFLAGS += -O2 -DNDEBUG
    CXXFLAGS += -O2 -DNDEBUG
    LDFLAGS += -s
else ifeq ($(BUILD_TYPE),profile)
    CFLAGS += -O2 -g -pg -DNDEBUG
    CXXFLAGS += -O2 -g -pg -DNDEBUG
    LDFLAGS += -pg
endif

# Verbose output
ifeq ($(VERBOSE),1)
    Q :=
    VECHO = @echo
else
    Q := @
    VECHO = @echo
endif

# Include directories
INCLUDES := -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/core/include -I$(KERNEL_DIR)/drivers/include

# Library directories
LIBDIRS := -L$(BIN_DIR)

# Libraries
LIBS := -lkernel -ldrivers -lgui -lapps

# Find all source files
KERNEL_SOURCES := $(shell find $(KERNEL_DIR) -name "*.c" -o -name "*.cpp")
BOOTLOADER_SOURCES := $(shell find $(BOOTLOADER_DIR) -name "*.asm" -o -name "*.c")
DRIVER_SOURCES := $(shell find $(DRIVERS_DIR) -name "*.c" -o -name "*.cpp")
GUI_SOURCES := $(shell find $(GUI_DIR) -name "*.c" -o -name "*.cpp")
APP_SOURCES := $(shell find $(APPS_DIR) -name "*.c" -o -name "*.cpp")

# Generate object file lists
KERNEL_OBJECTS := $(KERNEL_SOURCES:%.c=$(OBJ_DIR)/%.o)
KERNEL_OBJECTS := $(KERNEL_OBJECTS:%.cpp=$(OBJ_DIR)/%.o)
BOOTLOADER_OBJECTS := $(BOOTLOADER_SOURCES:%.asm=$(OBJ_DIR)/%.o)
BOOTLOADER_OBJECTS := $(BOOTLOADER_OBJECTS:%.c=$(OBJ_DIR)/%.o)
DRIVER_OBJECTS := $(DRIVER_SOURCES:%.c=$(OBJ_DIR)/%.o)
DRIVER_OBJECTS := $(DRIVER_OBJECTS:%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJECTS := $(GUI_SOURCES:%.c=$(OBJ_DIR)/%.o)
GUI_OBJECTS := $(GUI_OBJECTS:%.cpp=$(OBJ_DIR)/%.o)
APP_OBJECTS := $(APP_SOURCES:%.c=$(OBJ_DIR)/%.o)
APP_OBJECTS := $(APP_OBJECTS:%.cpp=$(OBJ_DIR)/%.o)

# Main targets
.PHONY: all clean distclean setup test install uninstall help

# Default target
all: setup kernel drivers gui apps iso

# Setup build directories
setup:
	$(VECHO) "Setting up build directories..."
	$(Q)mkdir -p $(OBJ_DIR) $(BIN_DIR) $(ISO_DIR) $(DOC_DIR) $(TEST_DIR)
	$(Q)mkdir -p $(OBJ_DIR)/$(KERNEL_DIR) $(OBJ_DIR)/$(BOOTLOADER_DIR)
	$(Q)mkdir -p $(OBJ_DIR)/$(DRIVERS_DIR) $(OBJ_DIR)/$(GUI_DIR) $(OBJ_DIR)/$(APPS_DIR)

# Kernel compilation
kernel: $(BIN_DIR)/raeenos_kernel.bin

$(BIN_DIR)/raeenos_kernel.bin: $(KERNEL_OBJECTS) $(BOOTLOADER_OBJECTS)
	$(VECHO) "Linking kernel..."
	$(Q)$(LD) $(LDFLAGS) -T $(KERNEL_DIR)/kernel.ld -o $@ $^ $(LIBS)
	$(Q)$(OBJCOPY) -O binary $@ $(BIN_DIR)/raeenos_kernel.raw

# Drivers compilation
drivers: $(BIN_DIR)/libdrivers.a

$(BIN_DIR)/libdrivers.a: $(DRIVER_OBJECTS)
	$(VECHO) "Building drivers library..."
	$(Q)$(AR) rcs $@ $^

# GUI compilation
gui: $(BIN_DIR)/libgui.a

$(BIN_DIR)/libgui.a: $(GUI_OBJECTS)
	$(VECHO) "Building GUI library..."
	$(Q)$(AR) rcs $@ $^

# Applications compilation
apps: $(BIN_DIR)/libapps.a

$(BIN_DIR)/libapps.a: $(APP_OBJECTS)
	$(VECHO) "Building applications library..."
	$(Q)$(AR) rcs $@ $^

# ISO creation
iso: kernel drivers gui apps
	$(VECHO) "Creating bootable ISO..."
	$(Q)mkdir -p $(ISO_DIR)/boot/grub
	$(Q)cp $(BIN_DIR)/raeenos_kernel.raw $(ISO_DIR)/boot/
	$(Q)cp $(BOOTLOADER_DIR)/grub.cfg $(ISO_DIR)/boot/grub/
	$(Q)grub-mkrescue -o $(BIN_DIR)/raeenos.iso $(ISO_DIR)

# Compilation rules
$(OBJ_DIR)/%.o: %.c
	$(VECHO) "Compiling $<..."
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(VECHO) "Compiling $<..."
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.asm
	$(VECHO) "Assembling $<..."
	$(Q)mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@

# Testing
test: setup
	$(VECHO) "Running tests..."
	$(Q)$(MAKE) -C $(TEST_DIR) test

# Documentation
docs: setup
	$(VECHO) "Generating documentation..."
	$(Q)doxygen Doxyfile
	$(Q)cp -r $(DOCS_DIR)/* $(DOC_DIR)/

# Installation
install: all
	$(VECHO) "Installing RaeenOS..."
	$(Q)sudo cp $(BIN_DIR)/raeenos.iso /boot/raeenos.iso
	$(Q)sudo update-grub

# Uninstallation
uninstall:
	$(VECHO) "Uninstalling RaeenOS..."
	$(Q)sudo rm -f /boot/raeenos.iso
	$(Q)sudo update-grub

# Cleaning
clean:
	$(VECHO) "Cleaning build artifacts..."
	$(Q)rm -rf $(BUILD_DIR)

distclean: clean
	$(VECHO) "Deep cleaning..."
	$(Q)find . -name "*.o" -delete
	$(Q)find . -name "*.a" -delete
	$(Q)find . -name "*.so" -delete
	$(Q)find . -name "*.d" -delete

# Development targets
debug: BUILD_TYPE=debug
debug: all

release: BUILD_TYPE=release
release: all

profile: BUILD_TYPE=profile
profile: all

# Cross-compilation targets
x86_64: TARGET_ARCH=x86_64
x86_64: all

i386: TARGET_ARCH=i386
i386: all

aarch64: TARGET_ARCH=aarch64
aarch64: all

# Verbose builds
verbose: VERBOSE=1
verbose: all

# Help
help:
	@echo "RaeenOS Build System"
	@echo "===================="
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build complete system (default)"
	@echo "  setup        - Create build directories"
	@echo "  kernel       - Build kernel only"
	@echo "  drivers      - Build drivers only"
	@echo "  gui          - Build GUI only"
	@echo "  apps         - Build applications only"
	@echo "  iso          - Create bootable ISO"
	@echo "  test         - Run tests"
	@echo "  docs         - Generate documentation"
	@echo "  install      - Install to system"
	@echo "  uninstall    - Remove from system"
	@echo "  clean        - Clean build artifacts"
	@echo "  distclean    - Deep clean"
	@echo ""
	@echo "Build Types:"
	@echo "  debug        - Debug build"
	@echo "  release      - Release build (default)"
	@echo "  profile      - Profiling build"
	@echo ""
	@echo "Architectures:"
	@echo "  x86_64       - 64-bit x86 (default)"
	@echo "  i386         - 32-bit x86"
	@echo "  aarch64      - 64-bit ARM"
	@echo ""
	@echo "Options:"
	@echo "  verbose      - Verbose output"
	@echo "  CROSS_COMPILE=prefix - Cross-compilation"
	@echo ""
	@echo "Examples:"
	@echo "  make debug verbose"
	@echo "  make CROSS_COMPILE=aarch64-linux-gnu- aarch64"
	@echo "  make clean && make release"

# Dependency generation
%.d: %.c
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

%.d: %.cpp
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# Include dependency files
-include $(KERNEL_SOURCES:.c=.d)
-include $(KERNEL_SOURCES:.cpp=.d)
-include $(DRIVER_SOURCES:.c=.d)
-include $(DRIVER_SOURCES:.cpp=.d)
-include $(GUI_SOURCES:.c=.d)
-include $(GUI_SOURCES:.cpp=.d)
-include $(APP_SOURCES:.c=.d)
-include $(APP_SOURCES:.cpp=.d)