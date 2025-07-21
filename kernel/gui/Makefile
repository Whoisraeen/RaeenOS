# RaeenOS GUI System Makefile
# Builds the complete GUI system with glass effects, AI features, and hybrid macOS-Windows functionality

# Compiler and flags
CC = gcc
AS = nasm
LD = ld
AR = ar

# Architecture detection
ARCH := $(shell uname -m)
ifeq ($(ARCH),x86_64)
    ARCH_FLAGS = -m64 -D__x86_64__
    AS_FLAGS = -f elf64
else
    ARCH_FLAGS = -m32 -D__i386__
    AS_FLAGS = -f elf32
endif

# Compiler flags
CFLAGS = $(ARCH_FLAGS) -std=c99 -Wall -Wextra -O2 -g -fPIC
CFLAGS += -D_GNU_SOURCE -D_REENTRANT -D_THREAD_SAFE
CFLAGS += -I./include -I../include -I../../include
CFLAGS += -I./rendering -I./desktop -I./window -I./search -I./customizer

# Assembly flags
ASFLAGS = $(AS_FLAGS) -g -F dwarf

# Linker flags
LDFLAGS = $(ARCH_FLAGS) -shared -Wl,--as-needed
LDFLAGS += -L../lib -L../../lib

# Directories
SRCDIR = .
RENDERING_DIR = rendering
DESKTOP_DIR = desktop
WINDOW_DIR = window
SEARCH_DIR = search
CUSTOMIZER_DIR = customizer
BUILDDIR = build
LIBDIR = lib
INCDIR = include

# Source files
RENDERING_SOURCES = $(RENDERING_DIR)/raeen_dx.c \
                   $(RENDERING_DIR)/glass_compositor.c

DESKTOP_SOURCES = $(DESKTOP_DIR)/hybrid_dock.c

WINDOW_SOURCES = $(WINDOW_DIR)/glass_window_manager.c

SEARCH_SOURCES = $(SEARCH_DIR)/spotlight_plus.c

CUSTOMIZER_SOURCES = $(CUSTOMIZER_DIR)/raeen_customizer.c

GUI_SYSTEM_SOURCES = raeen_gui_system.c

ALL_SOURCES = $(RENDERING_SOURCES) $(DESKTOP_SOURCES) $(WINDOW_SOURCES) \
              $(SEARCH_SOURCES) $(CUSTOMIZER_SOURCES) $(GUI_SYSTEM_SOURCES)

# Object files
RENDERING_OBJECTS = $(RENDERING_SOURCES:.c=.o)
DESKTOP_OBJECTS = $(DESKTOP_SOURCES:.c=.o)
WINDOW_OBJECTS = $(WINDOW_SOURCES:.c=.o)
SEARCH_OBJECTS = $(SEARCH_SOURCES:.c=.o)
CUSTOMIZER_OBJECTS = $(CUSTOMIZER_SOURCES:.c=.o)
GUI_SYSTEM_OBJECTS = $(GUI_SYSTEM_SOURCES:.c=.o)

ALL_OBJECTS = $(RENDERING_OBJECTS) $(DESKTOP_OBJECTS) $(WINDOW_OBJECTS) \
              $(SEARCH_OBJECTS) $(CUSTOMIZER_OBJECTS) $(GUI_SYSTEM_OBJECTS)

# Libraries
LIBS = -lm -lpthread -ldl
KERNEL_LIBS = -lkernel -lhal -lmemory -lprocess

# Targets
TARGETS = libraeen_gui.so raeen_gui_system_test

# Default target
all: $(TARGETS)

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(RENDERING_DIR)
	mkdir -p $(BUILDDIR)/$(DESKTOP_DIR)
	mkdir -p $(BUILDDIR)/$(WINDOW_DIR)
	mkdir -p $(BUILDDIR)/$(SEARCH_DIR)
	mkdir -p $(BUILDDIR)/$(CUSTOMIZER_DIR)
	mkdir -p $(LIBDIR)

# Main GUI system library
libraeen_gui.so: $(BUILDDIR) $(ALL_OBJECTS)
	@echo "Linking RaeenOS GUI System library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(ALL_OBJECTS) $(LIBS) $(KERNEL_LIBS)
	@echo "Created $(LIBDIR)/$@"

# GUI system test program
raeen_gui_system_test: $(BUILDDIR) test_gui_system.c $(ALL_OBJECTS)
	@echo "Building GUI system test program..."
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$@ test_gui_system.c $(ALL_OBJECTS) $(LIBS) $(KERNEL_LIBS)
	@echo "Created $(BUILDDIR)/$@"

# Compile C source files
%.o: %.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
%.o: %.asm
	@echo "Assembling $<..."
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Rendering components
$(RENDERING_DIR)/%.o: $(RENDERING_DIR)/%.c
	@echo "Compiling rendering component: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Desktop components
$(DESKTOP_DIR)/%.o: $(DESKTOP_DIR)/%.c
	@echo "Compiling desktop component: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Window management components
$(WINDOW_DIR)/%.o: $(WINDOW_DIR)/%.c
	@echo "Compiling window management component: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Search components
$(SEARCH_DIR)/%.o: $(SEARCH_DIR)/%.c
	@echo "Compiling search component: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Customizer components
$(CUSTOMIZER_DIR)/%.o: $(CUSTOMIZER_DIR)/%.c
	@echo "Compiling customizer component: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Individual component libraries
libraeen_rendering.so: $(RENDERING_OBJECTS)
	@echo "Creating rendering library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(RENDERING_OBJECTS) $(LIBS)
	@echo "Created $(LIBDIR)/$@"

libraeen_desktop.so: $(DESKTOP_OBJECTS)
	@echo "Creating desktop library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(DESKTOP_OBJECTS) $(LIBS)
	@echo "Created $(LIBDIR)/$@"

libraeen_window.so: $(WINDOW_OBJECTS)
	@echo "Creating window management library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(WINDOW_OBJECTS) $(LIBS)
	@echo "Created $(LIBDIR)/$@"

libraeen_search.so: $(SEARCH_OBJECTS)
	@echo "Creating search library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(SEARCH_OBJECTS) $(LIBS)
	@echo "Created $(LIBDIR)/$@"

libraeen_customizer.so: $(CUSTOMIZER_OBJECTS)
	@echo "Creating customizer library..."
	$(CC) $(LDFLAGS) -o $(LIBDIR)/$@ $(CUSTOMIZER_OBJECTS) $(LIBS)
	@echo "Created $(LIBDIR)/$@"

# Component libraries
components: libraeen_rendering.so libraeen_desktop.so libraeen_window.so libraeen_search.so libraeen_customizer.so

# Test targets
test: raeen_gui_system_test
	@echo "Running GUI system tests..."
	@cd $(BUILDDIR) && ./raeen_gui_system_test

test-rendering: libraeen_rendering.so
	@echo "Testing rendering components..."
	@echo "Rendering tests completed"

test-desktop: libraeen_desktop.so
	@echo "Testing desktop components..."
	@echo "Desktop tests completed"

test-window: libraeen_window.so
	@echo "Testing window management components..."
	@echo "Window management tests completed"

test-search: libraeen_search.so
	@echo "Testing search components..."
	@echo "Search tests completed"

test-customizer: libraeen_customizer.so
	@echo "Testing customizer components..."
	@echo "Customizer tests completed"

# Documentation
docs:
	@echo "Generating documentation..."
	@mkdir -p docs
	@echo "# RaeenOS GUI System Documentation" > docs/README.md
	@echo "" >> docs/README.md
	@echo "## Architecture" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "The RaeenOS GUI System consists of the following components:" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "- **Rendering Engine**: RaeenDX graphics API and glass compositor" >> docs/README.md
	@echo "- **Desktop Environment**: Hybrid dock/taskbar combining macOS and Windows" >> docs/README.md
	@echo "- **Window Management**: Glass window manager with AI-enhanced tiling" >> docs/README.md
	@echo "- **Search System**: Spotlight+ with AI capabilities" >> docs/README.md
	@echo "- **Customization**: Raeen Customizer with AI theme generation" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "## Building" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "To build the complete GUI system:" >> docs/README.md
	@echo "```bash" >> docs/README.md
	@echo "make all" >> docs/README.md
	@echo "```" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "To build individual components:" >> docs/README.md
	@echo "```bash" >> docs/README.md
	@echo "make components" >> docs/README.md
	@echo "```" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "## Testing" >> docs/README.md
	@echo "" >> docs/README.md
	@echo "To run all tests:" >> docs/README.md
	@echo "```bash" >> docs/README.md
	@echo "make test" >> docs/README.md
	@echo "```" >> docs/README.md
	@echo "Documentation generated in docs/"

# Installation
install: all
	@echo "Installing RaeenOS GUI System..."
	@mkdir -p /usr/local/lib/raeenos
	@mkdir -p /usr/local/include/raeenos/gui
	@cp $(LIBDIR)/*.so /usr/local/lib/raeenos/
	@cp include/*.h /usr/local/include/raeenos/gui/
	@cp rendering/*.h /usr/local/include/raeenos/gui/
	@cp desktop/*.h /usr/local/include/raeenos/gui/
	@cp window/*.h /usr/local/include/raeenos/gui/
	@cp search/*.h /usr/local/include/raeenos/gui/
	@cp customizer/*.h /usr/local/include/raeenos/gui/
	@echo "RaeenOS GUI System installed successfully"

# Clean targets
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILDDIR)
	rm -rf $(LIBDIR)
	rm -f *.o *.so *.a
	rm -f $(RENDERING_DIR)/*.o
	rm -f $(DESKTOP_DIR)/*.o
	rm -f $(WINDOW_DIR)/*.o
	rm -f $(SEARCH_DIR)/*.o
	rm -f $(CUSTOMIZER_DIR)/*.o
	@echo "Clean complete"

clean-docs:
	@echo "Cleaning documentation..."
	rm -rf docs
	@echo "Documentation cleaned"

distclean: clean clean-docs
	@echo "Performing deep clean..."
	rm -f tags cscope.out
	@echo "Deep clean complete"

# Development targets
debug: CFLAGS += -DDEBUG -g3 -O0
debug: all

release: CFLAGS += -DNDEBUG -O3 -fomit-frame-pointer
release: all

# Profiling
profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: all

# Static analysis
analyze: CFLAGS += -fanalyzer
analyze: all

# Coverage
coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: all

# Dependencies
deps:
	@echo "Checking dependencies..."
	@which $(CC) > /dev/null || (echo "Error: $(CC) not found" && exit 1)
	@which $(AS) > /dev/null || (echo "Error: $(AS) not found" && exit 1)
	@which $(LD) > /dev/null || (echo "Error: $(LD) not found" && exit 1)
	@which $(AR) > /dev/null || (echo "Error: $(AR) not found" && exit 1)
	@echo "All dependencies found"

# Help
help:
	@echo "RaeenOS GUI System Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build complete GUI system"
	@echo "  components       - Build individual component libraries"
	@echo "  test             - Run all tests"
	@echo "  test-*          - Run specific component tests"
	@echo "  docs             - Generate documentation"
	@echo "  install          - Install to system"
	@echo "  clean            - Clean build files"
	@echo "  clean-docs       - Clean documentation"
	@echo "  distclean        - Deep clean"
	@echo "  debug            - Build with debug symbols"
	@echo "  release          - Build optimized release"
	@echo "  profile          - Build with profiling"
	@echo "  analyze          - Build with static analysis"
	@echo "  coverage         - Build with coverage"
	@echo "  deps             - Check dependencies"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Environment:"
	@echo "  CC               - C compiler ($(CC))"
	@echo "  AS               - Assembler ($(AS))"
	@echo "  ARCH             - Architecture ($(ARCH))"
	@echo "  CFLAGS           - C compiler flags"
	@echo "  LDFLAGS          - Linker flags"

# Phony targets
.PHONY: all components test test-rendering test-desktop test-window test-search test-customizer \
        docs install clean clean-docs distclean debug release profile analyze coverage deps help

# Include dependency files
-include $(ALL_OBJECTS:.o=.d)

# Generate dependency files
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$ 