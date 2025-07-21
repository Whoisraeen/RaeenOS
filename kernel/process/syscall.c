#include "syscall.h"
#include "process.h"
#include "memory.h"
#include "hal.h"
#include <string.h>

#define MAX_SYSCALLS 256

// Syscall handler table
static syscall_handler_t syscall_handlers[MAX_SYSCALLS] = {0};

// Current process (set by interrupt handler)
static process_t* current_process = NULL;

// Initialize syscall system
void syscall_init(void) {
    // Clear handler table
    memset(syscall_handlers, 0, sizeof(syscall_handlers));
    
    // Register the basic syscalls
    syscall_register(SYS_EXIT, sys_exit);
    syscall_register(SYS_WRITE, sys_write);
    syscall_register(SYS_GETPID, sys_getpid);
    syscall_register(SYS_READ, sys_read);
    syscall_register(SYS_OPEN, sys_open);
    syscall_register(SYS_CLOSE, sys_close);
    syscall_register(SYS_FORK, sys_fork);
    syscall_register(SYS_EXEC, sys_exec);
    syscall_register(SYS_WAIT, sys_wait);
    syscall_register(SYS_BRK, sys_brk);
    syscall_register(SYS_MMAP, sys_mmap);
    syscall_register(SYS_MUNMAP, sys_munmap);
    syscall_register(SYS_GETTIMEOFDAY, sys_gettimeofday);
    syscall_register(SYS_SLEEP, sys_sleep);
}

// Register a syscall handler
int64_t syscall_register(int syscall_number, syscall_handler_t handler) {
    if (syscall_number < 0 || syscall_number >= MAX_SYSCALLS) {
        return -1;
    }
    
    syscall_handlers[syscall_number] = handler;
    return 0;
}

// Main syscall handler (called from interrupt handler)
void syscall_handler(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    // Get current process (this would be set by the interrupt handler)
    process_t* process = get_current_process();
    if (!process) {
        // No current process, return error
        return;
    }
    
    // Validate syscall number
    if (syscall_number >= MAX_SYSCALLS || !syscall_handlers[syscall_number]) {
        // Invalid syscall
        return;
    }
    
    // Call the appropriate handler
    int64_t result = syscall_handlers[syscall_number](process, arg1, arg2, arg3, arg4, arg5, arg6);
    
    // Store result in RAX (this would be done by the interrupt handler)
    // For now, we'll just return
}

// Syscall: Exit process
int64_t sys_exit(process_t* process, uint64_t exit_code, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    KINFO("Process %d (PID: %d) exiting with code %d", process->pid, process->pid, (int)exit_code);
    
    // Set exit code and state
    process->exit_code = (int)exit_code;
    process->state = PROCESS_STATE_ZOMBIE;
    
    // Clean up process resources
    // Free file descriptors
    if (process->file_descriptors) {
        for (u32 i = 0; i < process->num_file_descriptors; i++) {
            if (process->file_descriptors[i]) {
                // Close file descriptor (placeholder for now)
                process->file_descriptors[i] = NULL;
            }
        }
        kfree(process->file_descriptors);
        process->file_descriptors = NULL;
    }
    
    // Free thread structures
    if (process->threads) {
        for (u32 i = 0; i < process->num_threads; i++) {
            if (process->threads[i]) {
                // Free thread stack
                if (process->threads[i]->stack) {
                    kfree(process->threads[i]->stack);
                }
                kfree(process->threads[i]);
            }
        }
        kfree(process->threads);
        process->threads = NULL;
    }
    
    // Free virtual memory space
    if (process->vm_space) {
        vm_space_destroy(process->vm_space);
        process->vm_space = NULL;
    }
    
    // Free process structure itself
    // Note: We don't free it immediately as the parent might want to get exit status
    // It will be freed when the parent calls wait() or when the process is reaped
    
    // Schedule another process
    schedule_next_process();
    
    // This should never return - the scheduler should switch to another process
    // If we get here, something went wrong
    KERROR("sys_exit returned unexpectedly!");
    hal_halt_cpu();
    
    return 0;
}

// Syscall: Write to file descriptor
int64_t sys_write(process_t* process, uint64_t fd, uint64_t buf, uint64_t count, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, only support stdout (fd = 1) and stderr (fd = 2)
    if (fd != 1 && fd != 2) {
        return -1; // EBADF
    }
    
    // Validate buffer pointer
    if (!buf) {
        return -1; // EFAULT
    }
    
    // Validate count
    if (count == 0) {
        return 0;
    }
    
    // Check if buffer is in valid user space
    if (!vm_validate_user_ptr(process, (void*)buf, count)) {
        return -1; // EFAULT
    }
    
    // Allocate kernel buffer for safe copying
    char* kernel_buf = kmalloc(count);
    if (!kernel_buf) {
        return -1; // ENOMEM
    }
    
    // Copy data from user space to kernel space
    // This prevents TOCTOU vulnerabilities
    if (vm_copy_from_user(process, kernel_buf, (void*)buf, count) != 0) {
        kfree(kernel_buf);
        return -1; // EFAULT
    }
    
    // Write to console (using HAL)
    hal_console_write(kernel_buf, count);
    
    // Clean up kernel buffer
    kfree(kernel_buf);
    
    return count;
}

// Syscall: Get process ID
int64_t sys_getpid(process_t* process, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    return process->pid;
}

// Helper function to get current process
process_t* get_current_process(void) {
    return current_process;
}

// Set current process (called by scheduler)
void set_current_process(process_t* process) {
    current_process = process;
}

// Syscall: Read from file descriptor
int64_t sys_read(process_t* process, uint64_t fd, uint64_t buf, uint64_t count, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, only support stdin (fd = 0)
    if (fd != 0) {
        return -1; // EBADF
    }
    
    // Validate buffer
    if (!buf || count == 0) {
        return 0;
    }
    
    // Check if buffer is in user space
    if (!vm_validate_user_ptr(process, (void*)buf, count)) {
        return -1; // EFAULT
    }
    
    // Read from keyboard
    char* kernel_buf = kmalloc(count);
    if (!kernel_buf) {
        return -1; // ENOMEM
    }
    
    size_t bytes_read = 0;
    for (size_t i = 0; i < count; i++) {
        char ch;
        if (keyboard_read_char(&ch) == 0) {
            kernel_buf[i] = ch;
            bytes_read++;
            
            // Stop on newline for line-based input
            if (ch == '\n') {
                break;
            }
        } else {
            // No more input available
            break;
        }
    }
    
    // Copy data to user space
    if (bytes_read > 0) {
        if (vm_copy_to_user(process, (void*)buf, kernel_buf, bytes_read) != 0) {
            kfree(kernel_buf);
            return -1; // EFAULT
        }
    }
    
    kfree(kernel_buf);
    return bytes_read;
}

// Syscall: Open file
int64_t sys_open(process_t* process, uint64_t pathname, uint64_t flags, uint64_t mode, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, return -1 (not implemented)
    // In a real implementation, this would open files from the filesystem
    return -1; // ENOSYS
}

// Syscall: Close file descriptor
int64_t sys_close(process_t* process, uint64_t fd, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, just return success
    // In a real implementation, this would close the file descriptor
    return 0;
}

// Syscall: Fork process
int64_t sys_fork(process_t* process, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    KINFO("Process %d forking", process->pid);
    
    // Create new process
    process_t* child_process = process_create("forked_process", NULL, NULL, PROCESS_TYPE_NATIVE, process->priority);
    if (!child_process) {
        return -1; // ENOMEM
    }
    
    // Set parent-child relationship
    child_process->ppid = process->pid;
    child_process->parent = process;
    
    // Copy process attributes
    child_process->uid = process->uid;
    child_process->gid = process->gid;
    child_process->flags = process->flags;
    child_process->priority = process->priority;
    
    // Copy file descriptors (shallow copy for now)
    if (process->file_descriptors) {
        child_process->file_descriptors = kmalloc(process->max_file_descriptors * sizeof(void*));
        if (child_process->file_descriptors) {
            memcpy(child_process->file_descriptors, process->file_descriptors, 
                   process->max_file_descriptors * sizeof(void*));
            child_process->max_file_descriptors = process->max_file_descriptors;
            child_process->num_file_descriptors = process->num_file_descriptors;
        }
    }
    
    // Copy virtual memory space with copy-on-write
    if (process->vm_space) {
        // Create new VM space for child
        child_process->vm_space = vm_space_create();
        if (child_process->vm_space) {
            // Copy memory regions with copy-on-write
            vm_region_t* region = process->vm_space->regions;
            while (region) {
                // Map the same physical pages but mark as copy-on-write
                vm_map_region(child_process->vm_space, region->start, region->end,
                             region->prot, region->flags | VM_REGION_COW, region->data);
                region = region->next;
            }
        }
    }
    
    // Copy thread context (simplified - just the main thread for now)
    if (process->threads && process->num_threads > 0) {
        thread_t* parent_thread = process->threads[0];
        if (parent_thread) {
            // Create new thread for child
            thread_t* child_thread = kmalloc(sizeof(thread_t));
            if (child_thread) {
                memcpy(child_thread, parent_thread, sizeof(thread_t));
                child_thread->tid = get_next_thread_id(); // Generate new TID
                child_thread->pid = child_process->pid;
                
                // Allocate new stack for child thread
                child_thread->stack = kmalloc(child_thread->stack_size);
                if (child_thread->stack) {
                    // Copy stack contents
                    memcpy(child_thread->stack, parent_thread->stack, child_thread->stack_size);
                }
                
                // Add thread to child process
                child_process->threads = kmalloc(sizeof(thread_t*));
                child_process->threads[0] = child_thread;
                child_process->num_threads = 1;
                child_process->max_threads = 1;
                child_process->main_thread_id = child_thread->tid;
            }
        }
    }
    
    // Add child to parent's children list
    if (!process->children) {
        process->children = child_process;
    } else {
        // Add to end of children list
        process_t* last_child = process->children;
        while (last_child->next_sibling) {
            last_child = last_child->next_sibling;
        }
        last_child->next_sibling = child_process;
        child_process->prev_sibling = last_child;
    }
    
    KINFO("Forked process %d -> %d", process->pid, child_process->pid);
    
    // Return child PID to parent, 0 to child
    // In a real implementation, this would be handled by the scheduler
    // For now, we'll return the child PID
    return child_process->pid;
}

// Syscall: Execute program
int64_t sys_exec(process_t* process, uint64_t pathname, uint64_t argv, uint64_t envp, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // Validate pathname pointer
    if (!pathname || !vm_validate_user_ptr(process, (void*)pathname, 1)) {
        return -1; // EFAULT
    }
    
    // Copy pathname from user space
    char kernel_path[256];
    if (vm_copy_from_user(process, kernel_path, (void*)pathname, sizeof(kernel_path) - 1) != 0) {
        return -1; // EFAULT
    }
    kernel_path[sizeof(kernel_path) - 1] = '\0';
    
    KINFO("Process %d executing: %s", process->pid, kernel_path);
    
    // Load the new program
    if (load_flat_binary(kernel_path, process) != 0) {
        KERROR("Failed to load program: %s", kernel_path);
        return -1; // ENOENT
    }
    
    // Clean up old process resources (but keep the process structure)
    // Free old file descriptors
    if (process->file_descriptors) {
        for (u32 i = 0; i < process->num_file_descriptors; i++) {
            if (process->file_descriptors[i]) {
                // Close file descriptor (placeholder for now)
                process->file_descriptors[i] = NULL;
            }
        }
        kfree(process->file_descriptors);
        process->file_descriptors = NULL;
        process->num_file_descriptors = 0;
    }
    
    // Free old thread structures
    if (process->threads) {
        for (u32 i = 0; i < process->num_threads; i++) {
            if (process->threads[i]) {
                if (process->threads[i]->stack) {
                    kfree(process->threads[i]->stack);
                }
                kfree(process->threads[i]);
            }
        }
        kfree(process->threads);
        process->threads = NULL;
        process->num_threads = 0;
    }
    
    // Free old virtual memory space
    if (process->vm_space) {
        vm_space_destroy(process->vm_space);
        process->vm_space = NULL;
    }
    
    // Set up new main thread
    thread_t* main_thread = kmalloc(sizeof(thread_t));
    if (main_thread) {
        memset(main_thread, 0, sizeof(thread_t));
        main_thread->tid = get_next_thread_id();
        main_thread->pid = process->pid;
        strcpy(main_thread->name, "main");
        main_thread->state = THREAD_STATE_READY;
        main_thread->priority = process->priority;
        main_thread->entry_point = (void*)process->entry_point;
        
        // Allocate stack for new thread
        main_thread->stack_size = DEFAULT_STACK_SIZE;
        main_thread->stack = kmalloc(main_thread->stack_size);
        if (main_thread->stack) {
            memset(main_thread->stack, 0, main_thread->stack_size);
        }
        
        // Add thread to process
        process->threads = kmalloc(sizeof(thread_t*));
        process->threads[0] = main_thread;
        process->num_threads = 1;
        process->max_threads = 1;
        process->main_thread_id = main_thread->tid;
    }
    
    // Set up standard file descriptors
    process->file_descriptors = kmalloc(3 * sizeof(void*));
    if (process->file_descriptors) {
        process->file_descriptors[0] = (void*)0x1; // stdin (placeholder)
        process->file_descriptors[1] = (void*)0x2; // stdout (placeholder)
        process->file_descriptors[2] = (void*)0x3; // stderr (placeholder)
        process->max_file_descriptors = 3;
        process->num_file_descriptors = 3;
    }
    
    // Jump to new program entry point
    // This will replace the current process's execution
    jump_to_user_mode(process, process->entry_point);
    
    // Should never reach here
    KERROR("sys_exec returned unexpectedly!");
    return -1;
}

// Syscall: Wait for child process
int64_t sys_wait(process_t* process, uint64_t pid, uint64_t status, uint64_t options, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    KINFO("Process %d waiting for child (pid=%d)", process->pid, (int)pid);
    
    // Find the child process to wait for
    process_t* child = NULL;
    process_t* current_child = process->children;
    
    while (current_child) {
        if (pid == 0 || current_child->pid == (pid_t)pid) {
            // Check if this child is a zombie (has exited)
            if (current_child->state == PROCESS_STATE_ZOMBIE) {
                child = current_child;
                break;
            }
        }
        current_child = current_child->next_sibling;
    }
    
    if (!child) {
        // No zombie children found
        if (pid == 0) {
            // Wait for any child
            return -1; // ECHILD - no children
        } else {
            // Wait for specific child that doesn't exist or isn't zombie
            return -1; // ECHILD
        }
    }
    
    // Get the exit status
    if (status && vm_validate_user_ptr(process, (void*)status, sizeof(int))) {
        int exit_status = child->exit_code;
        vm_copy_to_user(process, (void*)status, &exit_status, sizeof(int));
    }
    
    // Remove child from parent's children list
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        process->children = child->next_sibling;
    }
    
    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    }
    
    // Free the child process structure
    kfree(child);
    
    KINFO("Process %d collected child %d (exit code: %d)", 
          process->pid, child->pid, child->exit_code);
    
    return child->pid;
}

// Syscall: Change data segment size
int64_t sys_brk(process_t* process, uint64_t addr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, return current break (not implemented)
    // In a real implementation, this would adjust the heap size
    return process->heap_end;
}

// Syscall: Memory map
int64_t sys_mmap(process_t* process, uint64_t addr, uint64_t length, uint64_t prot, uint64_t flags, uint64_t fd, uint64_t offset) {
    if (!process) {
        return -1;
    }
    
    // For now, return -1 (not implemented)
    // In a real implementation, this would map memory regions
    return -1; // ENOSYS
}

// Syscall: Unmap memory
int64_t sys_munmap(process_t* process, uint64_t addr, uint64_t length, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, return success (not implemented)
    // In a real implementation, this would unmap memory regions
    return 0;
}

// Syscall: Get time of day
int64_t sys_gettimeofday(process_t* process, uint64_t tv, uint64_t tz, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, return -1 (not implemented)
    // In a real implementation, this would return the current time
    return -1; // ENOSYS
}

// Syscall: Sleep
int64_t sys_sleep(process_t* process, uint64_t seconds, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (!process) {
        return -1;
    }
    
    // For now, return success (not implemented)
    // In a real implementation, this would sleep for the specified time
    return 0;
} 