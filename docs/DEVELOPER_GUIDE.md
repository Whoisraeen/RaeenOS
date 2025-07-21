# RaeenOS Developer Guide

## Table of Contents

1. [Introduction](#introduction)
2. [Development Environment Setup](#development-environment-setup)
3. [Project Architecture](#project-architecture)
4. [Building RaeenOS](#building-raeenos)
5. [Testing Framework](#testing-framework)
6. [Contributing Guidelines](#contributing-guidelines)
7. [API Documentation](#api-documentation)
8. [Debugging and Troubleshooting](#debugging-and-troubleshooting)
9. [Performance Optimization](#performance-optimization)
10. [Security Considerations](#security-considerations)

## Introduction

RaeenOS is a revolutionary operating system that combines the best features of Windows and macOS while introducing cutting-edge technologies like AI, quantum computing, and blockchain integration. This guide provides comprehensive information for developers contributing to the project.

### Key Features

- **Modern GUI System**: Glass effects, AI-enhanced themes, and responsive design
- **Advanced Hardware Support**: Multi-monitor, spatial audio, biometric authentication
- **AI Integration**: Machine learning, neural networks, and intelligent automation
- **Quantum Computing**: Quantum simulation and algorithm support
- **Blockchain Integration**: Cryptocurrency and smart contract support
- **Enterprise Security**: Military-grade security and compliance features

## Development Environment Setup

### Prerequisites

#### Required Software
- **Operating System**: Linux (Ubuntu 20.04+), Windows 10+, or macOS 10.15+
- **Compiler**: GCC 9+ or Clang 10+
- **Assembler**: NASM 2.14+
- **Build System**: Make 4.0+
- **Version Control**: Git 2.25+

#### Optional Software
- **IDE**: Visual Studio Code, CLion, or Eclipse
- **Debugger**: GDB or LLDB
- **Profiler**: Valgrind, perf, or Instruments
- **Documentation**: Doxygen, Sphinx

### Installation

#### Ubuntu/Debian
```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install -y build-essential nasm gcc-multilib
sudo apt-get install -y qemu-system-x86 qemu-system-i386
sudo apt-get install -y grub-pc-bin xorriso
sudo apt-get install -y doxygen graphviz

# Install development tools
sudo apt-get install -y clang-tidy cppcheck valgrind
sudo apt-get install -y git cmake ninja-build
```

#### Windows
```bash
# Install MSYS2
choco install msys2

# Install toolchain
msys2 -c "pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-nasm mingw-w64-x86_64-make"
msys2 -c "pacman -S mingw-w64-x86_64-qemu mingw-w64-x86_64-grub"
```

#### macOS
```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install gcc nasm make qemu
brew install doxygen graphviz
```

### Repository Setup

```bash
# Clone the repository
git clone https://github.com/whoisraeen/raeenos.git
cd raeenos

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure build
cmake ..
```

## Project Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    RaeenOS Architecture                      │
├─────────────────────────────────────────────────────────────┤
│  Applications Layer                                         │
│  ├── Desktop Environment                                    │
│  ├── File Manager                                          │
│  ├── Web Browser                                           │
│  ├── Email Client                                          │
│  └── Terminal Emulator                                     │
├─────────────────────────────────────────────────────────────┤
│  GUI Framework                                             │
│  ├── RaeenDX Graphics API                                  │
│  ├── Glass Compositor                                      │
│  ├── Window Manager                                        │
│  └── Input System                                          │
├─────────────────────────────────────────────────────────────┤
│  System Services                                           │
│  ├── Authentication System                                 │
│  ├── Power Management                                      │
│  ├── Network Stack                                         │
│  └── File System                                           │
├─────────────────────────────────────────────────────────────┤
│  Kernel Layer                                              │
│  ├── Process Management                                    │
│  ├── Memory Management                                     │
│  ├── Device Drivers                                        │
│  └── System Calls                                          │
├─────────────────────────────────────────────────────────────┤
│  Hardware Abstraction Layer (HAL)                          │
│  ├── CPU Management                                        │
│  ├── Memory Controller                                     │
│  ├── Interrupt Controller                                  │
│  └── Device Enumeration                                    │
├─────────────────────────────────────────────────────────────┤
│  Bootloader                                                │
│  ├── Multiboot Support                                     │
│  ├── Kernel Loading                                        │
│  └── Hardware Initialization                               │
└─────────────────────────────────────────────────────────────┘
```

### Directory Structure

```
raeenos/
├── bootloader/              # Bootloader implementation
│   ├── boot.asm            # Main bootloader
│   ├── stage1/             # Stage 1 bootloader
│   └── stage2/             # Stage 2 bootloader
├── kernel/                  # Kernel source code
│   ├── core/               # Core kernel components
│   │   ├── kernel_main.c   # Kernel entry point
│   │   ├── interrupts.c    # Interrupt handling
│   │   └── syscall.c       # System call interface
│   ├── memory/             # Memory management
│   │   ├── pmm.c          # Physical memory manager
│   │   ├── vmm.c          # Virtual memory manager
│   │   └── heap.c         # Kernel heap
│   ├── process/            # Process management
│   │   ├── process.c      # Process creation/destruction
│   │   ├── scheduler.c    # Process scheduler
│   │   └── thread.c       # Thread management
│   ├── drivers/            # Device drivers
│   │   ├── display/       # Display drivers
│   │   ├── input/         # Input device drivers
│   │   ├── storage/       # Storage drivers
│   │   └── network/       # Network drivers
│   ├── filesystem/         # File system implementations
│   │   ├── vfs.c          # Virtual file system
│   │   ├── ext4.c         # EXT4 file system
│   │   └── fat32.c        # FAT32 file system
│   ├── network/            # Network stack
│   │   ├── tcpip.c        # TCP/IP implementation
│   │   ├── socket.c       # Socket API
│   │   └── wifi.c         # WiFi support
│   ├── security/           # Security components
│   │   ├── auth.c         # Authentication
│   │   ├── encryption.c   # Encryption
│   │   └── firewall.c     # Firewall
│   └── gui/                # GUI framework
│       ├── rendering/     # Rendering engine
│       ├── window/        # Window management
│       └── input/         # Input handling
├── apps/                   # Applications
│   ├── calculator/        # Calculator application
│   ├── file_manager/      # File manager
│   ├── browser/           # Web browser
│   └── terminal/          # Terminal emulator
├── tools/                  # Development tools
│   ├── build/             # Build scripts
│   ├── test/              # Test utilities
│   └── debug/             # Debugging tools
├── tests/                  # Test suites
│   ├── unit/              # Unit tests
│   ├── integration/       # Integration tests
│   └── system/            # System tests
├── docs/                   # Documentation
│   ├── api/               # API documentation
│   ├── guides/            # User guides
│   └── design/            # Design documents
├── build/                  # Build artifacts
├── Makefile               # Main build file
├── CMakeLists.txt         # CMake configuration
└── README.md              # Project overview
```

## Building RaeenOS

### Basic Build

```bash
# Clean previous build
make clean

# Build for x86_64
make TARGET_ARCH=x86_64 BUILD_TYPE=release

# Build with verbose output
make verbose

# Build with specific configuration
make CROSS_COMPILE=aarch64-linux-gnu- TARGET_ARCH=aarch64
```

### Build Types

- **Debug**: Includes debug symbols and assertions
- **Release**: Optimized for performance
- **Profile**: Includes profiling information

### Build Targets

```bash
# Build specific components
make kernel          # Build kernel only
make drivers         # Build drivers only
make gui             # Build GUI only
make apps            # Build applications only

# Build for different architectures
make x86_64          # 64-bit x86
make i386            # 32-bit x86
make aarch64         # 64-bit ARM

# Create bootable ISO
make iso             # Create bootable ISO image
```

### Cross-Compilation

```bash
# Set up cross-compilation toolchain
export CROSS_COMPILE=aarch64-linux-gnu-
export TARGET_ARCH=aarch64

# Build for target architecture
make clean
make CROSS_COMPILE=$CROSS_COMPILE TARGET_ARCH=$TARGET_ARCH
```

## Testing Framework

### Running Tests

```bash
# Run all tests
make test

# Run specific test categories
make test TEST_CATEGORY=unit
make test TEST_CATEGORY=integration
make test TEST_CATEGORY=system

# Run tests with coverage
make test COVERAGE=1

# Run performance tests
make test TEST_CATEGORY=performance
```

### Writing Tests

```c
#include <test_framework.h>

TEST_FUNCTION(test_memory_allocation) {
    // Test setup
    void* ptr = memory_allocate(1024, 8);
    
    // Assertions
    TEST_ASSERT_NOT_NULL(ptr);
    TEST_ASSERT_TRUE(memory_is_valid(ptr));
    
    // Cleanup
    memory_free(ptr);
    
    return TEST_RESULT_PASS;
}

TEST_FUNCTION(test_memory_allocation_failure) {
    // Test large allocation that should fail
    void* ptr = memory_allocate(SIZE_MAX, 8);
    
    // Assertions
    TEST_ASSERT_NULL(ptr);
    
    return TEST_RESULT_PASS;
}
```

### Test Categories

- **Unit Tests**: Test individual functions and modules
- **Integration Tests**: Test interactions between components
- **System Tests**: Test complete system functionality
- **Performance Tests**: Test performance characteristics
- **Security Tests**: Test security features and vulnerabilities

## Contributing Guidelines

### Code Style

Follow the established coding standards:

- Use `snake_case` for functions and variables
- Use `UPPER_SNAKE_CASE` for constants
- Use 4 spaces for indentation
- Maximum line length: 100 characters
- Always use braces for control structures

### Commit Messages

Use conventional commit format:

```
type(scope): description

[optional body]

[optional footer]
```

Examples:
```
feat(kernel): add process priority scheduling
fix(memory): resolve memory leak in heap allocator
docs(api): update memory management documentation
test(drivers): add unit tests for USB driver
```

### Pull Request Process

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**
4. **Write tests for new functionality**
5. **Update documentation**
6. **Run the test suite**
   ```bash
   make test
   ```
7. **Submit a pull request**

### Code Review Checklist

- [ ] Code follows style guidelines
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] No compiler warnings
- [ ] No memory leaks
- [ ] Security considerations addressed
- [ ] Performance impact evaluated

## API Documentation

### Core APIs

#### Memory Management API

```c
/**
 * Allocates a block of memory from the kernel heap.
 *
 * @param size The size of the memory block in bytes
 * @param alignment The alignment requirement (must be power of 2)
 * @return Pointer to allocated memory, or NULL on failure
 */
void* memory_allocate(size_t size, size_t alignment);

/**
 * Frees a previously allocated memory block.
 *
 * @param ptr Pointer to memory block to free
 */
void memory_free(void* ptr);

/**
 * Reallocates a memory block.
 *
 * @param ptr Pointer to existing memory block
 * @param new_size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 */
void* memory_reallocate(void* ptr, size_t new_size);
```

#### Process Management API

```c
/**
 * Creates a new process.
 *
 * @param config Process configuration
 * @param process Pointer to store created process
 * @return Error code indicating success or failure
 */
error_code_t process_create(process_config_t* config, process_t** process);

/**
 * Terminates a process.
 *
 * @param process Process to terminate
 * @param exit_code Process exit code
 * @return Error code indicating success or failure
 */
error_code_t process_terminate(process_t* process, int exit_code);

/**
 * Suspends a process.
 *
 * @param process Process to suspend
 * @return Error code indicating success or failure
 */
error_code_t process_suspend(process_t* process);
```

#### File System API

```c
/**
 * Opens a file.
 *
 * @param path File path
 * @param mode Open mode
 * @return File handle, or NULL on failure
 */
file_handle_t file_open(const char* path, file_mode_t mode);

/**
 * Reads data from a file.
 *
 * @param file File handle
 * @param buffer Buffer to store read data
 * @param size Number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
ssize_t file_read(file_handle_t file, void* buffer, size_t size);

/**
 * Writes data to a file.
 *
 * @param file File handle
 * @param buffer Data to write
 * @param size Number of bytes to write
 * @return Number of bytes written, or -1 on error
 */
ssize_t file_write(file_handle_t file, const void* buffer, size_t size);
```

### GUI APIs

#### Window Management API

```c
/**
 * Creates a new window.
 *
 * @param config Window configuration
 * @return Window handle, or NULL on failure
 */
window_handle_t window_create(window_config_t* config);

/**
 * Shows a window.
 *
 * @param window Window handle
 */
void window_show(window_handle_t window);

/**
 * Hides a window.
 *
 * @param window Window handle
 */
void window_hide(window_handle_t window);

/**
 * Sets window title.
 *
 * @param window Window handle
 * @param title New title
 */
void window_set_title(window_handle_t window, const char* title);
```

## Debugging and Troubleshooting

### Debugging Tools

#### GDB Debugging

```bash
# Build with debug symbols
make BUILD_TYPE=debug

# Start GDB
gdb build/bin/raeenos_kernel.bin

# Common GDB commands
(gdb) break kernel_main
(gdb) run
(gdb) next
(gdb) print variable_name
(gdb) bt
(gdb) info registers
```

#### Valgrind Memory Checking

```bash
# Run with Valgrind
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./test_program
```

#### QEMU Debugging

```bash
# Start QEMU with GDB server
qemu-system-x86_64 -cdrom build/bin/raeenos.iso -s -S

# Connect GDB to QEMU
gdb -ex "target remote localhost:1234" build/bin/raeenos_kernel.bin
```

### Common Issues

#### Build Issues

**Problem**: Compilation errors
```bash
# Solution: Check dependencies
sudo apt-get install build-essential nasm gcc-multilib

# Solution: Clean and rebuild
make clean
make
```

**Problem**: Cross-compilation issues
```bash
# Solution: Install cross-compilation toolchain
sudo apt-get install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

# Solution: Set environment variables
export CROSS_COMPILE=aarch64-linux-gnu-
export TARGET_ARCH=aarch64
```

#### Runtime Issues

**Problem**: Kernel panic
```bash
# Solution: Enable debug output
make BUILD_TYPE=debug
# Check kernel logs for error messages
```

**Problem**: Memory corruption
```bash
# Solution: Use Valgrind for detection
valgrind --tool=memcheck --leak-check=full ./test_program
```

## Performance Optimization

### Profiling

#### CPU Profiling

```bash
# Build with profiling
make BUILD_TYPE=profile

# Run with perf
perf record ./test_program
perf report

# Run with gprof
gprof ./test_program gmon.out > analysis.txt
```

#### Memory Profiling

```bash
# Use Valgrind for memory profiling
valgrind --tool=massif ./test_program
ms_print massif.out.* > memory_analysis.txt
```

### Optimization Techniques

#### Algorithm Optimization

```c
// Use efficient data structures
typedef struct {
    uint32_t* data;
    size_t size;
    size_t capacity;
} dynamic_array_t;

// Use caching for frequently accessed data
static cache_t* g_cache = NULL;

// Use inline functions for small, frequently called functions
static inline uint32_t fast_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}
```

#### Memory Optimization

```c
// Use memory pools for small allocations
typedef struct {
    void* pool;
    size_t block_size;
    size_t block_count;
    void* free_list;
} memory_pool_t;

// Use object pooling
typedef struct {
    object_t* objects;
    size_t count;
    size_t capacity;
    void* free_indices;
} object_pool_t;
```

## Security Considerations

### Input Validation

```c
// Always validate input parameters
bool process_user_input(const char* input, size_t input_size) {
    // Check for null pointer
    if (!input) {
        return false;
    }
    
    // Check for reasonable size
    if (input_size == 0 || input_size > MAX_INPUT_SIZE) {
        return false;
    }
    
    // Check for null termination
    if (input[input_size - 1] != '\0') {
        return false;
    }
    
    // Validate content
    for (size_t i = 0; i < input_size - 1; i++) {
        if (!isprint(input[i])) {
            return false;
        }
    }
    
    return true;
}
```

### Buffer Overflow Prevention

```c
// Use safe string functions
char* safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return NULL;
    }
    
    size_t src_len = strlen(src);
    if (src_len >= dest_size) {
        return NULL;
    }
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    return dest;
}
```

### Privilege Separation

```c
// Check permissions before operations
bool perform_privileged_operation(process_t* process, operation_t operation) {
    // Check if process has required capabilities
    if (!process_has_capability(process, operation.required_capability)) {
        return false;
    }
    
    // Check if operation is allowed for process
    if (!operation_is_allowed(process, operation)) {
        return false;
    }
    
    // Perform operation with minimal privileges
    return execute_operation_with_privileges(process, operation, MINIMAL_PRIVILEGES);
}
```

## Conclusion

This developer guide provides comprehensive information for contributing to RaeenOS. Follow the established guidelines and best practices to ensure code quality and maintainability.

For additional information, refer to:
- [API Documentation](api/)
- [Design Documents](design/)
- [User Guide](../USER_GUIDE.md)
- [Contributing Guidelines](../CONTRIBUTING.md)

Remember: **Quality over quantity. Write code that is readable, maintainable, and secure.** 