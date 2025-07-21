#include "kernel.h"
#include "process/process.h"
#include "filesystem/include/vfs.h"
#include "memory/memory.h"
#include "hal/hal.h"
#include <string.h>

// System call numbers
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_EXIT    60
#define SYS_FORK    57
#define SYS_EXECVE  59
#define SYS_GETPID  39
#define SYS_GETPPID 110

// File descriptor limits
#define MAX_FILE_DESCRIPTORS 1024
#define STDIN_FD  0
#define STDOUT_FD 1
#define STDERR_FD 2

// File descriptor flags
#define FD_CLOEXEC 0x00000001

// File descriptor entry
typedef struct {
    u32 inode_number;
    u32 flags;
    u64 offset;
    bool active;
} file_descriptor_t;

// Process file descriptor table
typedef struct {
    file_descriptor_t fds[MAX_FILE_DESCRIPTORS];
    u32 next_fd;
} process_fd_table_t;

// Global system call handlers
static syscall_handler_t syscall_handlers[256] = {0};
static bool syscalls_initialized = false;

// Forward declarations
static error_t syscall_validate_user_pointer(const void* ptr, size_t size);
static error_t syscall_copy_from_user(void* kernel_ptr, const void* user_ptr, size_t size);
static error_t syscall_copy_to_user(void* user_ptr, const void* kernel_ptr, size_t size);
static error_t syscall_validate_string(const char* user_str, size_t max_len);
static error_t syscall_copy_string_from_user(char* kernel_str, const char* user_str, size_t max_len);
static file_descriptor_t* syscall_get_fd(u32 fd);
static u32 syscall_alloc_fd(void);
static void syscall_free_fd(u32 fd);

// Initialize system calls
error_t syscalls_init(void) {
    KINFO("Initializing system calls");
    
    // Clear syscall handlers
    memset(syscall_handlers, 0, sizeof(syscall_handlers));
    
    // Register system call handlers
    syscall_handlers[SYS_READ] = sys_read;
    syscall_handlers[SYS_WRITE] = sys_write;
    syscall_handlers[SYS_OPEN] = sys_open;
    syscall_handlers[SYS_CLOSE] = sys_close;
    syscall_handlers[SYS_EXIT] = sys_exit;
    syscall_handlers[SYS_FORK] = sys_fork;
    syscall_handlers[SYS_EXECVE] = sys_execve;
    syscall_handlers[SYS_GETPID] = sys_getpid;
    syscall_handlers[SYS_GETPPID] = sys_getppid;
    
    syscalls_initialized = true;
    
    KINFO("System calls initialized");
    return SUCCESS;
}

// System call dispatcher
u64 syscall_dispatch(u64 syscall_number, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    if (!syscalls_initialized) {
        return -E_NOSYS;
    }
    
    if (syscall_number >= 256) {
        return -E_INVAL;
    }
    
    syscall_handler_t handler = syscall_handlers[syscall_number];
    if (!handler) {
        return -E_NOSYS;
    }
    
    // Call the system call handler
    return handler(arg1, arg2, arg3, arg4, arg5);
}

// System call: read
u64 sys_read(u64 fd, u64 buf, u64 count, u64 arg4, u64 arg5) {
    // Validate file descriptor
    if (fd >= MAX_FILE_DESCRIPTORS) {
        return -E_INVAL;
    }
    
    file_descriptor_t* file_desc = syscall_get_fd((u32)fd);
    if (!file_desc || !file_desc->active) {
        return -E_BADF;
    }
    
    // Validate buffer
    if (count > 0) {
        error_t result = syscall_validate_user_pointer((void*)buf, count);
        if (result != SUCCESS) {
            return -result;
        }
    }
    
    // Allocate kernel buffer
    void* kernel_buf = NULL;
    if (count > 0) {
        kernel_buf = memory_alloc(count);
        if (!kernel_buf) {
            return -E_NOMEM;
        }
    }
    
    // Read from file
    ssize_t bytes_read = vfs_read_file(file_desc->inode_number, kernel_buf, count, file_desc->offset);
    if (bytes_read < 0) {
        if (kernel_buf) {
            memory_free(kernel_buf);
        }
        return bytes_read;
    }
    
    // Update file offset
    file_desc->offset += bytes_read;
    
    // Copy data to user buffer
    if (bytes_read > 0 && kernel_buf) {
        error_t result = syscall_copy_to_user((void*)buf, kernel_buf, bytes_read);
        if (result != SUCCESS) {
            memory_free(kernel_buf);
            return -result;
        }
    }
    
    if (kernel_buf) {
        memory_free(kernel_buf);
    }
    
    return bytes_read;
}

// System call: write
u64 sys_write(u64 fd, u64 buf, u64 count, u64 arg4, u64 arg5) {
    // Validate file descriptor
    if (fd >= MAX_FILE_DESCRIPTORS) {
        return -E_INVAL;
    }
    
    file_descriptor_t* file_desc = syscall_get_fd((u32)fd);
    if (!file_desc || !file_desc->active) {
        return -E_BADF;
    }
    
    // Handle special file descriptors
    if (fd == STDOUT_FD || fd == STDERR_FD) {
        // Write to console
        if (count > 0) {
            error_t result = syscall_validate_user_pointer((void*)buf, count);
            if (result != SUCCESS) {
                return -result;
            }
            
            // Allocate kernel buffer
            void* kernel_buf = memory_alloc(count);
            if (!kernel_buf) {
                return -E_NOMEM;
            }
            
            // Copy from user buffer
            result = syscall_copy_from_user(kernel_buf, (void*)buf, count);
            if (result != SUCCESS) {
                memory_free(kernel_buf);
                return -result;
            }
            
            // Write to console
            hal_console_write((char*)kernel_buf, count);
            
            memory_free(kernel_buf);
            return count;
        }
        return 0;
    }
    
    // Validate buffer
    if (count > 0) {
        error_t result = syscall_validate_user_pointer((void*)buf, count);
        if (result != SUCCESS) {
            return -result;
        }
    }
    
    // Allocate kernel buffer
    void* kernel_buf = NULL;
    if (count > 0) {
        kernel_buf = memory_alloc(count);
        if (!kernel_buf) {
            return -E_NOMEM;
        }
        
        // Copy from user buffer
        error_t result = syscall_copy_from_user(kernel_buf, (void*)buf, count);
        if (result != SUCCESS) {
            memory_free(kernel_buf);
            return -result;
        }
    }
    
    // Write to file
    ssize_t bytes_written = vfs_write_file(file_desc->inode_number, kernel_buf, count, file_desc->offset);
    if (bytes_written < 0) {
        if (kernel_buf) {
            memory_free(kernel_buf);
        }
        return bytes_written;
    }
    
    // Update file offset
    file_desc->offset += bytes_written;
    
    if (kernel_buf) {
        memory_free(kernel_buf);
    }
    
    return bytes_written;
}

// System call: open
u64 sys_open(u64 pathname, u64 flags, u64 mode, u64 arg4, u64 arg5) {
    // Validate pathname
    error_t result = syscall_validate_string((char*)pathname, 1024);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Copy pathname from user space
    char kernel_path[1024];
    result = syscall_copy_string_from_user(kernel_path, (char*)pathname, sizeof(kernel_path));
    if (result != SUCCESS) {
        return -result;
    }
    
    // Open file
    u32 inode_number;
    result = vfs_open_file(kernel_path, (u32)flags, (u32)mode, &inode_number);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Allocate file descriptor
    u32 fd = syscall_alloc_fd();
    if (fd == (u32)-1) {
        return -E_NOMEM;
    }
    
    // Set up file descriptor
    file_descriptor_t* file_desc = syscall_get_fd(fd);
    file_desc->inode_number = inode_number;
    file_desc->flags = (u32)flags;
    file_desc->offset = 0;
    file_desc->active = true;
    
    return fd;
}

// System call: close
u64 sys_close(u64 fd, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    // Validate file descriptor
    if (fd >= MAX_FILE_DESCRIPTORS) {
        return -E_INVAL;
    }
    
    file_descriptor_t* file_desc = syscall_get_fd((u32)fd);
    if (!file_desc || !file_desc->active) {
        return -E_BADF;
    }
    
    // Close file
    error_t result = vfs_close_file(file_desc->inode_number);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Free file descriptor
    syscall_free_fd((u32)fd);
    
    return 0;
}

// System call: exit
u64 sys_exit(u64 status, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    process_t* current = process_get_current();
    if (current) {
        current->exit_code = (u32)status;
        process_terminate(current->pid);
    }
    
    // Schedule another process
    scheduler_yield();
    
    // Should not reach here
    return 0;
}

// System call: fork
u64 sys_fork(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    process_t* current = process_get_current();
    if (!current) {
        return -E_INVAL;
    }
    
    u32 child_pid = process_fork(current->pid);
    if (child_pid == (u32)-1) {
        return -E_NOMEM;
    }
    
    return child_pid;
}

// System call: execve
u64 sys_execve(u64 filename, u64 argv, u64 envp, u64 arg4, u64 arg5) {
    // Validate filename
    error_t result = syscall_validate_string((char*)filename, 1024);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Copy filename from user space
    char kernel_filename[1024];
    result = syscall_copy_string_from_user(kernel_filename, (char*)filename, sizeof(kernel_filename));
    if (result != SUCCESS) {
        return -result;
    }
    
    // Execute program
    result = process_execve(kernel_filename, (char**)argv, (char**)envp);
    if (result != SUCCESS) {
        return -result;
    }
    
    // Should not reach here
    return 0;
}

// System call: getpid
u64 sys_getpid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    process_t* current = process_get_current();
    if (!current) {
        return -E_INVAL;
    }
    
    return current->pid;
}

// System call: getppid
u64 sys_getppid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
    process_t* current = process_get_current();
    if (!current) {
        return -E_INVAL;
    }
    
    return current->parent_pid;
}

// Validate user pointer
static error_t syscall_validate_user_pointer(const void* ptr, size_t size) {
    if (!ptr) {
        return E_INVAL;
    }
    
    u64 addr = (u64)ptr;
    
    // Check if pointer is in user space
    if (addr < 0x400000 || addr >= 0x800000000000) {
        return E_FAULT;
    }
    
    // Check if range is valid
    if (addr + size > 0x800000000000) {
        return E_FAULT;
    }
    
    return SUCCESS;
}

// Copy data from user space to kernel space
static error_t syscall_copy_from_user(void* kernel_ptr, const void* user_ptr, size_t size) {
    if (!kernel_ptr || !user_ptr) {
        return E_INVAL;
    }
    
    // Validate user pointer
    error_t result = syscall_validate_user_pointer(user_ptr, size);
    if (result != SUCCESS) {
        return result;
    }
    
    // Copy data (in a real implementation, this would handle page faults)
    memcpy(kernel_ptr, user_ptr, size);
    
    return SUCCESS;
}

// Copy data from kernel space to user space
static error_t syscall_copy_to_user(void* user_ptr, const void* kernel_ptr, size_t size) {
    if (!user_ptr || !kernel_ptr) {
        return E_INVAL;
    }
    
    // Validate user pointer
    error_t result = syscall_validate_user_pointer(user_ptr, size);
    if (result != SUCCESS) {
        return result;
    }
    
    // Copy data (in a real implementation, this would handle page faults)
    memcpy(user_ptr, kernel_ptr, size);
    
    return SUCCESS;
}

// Validate user string
static error_t syscall_validate_string(const char* user_str, size_t max_len) {
    if (!user_str) {
        return E_INVAL;
    }
    
    // Validate pointer
    error_t result = syscall_validate_user_pointer(user_str, max_len);
    if (result != SUCCESS) {
        return result;
    }
    
    // Check for null termination within max_len
    for (size_t i = 0; i < max_len; i++) {
        char c;
        if (syscall_copy_from_user(&c, user_str + i, 1) != SUCCESS) {
            return E_FAULT;
        }
        if (c == '\0') {
            return SUCCESS;
        }
    }
    
    return E_INVAL; // String too long
}

// Copy string from user space to kernel space
static error_t syscall_copy_string_from_user(char* kernel_str, const char* user_str, size_t max_len) {
    if (!kernel_str || !user_str) {
        return E_INVAL;
    }
    
    // Validate user string
    error_t result = syscall_validate_string(user_str, max_len);
    if (result != SUCCESS) {
        return result;
    }
    
    // Copy string
    for (size_t i = 0; i < max_len; i++) {
        char c;
        if (syscall_copy_from_user(&c, user_str + i, 1) != SUCCESS) {
            return E_FAULT;
        }
        kernel_str[i] = c;
        if (c == '\0') {
            break;
        }
    }
    
    return SUCCESS;
}

// Get file descriptor
static file_descriptor_t* syscall_get_fd(u32 fd) {
    process_t* current = process_get_current();
    if (!current) {
        return NULL;
    }
    
    // For now, use a simple global file descriptor table
    // In a real implementation, each process would have its own table
    static process_fd_table_t fd_table = {0};
    
    if (fd >= MAX_FILE_DESCRIPTORS) {
        return NULL;
    }
    
    return &fd_table.fds[fd];
}

// Allocate file descriptor
static u32 syscall_alloc_fd(void) {
    process_t* current = process_get_current();
    if (!current) {
        return (u32)-1;
    }
    
    // For now, use a simple global file descriptor table
    static process_fd_table_t fd_table = {0};
    
    // Find next available file descriptor
    for (u32 i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (!fd_table.fds[i].active) {
            fd_table.fds[i].active = true;
            return i;
        }
    }
    
    return (u32)-1;
}

// Free file descriptor
static void syscall_free_fd(u32 fd) {
    if (fd >= MAX_FILE_DESCRIPTORS) {
        return;
    }
    
    // For now, use a simple global file descriptor table
    static process_fd_table_t fd_table = {0};
    
    fd_table.fds[fd].active = false;
}

// Check if syscalls are initialized
bool syscalls_is_initialized(void) {
    return syscalls_initialized;
}

// Dump syscall information
void syscalls_dump_info(void) {
    KINFO("=== System Calls Information ===");
    KINFO("Initialized: %s", syscalls_initialized ? "Yes" : "No");
    
    if (syscalls_initialized) {
        KINFO("Registered handlers:");
        for (u32 i = 0; i < 256; i++) {
            if (syscall_handlers[i]) {
                KINFO("  syscall %u: handler registered", i);
            }
        }
    }
} 