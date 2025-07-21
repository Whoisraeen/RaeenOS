# Executable Loader and Syscalls Implementation

## Overview

This document describes the implementation of a flat binary executable loader and the first real syscalls for RaeenOS. The system allows loading and running user programs from a RAM disk with proper syscall support.

## Components Implemented

### 1. Flat Binary Executable Loader

**Files:**
- `kernel/process/executable_loader.h`
- `kernel/process/executable_loader.c`

**Features:**
- Flat binary format with custom header
- RAM disk for storing executables
- User address space setup
- Code, data, and BSS section mapping
- User stack allocation
- Process entry point configuration

**Flat Binary Format:**
```c
typedef struct {
    uint32_t magic;           // Magic number: 0xRAEENOS
    uint32_t entry_point;     // Entry point offset
    uint32_t code_size;       // Size of code section
    uint32_t data_size;       // Size of data section
    uint32_t bss_size;        // Size of BSS section
    uint32_t stack_size;      // Size of initial stack
    uint32_t flags;           // Load flags
} flat_binary_header_t;
```

### 2. Syscall System

**Files:**
- `kernel/process/syscall.h`
- `kernel/process/syscall.c`
- `kernel/hal/x64/syscall_entry.S`

**Implemented Syscalls:**
1. **sys_exit** (syscall #1) - Terminate process with exit code
2. **sys_write** (syscall #2) - Write data to file descriptor (stdout)
3. **sys_getpid** (syscall #3) - Get current process ID

**Features:**
- Syscall handler registration system
- User space to kernel space data copying
- Proper privilege level switching
- Error handling and validation

### 3. GDT and IDT Setup

**Files:**
- `kernel/hal/x64/hal_x64.c` (updated)
- `kernel/hal/x64/hal_x64.h` (updated)

**Features:**
- Complete GDT with kernel/user code/data segments
- TSS (Task State Segment) setup
- IDT with exception, IRQ, and syscall handlers
- MSR configuration for syscall/sysret instructions

### 4. Test Program

**Files:**
- `tools/test_program.c`
- `tools/Makefile`
- `tools/build_test_program.sh`

**Features:**
- Simple C program demonstrating syscalls
- Writes "Hello from user space!" to console
- Displays process ID
- Exits cleanly
- Compiles to flat binary format

## Architecture

### Memory Layout

```
User Space (0x400000 - 0x7fffffffffff):
├── Code Section (0x400000)
├── Data Section (after code)
├── BSS Section (after data)
└── Stack (0x7ffff0000000 - grows downward)

Kernel Space (0xffff800000000000 - 0xffffffffffffffff):
├── Kernel Code
├── Kernel Data
└── Kernel Stack
```

### Syscall Flow

1. User program executes `syscall` instruction
2. CPU switches to kernel mode (ring 0)
3. Control transfers to `syscall_entry` assembly handler
4. Handler saves user registers and switches to kernel stack
5. Calls `syscall_handler` with syscall number and arguments
6. Appropriate syscall function is executed
7. Result is returned to user space
8. `sysretq` instruction returns to user mode

### Process Creation Flow

1. `process_create()` creates new process structure
2. `load_flat_binary()` reads executable from RAM disk
3. `setup_user_address_space()` maps memory regions
4. `vm_create_address_space()` sets up page tables
5. Process is scheduled and begins execution

## Usage

### Building the Test Program

```bash
cd tools
make test_program
```

This creates `test_program.bin` in flat binary format.

### Running the System

The kernel automatically:
1. Initializes the RAM disk with test data
2. Creates a user process
3. Loads the test program
4. Starts the scheduler
5. The test program runs and demonstrates syscalls

### Expected Output

When the system runs, you should see:
```
Hello from user space!
My process ID is: 1
```

## Technical Details

### Syscall Numbers
- `SYS_EXIT = 1`
- `SYS_WRITE = 2`
- `SYS_GETPID = 3`

### File Descriptors
- `STDIN_FILENO = 0`
- `STDOUT_FILENO = 1`
- `STDERR_FILENO = 2`

### Memory Protection
- Code sections: `VM_PROT_READ | VM_PROT_EXEC`
- Data sections: `VM_PROT_READ | VM_PROT_WRITE`
- Stack: `VM_PROT_READ | VM_PROT_WRITE`

### GDT Segments
- Kernel Code: 0x08
- Kernel Data: 0x10
- User Code: 0x18
- User Data: 0x20
- TSS: 0x28

## Future Enhancements

1. **ELF Support**: Add ELF executable format support
2. **Dynamic Linking**: Implement dynamic library loading
3. **More Syscalls**: Add file I/O, process management, etc.
4. **Signal Handling**: Implement signal delivery and handling
5. **Memory Protection**: Add copy-on-write and demand paging
6. **Multi-threading**: Support for user-level threads

## Dependencies

- GCC with x86-64 support
- objcopy for binary manipulation
- Proper GDT/IDT setup in HAL
- Virtual memory management system
- Process management system

## Status

✅ **Completed:**
- Flat binary loader
- Basic syscall system
- GDT/IDT setup
- Test program
- RAM disk implementation

🔄 **In Progress:**
- Integration with existing kernel
- Testing and debugging

📋 **Next Steps:**
- Add more syscalls (open, read, close, etc.)
- Implement proper file system integration
- Add signal handling
- Enhance memory management 