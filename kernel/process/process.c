#include "process.h"
#include "memory.h"
#include "scheduler.h"
#include "kernel.h"
#include <string.h>

// Global process management state
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static pid_t next_pid = 1;
static tid_t next_tid = 1;
static bool process_management_initialized = false;

// Process table (for quick PID lookups)
static process_t* process_table[MAX_PROCESSES] = {NULL};

// Get next available thread ID
tid_t get_next_thread_id(void) {
    return next_tid++;
}

// Get next available process ID
pid_t get_next_pid(void) {
    // Find next available PID
    for (pid_t pid = next_pid; pid < MAX_PROCESSES; pid++) {
        if (!process_table[pid]) {
            next_pid = pid + 1;
            return pid;
        }
    }
    
    // Wrap around if we've used all PIDs
    for (pid_t pid = 1; pid < next_pid; pid++) {
        if (!process_table[pid]) {
            next_pid = pid + 1;
            return pid;
        }
    }
    
    // No available PIDs
    return 0;
}

// Initialize process management
error_t process_init(void) {
    if (process_management_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing process management");
    
    // Initialize process table
    memset(process_table, 0, sizeof(process_table));
    
    // Create idle process
    process_t* idle_process = NULL;
    error_t result = process_create("idle", NULL, NULL, PROCESS_TYPE_SYSTEM, PRIORITY_IDLE, &idle_process);
    if (result != SUCCESS) {
        KERROR("Failed to create idle process");
        return result;
    }
    
    idle_process->state = PROCESS_STATE_RUNNING;
    current_process = idle_process;
    
    // Initialize scheduler
    result = scheduler_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize scheduler");
        return result;
    }
    
    process_management_initialized = true;
    
    KINFO("Process management initialized");
    return SUCCESS;
}

// Shutdown process management
void process_shutdown(void) {
    if (!process_management_initialized) {
        return;
    }
    
    KINFO("Shutting down process management");
    
    // Terminate all processes
    process_t* process = process_list;
    while (process) {
        process_t* next = process->next_sibling;
        process_terminate(process->pid, 0);
        process = next;
    }
    
    process_management_initialized = false;
}

// Create a new process
error_t process_create(const char* name, void* entry_point, void* arg, 
                      process_type_t type, priority_t priority, process_t** process) {
    if (!name || !process) {
        return E_INVAL;
    }
    
    // Check if we have room for another process
    if (next_pid >= MAX_PROCESSES) {
        return E_NOMEM;
    }
    
    // Allocate process structure
    process_t* new_process = (process_t*)memory_alloc(sizeof(process_t));
    if (!new_process) {
        return E_NOMEM;
    }
    
    // Initialize process
    memset(new_process, 0, sizeof(process_t));
    new_process->pid = get_next_pid();
    if (new_process->pid == 0) {
        memory_free(new_process);
        return E_NOMEM;
    }
    
    strncpy(new_process->name, name, sizeof(new_process->name) - 1);
    new_process->name[sizeof(new_process->name) - 1] = '\0';
    new_process->state = PROCESS_STATE_NEW;
    new_process->type = type;
    new_process->priority = priority;
    new_process->entry_point = (uintptr_t)entry_point;
    
    // Set default values
    new_process->uid = 0; // root
    new_process->gid = 0; // root
    new_process->flags = 0;
    new_process->quantum = 100; // 100ms default quantum
    new_process->memory_limit = 1024 * 1024 * 1024; // 1GB default
    new_process->cpu_limit = 100; // 100% CPU limit
    new_process->file_limit = 1024; // 1024 file descriptors
    
    // Initialize file descriptor array
    new_process->file_descriptors = (void**)memory_alloc(sizeof(void*) * new_process->file_limit);
    if (!new_process->file_descriptors) {
        memory_free(new_process);
        return E_NOMEM;
    }
    memset(new_process->file_descriptors, 0, sizeof(void*) * new_process->file_limit);
    
    // Add to process table
    process_table[new_process->pid] = new_process;
    
    // Add to process list
    new_process->next_sibling = process_list;
    if (process_list) {
        process_list->prev_sibling = new_process;
    }
    process_list = new_process;
    
    *process = new_process;
    
    KDEBUG("Created process PID=%u, name='%s'", new_process->pid, new_process->name);
    return SUCCESS;
}

// Destroy a process
error_t process_destroy(pid_t pid) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    // Terminate the process first
    error_t result = process_terminate(pid, 0);
    if (result != SUCCESS) {
        return result;
    }
    
    // Remove from process table
    process_table[pid] = NULL;
    
    // Remove from process list
    if (process->prev_sibling) {
        process->prev_sibling->next_sibling = process->next_sibling;
    } else {
        process_list = process->next_sibling;
    }
    
    if (process->next_sibling) {
        process->next_sibling->prev_sibling = process->prev_sibling;
    }
    
    // Free process resources
    if (process->file_descriptors) {
        memory_free(process->file_descriptors);
    }
    
    memory_free(process);
    
    KDEBUG("Destroyed process PID=%u", pid);
    return SUCCESS;
}

// Terminate a process
error_t process_terminate(pid_t pid, int exit_code) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    KDEBUG("Terminating process PID=%u with exit code %d", pid, exit_code);
    
    // Set exit information
    process->exit_code = exit_code;
    process->state = PROCESS_STATE_TERMINATED;
    
    // Terminate all threads in the process
    for (u32 i = 0; i < process->num_threads; i++) {
        if (process->threads[i]) {
            thread_terminate(process->threads[i]->tid);
        }
    }
    
    // Remove from scheduler
    scheduler_remove_process(process);
    
    // Free process memory
    // This would normally be done by the memory management system
    // For now, just mark as terminated
    
    KDEBUG("Process PID=%u terminated", pid);
    return SUCCESS;
}

// Wait for a process to terminate
error_t process_wait(pid_t pid, int* exit_code) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    // Wait for process to terminate
    while (process->state != PROCESS_STATE_TERMINATED) {
        // Yield to other processes
        scheduler_yield();
    }
    
    if (exit_code) {
        *exit_code = process->exit_code;
    }
    
    return SUCCESS;
}

// Get process by PID
process_t* process_get_by_pid(pid_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}

// Get current process
process_t* process_get_current(void) {
    return current_process;
}

// Set current process
void process_set_current(process_t* process) {
    current_process = process;
}

// Get process information
error_t process_get_info(pid_t pid, process_t* info) {
    process_t* process = process_get_by_pid(pid);
    if (!process || !info) {
        return E_INVAL;
    }
    
    memcpy(info, process, sizeof(process_t));
    return SUCCESS;
}

// Get list of all processes
error_t process_get_list(process_t* processes, u32* count) {
    if (!processes || !count) {
        return E_INVAL;
    }
    
    u32 max_count = *count;
    u32 actual_count = 0;
    
    process_t* process = process_list;
    while (process && actual_count < max_count) {
        memcpy(&processes[actual_count], process, sizeof(process_t));
        actual_count++;
        process = process->next_sibling;
    }
    
    *count = actual_count;
    return SUCCESS;
}

// Suspend a process
error_t process_suspend(pid_t pid) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    if (process->state == PROCESS_STATE_RUNNING || process->state == PROCESS_STATE_READY) {
        process->state = PROCESS_STATE_SLEEPING;
        scheduler_remove_process(process);
        KDEBUG("Suspended process PID=%u", pid);
    }
    
    return SUCCESS;
}

// Resume a process
error_t process_resume(pid_t pid) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    if (process->state == PROCESS_STATE_SLEEPING) {
        process->state = PROCESS_STATE_READY;
        scheduler_add_process(process);
        KDEBUG("Resumed process PID=%u", pid);
    }
    
    return SUCCESS;
}

// Set process priority
error_t process_set_priority(pid_t pid, priority_t priority) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    process->priority = priority;
    
    // Update scheduler if process is currently scheduled
    if (process->state == PROCESS_STATE_READY || process->state == PROCESS_STATE_RUNNING) {
        scheduler_update_process_priority(process);
    }
    
    KDEBUG("Set process PID=%u priority to %d", pid, priority);
    return SUCCESS;
}

// Set process CPU affinity
error_t process_set_affinity(pid_t pid, u32 cpu_mask) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_NOENT;
    }
    
    process->cpu_affinity = cpu_mask;
    KDEBUG("Set process PID=%u CPU affinity to 0x%x", pid, cpu_mask);
    return SUCCESS;
}

// Schedule next process
void schedule_next_process(void) {
    if (!process_management_initialized) {
        return;
    }
    
    // Get next process from scheduler
    process_t* next_process = scheduler_next_process();
    if (next_process && next_process != current_process) {
        // Context switch
        process_t* old_process = current_process;
        current_process = next_process;
        
        KDEBUG("Context switch: PID=%u -> PID=%u", 
               old_process ? old_process->pid : 0,
               next_process->pid);
        
        // This would normally involve switching address spaces
        // For now, just update the current process pointer
    }
}

// Dump process information
void process_dump_info(process_t* process) {
    if (!process) {
        return;
    }
    
    KINFO("Process PID=%u: name='%s', state=%d, priority=%d", 
          process->pid, process->name, process->state, process->priority);
    KINFO("  UID=%u, GID=%u, memory_usage=%llu, cpu_usage=%u%%",
          process->uid, process->gid, process->memory_usage, process->cpu_usage_percent);
}

// Dump all processes
void process_dump_all(void) {
    KINFO("=== Process List ===");
    
    process_t* process = process_list;
    while (process) {
        process_dump_info(process);
        process = process->next_sibling;
    }
    
    KINFO("=== End Process List ===");
}

// Validate process integrity
void process_validate_integrity(void) {
    u32 count = 0;
    process_t* process = process_list;
    
    while (process) {
        count++;
        
        // Validate process table entry
        if (process_table[process->pid] != process) {
            KERROR("Process table inconsistency: PID=%u", process->pid);
        }
        
        // Validate process state
        if (process->state < PROCESS_STATE_NEW || process->state > PROCESS_STATE_ZOMBIE) {
            KERROR("Invalid process state: PID=%u, state=%d", process->pid, process->state);
        }
        
        process = process->next_sibling;
    }
    
    KDEBUG("Process integrity check: %u processes", count);
}

// Fork a process
error_t process_fork(process_t* parent, process_t** child) {
    if (!parent || !child) {
        return E_INVAL;
    }
    
    KDEBUG("Forking process PID=%u", parent->pid);
    
    // Create new process
    process_t* new_process = NULL;
    error_t result = process_create(parent->name, parent->entry_point, NULL, 
                                   parent->type, parent->priority, &new_process);
    if (result != SUCCESS) {
        return result;
    }
    
    // Set parent-child relationship
    new_process->ppid = parent->pid;
    new_process->parent = parent;
    
    // Copy process attributes
    new_process->uid = parent->uid;
    new_process->gid = parent->gid;
    new_process->flags = parent->flags;
    new_process->quantum = parent->quantum;
    new_process->memory_limit = parent->memory_limit;
    new_process->cpu_limit = parent->cpu_limit;
    new_process->file_limit = parent->file_limit;
    
    // Copy file descriptors
    for (int i = 0; i < parent->file_limit; i++) {
        if (parent->file_descriptors[i]) {
            // TODO: Implement file descriptor duplication
            new_process->file_descriptors[i] = parent->file_descriptors[i];
        }
    }
    
    // Copy memory space (copy-on-write)
    result = memory_fork_process(parent, new_process);
    if (result != SUCCESS) {
        process_destroy(new_process->pid);
        return result;
    }
    
    // Set process state
    new_process->state = PROCESS_STATE_READY;
    
    // Add to scheduler
    result = scheduler_add_process(new_process);
    if (result != SUCCESS) {
        process_destroy(new_process->pid);
        return result;
    }
    
    *child = new_process;
    
    KDEBUG("Forked process PID=%u -> PID=%u", parent->pid, new_process->pid);
    return SUCCESS;
}

// Execute a new program
error_t process_exec(const char* filename, char** argv, char** envp) {
    KDEBUG("Executing program: %s", filename);
    
    process_t* current = process_get_current();
    if (!current) {
        return E_INVAL;
    }
    
    // Load the executable
    void* entry_point;
    error_t result = elf_load_program(filename, &entry_point);
    if (result != SUCCESS) {
        KERROR("Failed to load program %s", filename);
        return result;
    }
    
    // Set up new memory space
    result = memory_exec_process(current, entry_point);
    if (result != SUCCESS) {
        KERROR("Failed to set up memory space for %s", filename);
        return result;
    }
    
    // Set up arguments and environment
    result = process_setup_args(current, argv, envp);
    if (result != SUCCESS) {
        KERROR("Failed to set up arguments for %s", filename);
        return result;
    }
    
    // Update process entry point
    current->entry_point = (uintptr_t)entry_point;
    
    // Set process state
    current->state = PROCESS_STATE_RUNNING;
    
    KDEBUG("Executed program: %s", filename);
    return SUCCESS;
}

// Get process break (heap end)
uintptr_t process_get_break(process_t* process) {
    if (!process) {
        return 0;
    }
    
    // TODO: Implement proper heap management
    return process->heap_end;
}

// Set process break
error_t process_set_break(process_t* process, uintptr_t new_brk) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("Setting break for PID=%u: 0x%lx -> 0x%lx", process->pid, process->heap_end, new_brk);
    
    // TODO: Implement proper heap expansion/contraction
    process->heap_end = new_brk;
    
    return SUCCESS;
}

// Memory mapping
error_t process_mmap(process_t* process, uintptr_t addr, size_t length, int prot, int flags, int fd, off_t offset, uintptr_t* mapped_addr) {
    if (!process || !mapped_addr) {
        return E_INVAL;
    }
    
    KDEBUG("MMAP for PID=%u: addr=0x%lx, length=%zu, prot=%d, flags=%d", 
           process->pid, addr, length, prot, flags);
    
    // TODO: Implement proper memory mapping
    // For now, just allocate memory
    void* memory = memory_alloc(length);
    if (!memory) {
        return E_NOMEM;
    }
    
    *mapped_addr = (uintptr_t)memory;
    return SUCCESS;
}

// Memory unmapping
error_t process_munmap(process_t* process, uintptr_t addr, size_t length) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("MUNMAP for PID=%u: addr=0x%lx, length=%zu", process->pid, addr, length);
    
    // TODO: Implement proper memory unmapping
    // For now, just free memory
    memory_free((void*)addr);
    
    return SUCCESS;
}

// Memory protection
error_t process_mprotect(process_t* process, uintptr_t addr, size_t length, int prot) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("MPROTECT for PID=%u: addr=0x%lx, length=%zu, prot=%d", 
           process->pid, addr, length, prot);
    
    // TODO: Implement proper memory protection
    return SUCCESS;
}

// Process sleep
error_t process_sleep_nanos(const struct timespec* req, struct timespec* rem) {
    if (!req) {
        return E_INVAL;
    }
    
    KDEBUG("Process sleep for %ld.%09ld seconds", req->tv_sec, req->tv_nsec);
    
    // TODO: Implement proper sleep with timer
    // For now, just yield
    process_yield();
    
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    
    return SUCCESS;
}

// Send signal to process
error_t process_send_signal(pid_t pid, int signal) {
    process_t* target = process_get_by_pid(pid);
    if (!target) {
        return E_NOENT;
    }
    
    KDEBUG("Sending signal %d to PID=%u", signal, pid);
    
    // TODO: Implement proper signal handling
    // For now, just terminate on SIGTERM/SIGKILL
    if (signal == SIGTERM || signal == SIGKILL) {
        return process_terminate(pid, 0);
    }
    
    return SUCCESS;
}

// Set signal handler
error_t process_set_signal_handler(process_t* process, int signal, void* handler) {
    if (!process || signal < 0 || signal >= MAX_SIGNALS) {
        return E_INVAL;
    }
    
    KDEBUG("Setting signal handler for PID=%u, signal=%d", process->pid, signal);
    
    // TODO: Implement proper signal handler storage
    process->signal_handlers[signal] = handler;
    
    return SUCCESS;
}

// Set signal action
error_t process_set_signal_action(process_t* process, int signal, const struct sigaction* act, struct sigaction* oact) {
    if (!process || signal < 0 || signal >= MAX_SIGNALS) {
        return E_INVAL;
    }
    
    KDEBUG("Setting signal action for PID=%u, signal=%d", process->pid, signal);
    
    // TODO: Implement proper sigaction
    if (oact) {
        // Return old action
        memset(oact, 0, sizeof(*oact));
    }
    
    if (act) {
        // Set new action
        process->signal_handlers[signal] = act->sa_handler;
    }
    
    return SUCCESS;
}

// Setup process arguments
error_t process_setup_args(process_t* process, char** argv, char** envp) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("Setting up arguments for PID=%u", process->pid);
    
    // TODO: Implement proper argument setup
    // For now, just store pointers
    process->argv = argv;
    process->envp = envp;
    
    return SUCCESS;
}

// Process yield
void process_yield(void) {
    KDEBUG("Process yield");
    
    // Schedule next process
    schedule_next_process();
} 