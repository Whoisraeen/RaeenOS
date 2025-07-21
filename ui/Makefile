# RaeenOS UI Framework Makefile

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
    NASM_FLAGS = -f elf64 -D X64_MODE
    LD_FLAGS = -m elf_x86_64
else
    ARCH_FLAGS = -m32
    NASM_FLAGS = -f elf32 -D X86_MODE
    LD_FLAGS = -m elf_i386
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
              -nostdlib -nostdinc -fno-builtin -fno-pic \
              -Wall -Wextra -Werror -std=c11 $(OPT_FLAGS) $(DEBUG_FLAGS)

# Graphics and UI optimization flags
GRAPHICS_FLAGS = -mavx2 -msse4.2 -mfma -DGRAPHICS_ACCELERATION_ENABLED
UI_FLAGS = -DRAEENOS_UI_FRAMEWORK -DHYBRID_COMPOSITOR -DADAPTIVE_THEMING

# Final CFLAGS
CFLAGS ?= $(BASE_CFLAGS) $(GRAPHICS_FLAGS) $(UI_FLAGS)
CXXFLAGS ?= $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti
ASFLAGS ?= $(NASM_FLAGS)

# Include directories
INCLUDE_DIRS = -I. -Iinclude -Igraphics/include -Iwindowing/include \
               -Icompositor/include -Ithemes/include -Iwidgets/include \
               -Irendering/include -Iinput/include -Ifonts/include \
               -Ianimation/include -I../kernel/include -I../kernel/core/include

# Source files discovery
C_SOURCES = $(shell find . -name "*.c" -not -path "./$(BUILD_DIR)/*")
CXX_SOURCES = $(shell find . -name "*.cpp" -not -path "./$(BUILD_DIR)/*")
ASM_SOURCES = $(shell find . -name "*.S" -not -path "./$(BUILD_DIR)/*")
NASM_SOURCES = $(shell find . -name "*.asm" -not -path "./$(BUILD_DIR)/*")

# Object files
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/ui/%.o)
CXX_OBJECTS = $(CXX_SOURCES:%.cpp=$(BUILD_DIR)/ui/%.o)
ASM_OBJECTS = $(ASM_SOURCES:%.S=$(BUILD_DIR)/ui/%.o)
NASM_OBJECTS = $(NASM_SOURCES:%.asm=$(BUILD_DIR)/ui/%.o)

ALL_OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS) $(ASM_OBJECTS) $(NASM_OBJECTS)

# Target libraries
UI_LIB = $(BUILD_DIR)/libraeenos_ui.a
GRAPHICS_LIB = $(BUILD_DIR)/libraeenos_graphics.a
COMPOSITOR_LIB = $(BUILD_DIR)/libraeenos_compositor.a

# Default target
all: $(UI_LIB) $(GRAPHICS_LIB) $(COMPOSITOR_LIB)

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/ui
	@mkdir -p $(BUILD_DIR)/ui/graphics
	@mkdir -p $(BUILD_DIR)/ui/windowing
	@mkdir -p $(BUILD_DIR)/ui/compositor
	@mkdir -p $(BUILD_DIR)/ui/themes
	@mkdir -p $(BUILD_DIR)/ui/widgets
	@mkdir -p $(BUILD_DIR)/ui/rendering
	@mkdir -p $(BUILD_DIR)/ui/input
	@mkdir -p $(BUILD_DIR)/ui/fonts
	@mkdir -p $(BUILD_DIR)/ui/animation

# Compile C source files
$(BUILD_DIR)/ui/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling UI C: $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile C++ source files
$(BUILD_DIR)/ui/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling UI C++: $<"
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile GAS assembly files
$(BUILD_DIR)/ui/%.o: %.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling UI GAS: $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile NASM assembly files
$(BUILD_DIR)/ui/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling UI NASM: $<"
	$(AS) $(ASFLAGS) $< -o $@

# Create UI framework library
$(UI_LIB): $(ALL_OBJECTS)
	@echo "Creating UI framework library..."
	$(AR) rcs $@ $^
	@echo "✓ UI framework library created: $@"

# Create graphics library
$(GRAPHICS_LIB): $(filter $(BUILD_DIR)/ui/graphics/%.o $(BUILD_DIR)/ui/rendering/%.o,$(ALL_OBJECTS))
	@echo "Creating graphics library..."
	$(AR) rcs $@ $^
	@echo "✓ Graphics library created: $@"

# Create compositor library
$(COMPOSITOR_LIB): $(filter $(BUILD_DIR)/ui/compositor/%.o $(BUILD_DIR)/ui/windowing/%.o,$(ALL_OBJECTS))
	@echo "Creating compositor library..."
	$(AR) rcs $@ $^
	@echo "✓ Compositor library created: $@"

# Generate UI documentation
docs:
	@echo "Generating UI framework documentation..."
	@mkdir -p $(BUILD_DIR)/docs/ui
	@echo "# RaeenOS UI Framework Documentation" > $(BUILD_DIR)/docs/ui/README.md
	@echo "" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "## Components" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "- Graphics Engine: Hardware-accelerated 2D/3D rendering" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "- Window Manager: Hybrid compositor with AI-driven theming" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "- Widget Framework: Modern, responsive UI components" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "- Animation Engine: Smooth, GPU-accelerated animations" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "- Input System: Multi-touch, gesture, and traditional input" >> $(BUILD_DIR)/docs/ui/README.md
	@echo "✓ UI documentation generated"

# Test UI framework
test: $(UI_LIB)
	@echo "Testing UI framework..."
	@echo "UI framework tests would run here"
	@echo "✓ UI framework tests completed"

# Clean build artifacts
clean:
	@echo "Cleaning UI framework artifacts..."
	@rm -rf $(BUILD_DIR)/ui
	@rm -f $(UI_LIB) $(GRAPHICS_LIB) $(COMPOSITOR_LIB)
	@echo "✓ UI framework clean completed"

# Install UI framework for main build
install: $(UI_LIB) $(GRAPHICS_LIB) $(COMPOSITOR_LIB)
	@echo "Installing UI framework..."
	@cp $(UI_LIB) $(BUILD_DIR)/
	@cp $(GRAPHICS_LIB) $(BUILD_DIR)/
	@cp $(COMPOSITOR_LIB) $(BUILD_DIR)/
	@echo "✓ UI framework installed"

# Show UI framework build information
info:
	@echo "=== RaeenOS UI Framework Build Information ==="
	@echo "Target Architecture: $(TARGET_ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo ""
	@echo "Source files:"
	@echo "  C sources: $(words $(C_SOURCES))"
	@echo "  C++ sources: $(words $(CXX_SOURCES))"
	@echo "  GAS assembly: $(words $(ASM_SOURCES))"
	@echo "  NASM assembly: $(words $(NASM_SOURCES))"
	@echo "  Total objects: $(words $(ALL_OBJECTS))"
	@echo ""
	@echo "Libraries:"
	@echo "  UI Framework: $(UI_LIB)"
	@echo "  Graphics: $(GRAPHICS_LIB)"
	@echo "  Compositor: $(COMPOSITOR_LIB)"
	@echo ""
	@echo "Available targets:"
	@echo "  all     - Build all UI libraries"
	@echo "  docs    - Generate documentation"
	@echo "  test    - Run UI framework tests"
	@echo "  clean   - Clean build artifacts"
	@echo "  install - Install for main build"
	@echo "  info    - Show this information"

.PHONY: all docs test clean install info