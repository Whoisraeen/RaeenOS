#include "process/include/process.h"
#include "memory/memory.h"
#include "filesystem/vfs.h"
#include "kernel.h"
#include <string.h>

// Signal definitions
#define MAX_SIGNALS 32
#define SIG_IGN 1
#define SIG_DFL 0

// Signal numbers (POSIX standard)
#define SIGHUP  1
#define SIGINT  2
#define SIGQUIT 3
#define SIGILL  4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS  7
#define SIGFPE  8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22

// Signal action structure
typedef struct {
    void (*sa_handler)(int);
    u64 sa_flags;
    void (*sa_restorer)(void);
} sigaction_t;

// Signal state
typedef struct {
    sigaction_t actions[MAX_SIGNALS];
    u64 pending_signals;
    u64 blocked_signals;
    u64 ignored_signals;
} signal_state_t;

// Process exit information
typedef struct {
    int exit_code;
    u32 exit_signal;
    bool core_dump;
    u64 exit_time;
} exit_info_t;

// Forward declarations
static error_t copy_process_memory(process_t* parent, process_t* child);
static error_t copy_process_file_descriptors(process_t* parent, process_t* child);
static error_t setup_child_stack(process_t* child, void* parent_stack, size_t stack_size);
static void signal_handler_default(int sig);
static void signal_handler_ignore(int sig);
static error_t deliver_signal(process_t* process, int signal);
static void handle_signal(process_t* process, int signal);
static void cleanup_zombie_processes(void);

// Initialize process lifecycle management
error_t process_lifecycle_init(void) {
    KINFO("Initializing process lifecycle management");
    
    // Initialize signal handling
    // Set up default signal handlers
    
    return SUCCESS;
}

// Fork a process
error_t process_fork(process_t* parent, process_t** child) {
    if (!parent || !child) {
        return E_INVAL;
    }
    
    KDEBUG("Forking process PID=%u", parent->pid);
    
    // Create new process structure
    process_t* new_process = NULL;
    error_t result = process_create(parent->name, parent->entry_point, NULL, 
                                   parent->type, parent->priority, &new_process);
    if (result != SUCCESS) {
        KERROR("Failed to create child process");
        return result;
    }
    
    // Set parent-child relationship
    new_process->ppid = parent->pid;
    new_process->parent = parent;
    
    // Copy process attributes
    new_process->uid = parent->uid;
    new_process->gid = parent->gid;
    new_process->flags = parent->flags;
    new_process->memory_limit = parent->memory_limit;
    new_process->cpu_limit = parent->cpu_limit;
    new_process->file_limit = parent->file_limit;
    
    // Copy memory (copy-on-write)
    result = copy_process_memory(parent, new_process);
    if (result != SUCCESS) {
        KERROR("Failed to copy process memory");
        process_destroy(new_process->pid);
        return result;
    }
    
    // Copy file descriptors
    result = copy_process_file_descriptors(parent, new_process);
    if (result != SUCCESS) {
        KERROR("Failed to copy file descriptors");
        process_destroy(new_process->pid);
        return result;
    }
    
    // Copy signal handlers
    memcpy(&new_process->signal_state, &parent->signal_state, sizeof(signal_state_t));
    
    // Set up child stack
    if (parent->stack_start && parent->stack_end) {
        size_t stack_size = parent->stack_end - parent->stack_start;
        result = setup_child_stack(new_process, (void*)parent->stack_start, stack_size);
        if (result != SUCCESS) {
            KERROR("Failed to setup child stack");
            process_destroy(new_process->pid);
            return result;
        }
    }
    
    // Add to parent's child list
    new_process->next_sibling = parent->children;
    if (parent->children) {
        parent->children->prev_sibling = new_process;
    }
    parent->children = new_process;
    
    *child = new_process;
    
    KDEBUG("Successfully forked process: PID=%u -> PID=%u", parent->pid, new_process->pid);
    return SUCCESS;
}

// Execute a new program
error_t process_exec(const char* path, char* const argv[], char* const envp[]) {
    if (!path || !current_process) {
        return E_INVAL;
    }
    
    KDEBUG("Executing program: %s", path);
    
    // Open executable file
    int fd = vfs_open(path, O_RDONLY, 0);
    if (fd < 0) {
        KERROR("Failed to open executable: %s", path);
        return E_NOENT;
    }
    
    // Load executable (this would parse ELF/PE format)
    // For now, just create a simple flat binary loader
    
    // Clear current process memory (except kernel parts)
    // This would involve unmapping user space pages
    
    // Load new program
    void* entry_point = load_executable(fd, current_process);
    vfs_close(fd);
    
    if (!entry_point) {
        KERROR("Failed to load executable: %s", path);
        return E_NOEXEC;
    }
    
    // Update process information
    strncpy(current_process->name, path, sizeof(current_process->name) - 1);
    current_process->entry_point = (uintptr_t)entry_point;
    
    // Set up new stack with arguments
    setup_exec_stack(current_process, argv, envp);
    
    // Clear signal handlers (new program gets default handlers)
    memset(&current_process->signal_state, 0, sizeof(signal_state_t));
    
    KDEBUG("Successfully executed: %s", path);
    return SUCCESS;
}

// Wait for a child process to terminate
error_t process_wait(pid_t pid, int* exit_code) {
    if (!current_process) {
        return E_EPERM;
    }
    
    KDEBUG("Waiting for process PID=%u", pid);
    
    process_t* child = NULL;
    
    if (pid == -1) {
        // Wait for any child
        child = current_process->children;
        while (child) {
            if (child->state == PROCESS_STATE_ZOMBIE) {
                break;
            }
            child = child->next_sibling;
        }
    } else {
        // Wait for specific child
        child = process_get_by_pid(pid);
        if (!child || child->parent != current_process) {
            return E_CHILD;
        }
    }
    
    if (!child) {
        return E_CHILD;
    }
    
    // Wait for child to become zombie
    while (child->state != PROCESS_STATE_ZOMBIE) {
        scheduler_yield();
    }
    
    // Get exit information
    if (exit_code) {
        *exit_code = child->exit_code;
    }
    
    // Clean up zombie process
    process_destroy(child->pid);
    
    KDEBUG("Process PID=%u terminated with exit code %d", child->pid, child->exit_code);
    return child->pid;
}

// Exit current process
void process_exit(int exit_code) {
    if (!current_process) {
        KERROR("No current process to exit");
        return;
    }
    
    KDEBUG("Process PID=%u exiting with code %d", current_process->pid, exit_code);
    
    // Set exit information
    current_process->exit_code = exit_code;
    current_process->state = PROCESS_STATE_ZOMBIE;
    
    // Close all file descriptors
    for (u32 i = 0; i < current_process->num_file_descriptors; i++) {
        if (current_process->file_descriptors[i]) {
            vfs_close(i);
            current_process->file_descriptors[i] = NULL;
        }
    }
    
    // Free process memory (except kernel parts)
    // This would involve unmapping user space pages
    
    // Send SIGCHLD to parent
    if (current_process->parent) {
        process_send_signal(current_process->parent->pid, SIGCHLD);
    }
    
    // Remove from parent's child list
    if (current_process->parent) {
        if (current_process->prev_sibling) {
            current_process->prev_sibling->next_sibling = current_process->next_sibling;
        } else {
            current_process->parent->children = current_process->next_sibling;
        }
        
        if (current_process->next_sibling) {
            current_process->next_sibling->prev_sibling = current_process->prev_sibling;
        }
    }
    
    // Switch to another process
    scheduler_yield();
    
    // This should never be reached
    KERROR("Process exit failed to switch to another process");
    hal_halt_cpu();
}

// Send signal to process
error_t process_send_signal(pid_t pid, int signal) {
    if (signal < 1 || signal >= MAX_SIGNALS) {
        return E_INVAL;
    }
    
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_SRCH;
    }
    
    // Check permissions (only root or same user can send signals)
    if (current_process && current_process->uid != 0 && 
        current_process->uid != process->uid) {
        return E_PERM;
    }
    
    KDEBUG("Sending signal %d to process PID=%u", signal, pid);
    
    return deliver_signal(process, signal);
}

// Set signal handler
error_t process_set_signal_handler(pid_t pid, int signal, sigaction_t* action) {
    if (!current_process || signal < 1 || signal >= MAX_SIGNALS) {
        return E_INVAL;
    }
    
    if (signal == SIGKILL || signal == SIGSTOP) {
        return E_INVAL; // Cannot be caught or ignored
    }
    
    if (action) {
        current_process->signal_state.actions[signal] = *action;
    } else {
        // Set default handler
        current_process->signal_state.actions[signal].sa_handler = signal_handler_default;
        current_process->signal_state.actions[signal].sa_flags = 0;
    }
    
    KDEBUG("Set signal handler for signal %d in process PID=%u", signal, current_process->pid);
    return SUCCESS;
}

// Block/unblock signals
error_t process_signal_mask(int how, u64* set, u64* oldset) {
    if (!current_process) {
        return E_EPERM;
    }
    
    if (oldset) {
        *oldset = current_process->signal_state.blocked_signals;
    }
    
    switch (how) {
        case SIG_BLOCK:
            if (set) {
                current_process->signal_state.blocked_signals |= *set;
            }
            break;
        case SIG_UNBLOCK:
            if (set) {
                current_process->signal_state.blocked_signals &= ~(*set);
            }
            break;
        case SIG_SETMASK:
            if (set) {
                current_process->signal_state.blocked_signals = *set;
            }
            break;
        default:
            return E_INVAL;
    }
    
    return SUCCESS;
}

// Copy process memory (copy-on-write)
static error_t copy_process_memory(process_t* parent, process_t* child) {
    // Create new address space for child
    address_space_t* child_as = address_space_create();
    if (!child_as) {
        return E_NOMEM;
    }
    
    child->address_space = child_as;
    
    // Copy page tables with copy-on-write
    // This would involve:
    // 1. Copy parent's page tables
    // 2. Mark pages as read-only
    // 3. Set up copy-on-write handlers
    
    // For now, just allocate new memory
    child->code_start = USER_SPACE_START;
    child->code_end = USER_SPACE_START + 0x1000; // 4KB code
    child->data_start = child->code_end;
    child->data_end = child->data_start + 0x1000; // 4KB data
    child->stack_start = USER_SPACE_END - 0x10000; // 64KB stack
    child->stack_end = USER_SPACE_END;
    child->heap_start = child->data_end;
    child->heap_end = child->stack_start;
    
    return SUCCESS;
}

// Copy process file descriptors
static error_t copy_process_file_descriptors(process_t* parent, process_t* child) {
    // Copy file descriptor table
    for (u32 i = 0; i < parent->num_file_descriptors; i++) {
        if (parent->file_descriptors[i]) {
            // Duplicate file descriptor
            int new_fd = vfs_dup(i);
            if (new_fd >= 0) {
                child->file_descriptors[i] = (void*)(uintptr_t)new_fd;
            }
        }
    }
    
    child->num_file_descriptors = parent->num_file_descriptors;
    return SUCCESS;
}

// Set up child stack
static error_t setup_child_stack(process_t* child, void* parent_stack, size_t stack_size) {
    // Allocate new stack for child
    void* child_stack = memory_alloc(stack_size);
    if (!child_stack) {
        return E_NOMEM;
    }
    
    // Copy stack contents
    memcpy(child_stack, parent_stack, stack_size);
    
    child->stack_start = (virt_addr_t)child_stack;
    child->stack_end = (virt_addr_t)child_stack + stack_size;
    
    return SUCCESS;
}

// Deliver signal to process
static error_t deliver_signal(process_t* process, int signal) {
    // Check if signal is blocked
    if (process->signal_state.blocked_signals & (1ULL << signal)) {
        return SUCCESS; // Signal blocked
    }
    
    // Check if signal is ignored
    if (process->signal_state.ignored_signals & (1ULL << signal)) {
        return SUCCESS; // Signal ignored
    }
    
    // Set pending signal
    process->signal_state.pending_signals |= (1ULL << signal);
    
    // Wake up process if it's sleeping
    if (process->state == PROCESS_STATE_SLEEPING) {
        process->state = PROCESS_STATE_READY;
        scheduler_add_process(process);
    }
    
    return SUCCESS;
}

// Handle signal
static void handle_signal(process_t* process, int signal) {
    sigaction_t* action = &process->signal_state.actions[signal];
    
    if (action->sa_handler == SIG_IGN) {
        // Signal ignored
        return;
    } else if (action->sa_handler == SIG_DFL) {
        // Default handler
        signal_handler_default(signal);
    } else if (action->sa_handler) {
        // Custom handler
        action->sa_handler(signal);
    }
    
    // Clear pending signal
    process->signal_state.pending_signals &= ~(1ULL << signal);
}

// Default signal handler
static void signal_handler_default(int sig) {
    KDEBUG("Default handler for signal %d", sig);
    
    switch (sig) {
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
            process_exit(128 + sig);
            break;
        case SIGSEGV:
        case SIGBUS:
        case SIGILL:
        case SIGFPE:
            process_exit(128 + sig);
            break;
        case SIGCHLD:
            // Ignore
            break;
        default:
            // Ignore other signals
            break;
    }
}

// Ignore signal handler
static void signal_handler_ignore(int sig) {
    // Do nothing
    (void)sig;
}

// Check for pending signals
void process_check_signals(void) {
    if (!current_process) {
        return;
    }
    
    signal_state_t* sig_state = &current_process->signal_state;
    
    // Check for pending signals
    for (int sig = 1; sig < MAX_SIGNALS; sig++) {
        if (sig_state->pending_signals & (1ULL << sig)) {
            if (!(sig_state->blocked_signals & (1ULL << sig))) {
                handle_signal(current_process, sig);
            }
        }
    }
}

// Load executable (simple flat binary loader)
void* load_executable(int fd, process_t* process) {
    // For now, just load a simple flat binary
    // In a real implementation, this would parse ELF/PE headers
    
    char buffer[1024];
    ssize_t bytes_read = vfs_read(fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        return NULL;
    }
    
    // Allocate memory for executable
    void* exec_mem = memory_alloc(bytes_read);
    if (!exec_mem) {
        return NULL;
    }
    
    // Copy executable data
    memcpy(exec_mem, buffer, bytes_read);
    
    // Set up process memory regions
    process->code_start = (virt_addr_t)exec_mem;
    process->code_end = (virt_addr_t)exec_mem + bytes_read;
    
    return exec_mem;
}

// Set up execution stack with arguments
void setup_exec_stack(process_t* process, char* const argv[], char* const envp[]) {
    // This would set up the stack with argc, argv, and envp
    // For now, just allocate a basic stack
    
    void* stack = memory_alloc(0x10000); // 64KB stack
    if (stack) {
        process->stack_start = (virt_addr_t)stack;
        process->stack_end = (virt_addr_t)stack + 0x10000;
    }
}

// Clean up zombie processes
static void cleanup_zombie_processes(void) {
    process_t* process = process_list;
    while (process) {
        process_t* next = process->next_sibling;
        
        if (process->state == PROCESS_STATE_ZOMBIE) {
            // Remove from process table
            process_table[process->pid] = NULL;
            
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
            memory_free(process);
        }
        
        process = next;
    }
}

// Get process exit status
error_t process_get_exit_status(pid_t pid, int* exit_code, int* exit_signal) {
    process_t* process = process_get_by_pid(pid);
    if (!process) {
        return E_SRCH;
    }
    
    if (process->state != PROCESS_STATE_ZOMBIE) {
        return E_AGAIN; // Process not terminated
    }
    
    if (exit_code) {
        *exit_code = process->exit_code;
    }
    
    if (exit_signal) {
        *exit_signal = process->exit_signal;
    }
    
    return SUCCESS;
}

// Dump process information
void process_dump_lifecycle_info(process_t* process) {
    if (!process) {
        return;
    }
    
    KINFO("Process PID=%u lifecycle info:", process->pid);
    KINFO("  State: %d", process->state);
    KINFO("  Parent PID: %u", process->ppid);
    KINFO("  Exit code: %d", process->exit_code);
    KINFO("  Exit signal: %u", process->exit_signal);
    KINFO("  Pending signals: 0x%llx", process->signal_state.pending_signals);
    KINFO("  Blocked signals: 0x%llx", process->signal_state.blocked_signals);
} 