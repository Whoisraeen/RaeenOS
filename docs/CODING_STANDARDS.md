# RaeenOS Coding Standards and Style Guidelines

## Overview

This document defines the coding standards and style guidelines for the RaeenOS operating system project. These standards ensure code quality, consistency, maintainability, and readability across the entire codebase.

## Table of Contents

1. [General Principles](#general-principles)
2. [File Organization](#file-organization)
3. [Naming Conventions](#naming-conventions)
4. [Code Formatting](#code-formatting)
5. [Documentation](#documentation)
6. [Error Handling](#error-handling)
7. [Memory Management](#memory-management)
8. [Security Guidelines](#security-guidelines)
9. [Performance Guidelines](#performance-guidelines)
10. [Testing Standards](#testing-standards)

## General Principles

### Code Quality
- **Readability**: Code should be self-documenting and easy to understand
- **Maintainability**: Code should be modular and well-structured
- **Reliability**: Code should be robust and handle edge cases
- **Performance**: Code should be efficient without sacrificing readability
- **Security**: Code should follow security best practices

### Design Principles
- **Single Responsibility**: Each function/class should have one clear purpose
- **Open/Closed**: Open for extension, closed for modification
- **Dependency Inversion**: Depend on abstractions, not concretions
- **Interface Segregation**: Keep interfaces small and focused
- **DRY (Don't Repeat Yourself)**: Avoid code duplication

## File Organization

### Directory Structure
```
raeenos/
├── kernel/                 # Kernel source code
│   ├── core/              # Core kernel components
│   ├── drivers/           # Device drivers
│   ├── filesystem/        # File system implementations
│   ├── memory/            # Memory management
│   ├── network/           # Network stack
│   ├── security/          # Security components
│   └── gui/               # GUI framework
├── bootloader/            # Bootloader code
├── tools/                 # Development tools
├── tests/                 # Test suites
├── docs/                  # Documentation
└── build/                 # Build artifacts
```

### File Naming
- **Header files**: Use `.h` extension, lowercase with underscores
- **Source files**: Use `.c` or `.cpp` extension, lowercase with underscores
- **Assembly files**: Use `.asm` or `.S` extension
- **Test files**: Prefix with `test_`, use `.c` or `.cpp` extension

**Examples:**
```
memory_manager.h
memory_manager.c
kernel_main.c
boot_loader.asm
test_memory_manager.c
```

## Naming Conventions

### Functions
- Use `snake_case` for function names
- Use descriptive, verb-based names
- Prefix with module name for clarity

**Examples:**
```c
// Good
memory_allocate_pages(size_t count);
kernel_initialize_system(void);
driver_register_device(device_t* device);

// Bad
alloc(size_t count);
init(void);
reg(device_t* device);
```

### Variables
- Use `snake_case` for variable names
- Use descriptive names that indicate purpose
- Avoid single-letter names except for loop counters

**Examples:**
```c
// Good
uint32_t page_count;
char* device_name;
bool is_initialized;

// Bad
uint32_t pc;
char* dn;
bool init;
```

### Constants
- Use `UPPER_SNAKE_CASE` for constants and macros
- Use descriptive names

**Examples:**
```c
#define MAX_PAGE_COUNT 1024
#define KERNEL_STACK_SIZE 8192
#define DEFAULT_TIMEOUT_MS 5000
```

### Types
- Use `snake_case` with `_t` suffix for typedefs
- Use descriptive names

**Examples:**
```c
typedef struct {
    uint32_t id;
    char name[64];
} process_t;

typedef enum {
    STATE_INITIALIZING,
    STATE_RUNNING,
    STATE_SUSPENDED
} process_state_t;
```

### Structs and Unions
- Use `snake_case` for struct/union names
- Use descriptive names

**Examples:**
```c
struct memory_block {
    uintptr_t address;
    size_t size;
    bool is_allocated;
};

union register_value {
    uint32_t raw;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};
```

## Code Formatting

### Indentation
- Use 4 spaces for indentation (no tabs)
- Align code blocks consistently

### Line Length
- Maximum line length: 100 characters
- Break long lines at logical points

### Spacing
- One space after keywords
- No space before semicolons
- One space around operators
- No trailing whitespace

**Examples:**
```c
// Good
if (condition) {
    do_something();
}

for (int i = 0; i < count; i++) {
    process_item(items[i]);
}

// Bad
if(condition){
    do_something();
}

for(int i=0;i<count;i++){
    process_item(items[i]);
}
```

### Braces
- Use K&R style (opening brace on same line)
- Always use braces for control structures

**Examples:**
```c
// Good
if (condition) {
    do_something();
} else {
    do_something_else();
}

// Bad
if (condition)
    do_something();
else
    do_something_else();
```

### Comments
- Use `/* */` for block comments
- Use `//` for line comments
- Write comments that explain "why", not "what"

**Examples:**
```c
/* Initialize the memory manager with the specified heap size.
 * This must be called before any memory allocation operations.
 */
bool memory_manager_init(size_t heap_size) {
    // Validate heap size is reasonable
    if (heap_size < MIN_HEAP_SIZE || heap_size > MAX_HEAP_SIZE) {
        return false;
    }
    
    // ... implementation
}
```

## Documentation

### Function Documentation
Every public function must have a header comment describing:
- Purpose and functionality
- Parameters and their types
- Return value and meaning
- Error conditions
- Usage examples (if complex)

**Example:**
```c
/**
 * Allocates a block of memory from the kernel heap.
 *
 * @param size The size of the memory block to allocate in bytes
 * @param alignment The alignment requirement (must be power of 2)
 * @return Pointer to allocated memory block, or NULL on failure
 * @note This function is not thread-safe. Use memory_allocate_safe() for
 *       multi-threaded environments.
 * @see memory_allocate_safe(), memory_free()
 */
void* memory_allocate(size_t size, size_t alignment);
```

### File Headers
Every source file must have a header comment:

```c
/**
 * @file memory_manager.c
 * @brief Kernel memory management implementation
 * @author Your Name <your.email@example.com>
 * @date 2024-01-01
 * @version 1.0
 * @copyright Copyright (c) 2024 RaeenOS Project
 *
 * This file implements the core memory management functionality for the
 * RaeenOS kernel, including page allocation, heap management, and memory
 * protection.
 */
```

### Module Documentation
Each module should have a README file describing:
- Purpose and functionality
- Dependencies
- API overview
- Usage examples
- Configuration options

## Error Handling

### Error Codes
- Use consistent error code enums
- Provide meaningful error messages
- Include error context when possible

**Example:**
```c
typedef enum {
    SUCCESS = 0,
    ERROR_INVALID_PARAMETER = -1,
    ERROR_OUT_OF_MEMORY = -2,
    ERROR_DEVICE_NOT_FOUND = -3,
    ERROR_PERMISSION_DENIED = -4,
    ERROR_TIMEOUT = -5,
    ERROR_NOT_SUPPORTED = -6,
} error_code_t;

typedef struct {
    error_code_t code;
    char message[256];
    char function[64];
    char file[64];
    uint32_t line;
} error_info_t;
```

### Error Handling Patterns
- Always check return values
- Use consistent error handling patterns
- Provide cleanup in error paths

**Example:**
```c
error_code_t process_create(process_config_t* config, process_t** process) {
    error_code_t result;
    
    // Validate parameters
    if (!config || !process) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Allocate process structure
    process_t* new_process = memory_allocate(sizeof(process_t), 8);
    if (!new_process) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize process
    result = process_initialize(new_process, config);
    if (result != SUCCESS) {
        memory_free(new_process);
        return result;
    }
    
    *process = new_process;
    return SUCCESS;
}
```

## Memory Management

### Allocation Patterns
- Always check allocation failures
- Use appropriate allocation functions
- Free memory in reverse order of allocation
- Use RAII patterns where possible

**Example:**
```c
bool process_load_executable(const char* path, process_t* process) {
    file_handle_t file = NULL;
    void* buffer = NULL;
    bool success = false;
    
    // Open file
    file = file_open(path, FILE_MODE_READ);
    if (!file) {
        goto cleanup;
    }
    
    // Allocate buffer
    buffer = memory_allocate(file_size(file), 8);
    if (!buffer) {
        goto cleanup;
    }
    
    // Read file
    if (!file_read(file, buffer, file_size(file))) {
        goto cleanup;
    }
    
    // Process executable
    if (!process_parse_executable(process, buffer)) {
        goto cleanup;
    }
    
    success = true;
    
cleanup:
    if (buffer) {
        memory_free(buffer);
    }
    if (file) {
        file_close(file);
    }
    return success;
}
```

### Memory Safety
- Always initialize allocated memory
- Use bounds checking
- Avoid buffer overflows
- Use safe string functions

## Security Guidelines

### Input Validation
- Validate all input parameters
- Check buffer sizes
- Use safe string functions
- Sanitize user input

**Example:**
```c
bool user_input_process(const char* input, size_t input_size) {
    // Validate input parameters
    if (!input || input_size == 0 || input_size > MAX_INPUT_SIZE) {
        return false;
    }
    
    // Check for null termination
    if (input[input_size - 1] != '\0') {
        return false;
    }
    
    // Validate input content
    for (size_t i = 0; i < input_size - 1; i++) {
        if (!isprint(input[i])) {
            return false;
        }
    }
    
    // Process validated input
    return process_validated_input(input);
}
```

### Privilege Separation
- Run with minimal privileges
- Validate permissions before operations
- Use capability-based security
- Implement proper access controls

### Secure Coding Practices
- Avoid buffer overflows
- Use secure random number generation
- Implement proper authentication
- Use encryption for sensitive data
- Follow principle of least privilege

## Performance Guidelines

### Optimization Principles
- Profile before optimizing
- Optimize algorithms first
- Use appropriate data structures
- Minimize memory allocations
- Cache frequently accessed data

### Performance Patterns
```c
// Use static allocation for small, frequent allocations
static char buffer[1024];

// Use inline functions for small, frequently called functions
static inline uint32_t fast_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Use const where possible
const char* get_error_message(error_code_t code) {
    static const char* messages[] = {
        "Success",
        "Invalid parameter",
        "Out of memory",
        // ...
    };
    return messages[code];
}
```

## Testing Standards

### Test Requirements
- Unit tests for all public functions
- Integration tests for modules
- System tests for complete features
- Performance tests for critical paths
- Security tests for sensitive operations

### Test Naming
- Test names should describe the scenario being tested
- Use descriptive test names that explain the expected behavior

**Example:**
```c
TEST_FUNCTION(test_memory_allocate_valid_size_returns_pointer) {
    // Test implementation
}

TEST_FUNCTION(test_memory_allocate_zero_size_returns_null) {
    // Test implementation
}

TEST_FUNCTION(test_memory_allocate_large_size_returns_null) {
    // Test implementation
}
```

### Test Coverage
- Aim for 90%+ code coverage
- Test error conditions
- Test edge cases
- Test boundary conditions

## Code Review Checklist

### Before Submitting Code
- [ ] Code follows naming conventions
- [ ] Code is properly formatted
- [ ] All functions are documented
- [ ] Error handling is implemented
- [ ] Memory management is correct
- [ ] Security considerations are addressed
- [ ] Tests are written and passing
- [ ] No compiler warnings
- [ ] No memory leaks
- [ ] Performance is acceptable

### Review Process
1. Self-review using the checklist above
2. Peer review by team member
3. Automated testing and analysis
4. Integration testing
5. Final approval by maintainer

## Tools and Automation

### Required Tools
- **Compiler**: GCC with strict warnings enabled
- **Static Analysis**: Clang Static Analyzer, cppcheck
- **Code Formatting**: clang-format
- **Documentation**: Doxygen
- **Testing**: Custom test framework
- **Coverage**: gcov, lcov

### Build Configuration
```makefile
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -std=c11 -O2 -g
CFLAGS += -fstack-protector-strong
CFLAGS += -D_FORTIFY_SOURCE=2
```

### Continuous Integration
- Automated builds on all supported platforms
- Static analysis on every commit
- Test suite execution
- Code coverage reporting
- Performance regression testing

## Conclusion

These coding standards ensure that RaeenOS maintains high code quality, consistency, and maintainability. All contributors must follow these guidelines to ensure the long-term success of the project.

Remember: **Code is read much more often than it is written. Write for the reader, not just for the compiler.** 