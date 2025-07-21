#include "include/process.h"
#include "../core/include/kernel.h"
#include "../memory/include/memory.h"
#include "../security/include/security.h"
#include "../core/include/interrupts.h"
#include "../hal/include/hal.h"
#include "../memory/memory_integration.h"

// Global process management state
static bool process_initialized = false;
static pid_t next_pid = 1;
static tid_t next_tid = 1;

// Process and thread lists
static process_t* process_list_head = NULL;
static thread_t* current_thread = NULL;
static process_t* current_process = NULL;

// Idle process and thread
static process_t* idle_process = NULL;
static thread_t* idle_thread = NULL;

// Statistics
static scheduler_stats_t scheduler_statistics = {0};

// Process table (for quick PID lookup)
static process_t* process_table[MAX_PROCESSES] = {NULL};

// Thread table (for quick TID lookup)
static thread_t* thread_table[MAX_PROCESSES * MAX_THREADS_PER_PROCESS] = {NULL};

// Forward declarations
static error_t create_idle_process(void);
static void idle_thread_main(void* arg);
static void cleanup_zombie_processes(void);

error_t process_init(void) {
    if (process_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing process management system");
    
    // Initialize statistics
    memset(&scheduler_statistics, 0, sizeof(scheduler_statistics));
    
    // Initialize process and thread tables
    memset(process_table, 0, sizeof(process_table));
    memset(thread_table, 0, sizeof(thread_table));
    
    // Create idle process
    error_t result = create_idle_process();
    if (result != SUCCESS) {
        KERROR("Failed to create idle process: %d", result);
        return result;
    }
    
    // Set current thread to idle initially
    current_thread = idle_thread;
    current_process = idle_process;
    
    process_initialized = true;
    
    KINFO("Process management system initialized");
    return SUCCESS;
}

void process_shutdown(void) {
    if (!process_initialized) {
        return;
    }
    
    KINFO("Shutting down process management system");
    
    // Terminate all processes
    process_t* process = process_list_head;
    while (process) {
        process_t* next = process->next_process;
        if (process != idle_process) {
            process_exit(process, -1);
        }
        process = next;
    }
    
    // Cleanup idle process
    if (idle_process) {
        process_destroy(idle_process);
        idle_process = NULL;
        idle_thread = NULL;
    }
    
    process_initialized = false;
}

static error_t create_idle_process(void) {
    // Create idle process
    idle_process = process_create("idle", PRIORITY_IDLE, 0, 0);
    if (!idle_process) {
        return E_NOMEM;
    }
    
    // Create idle thread
    idle_thread = thread_create(idle_process, idle_thread_main, NULL, KERNEL_STACK_SIZE);
    if (!idle_thread) {
        process_destroy(idle_process);
        return E_NOMEM;
    }
    
    idle_process->main_thread = idle_thread;
    idle_thread->state = THREAD_STATE_READY;
    
    KDEBUG("Created idle process (PID=%u, TID=%u)", idle_process->pid, idle_thread->tid);
    return SUCCESS;
}

static void idle_thread_main(void* arg) {
    (void)arg;
    
    KDEBUG("Idle thread started");
    
    while (1) {
        // Cleanup zombie processes periodically
        cleanup_zombie_processes();
        
        // Update scheduler statistics
        scheduler_statistics.idle_time++;
        
        // Halt CPU until next interrupt
        hal_halt_cpu();
    }
}

process_t* process_create(const char* name, process_priority_t priority, uid_t uid, gid_t gid) {
    if (!process_initialized) {
        return NULL;
    }
    
    // Find available PID
    pid_t pid = next_pid++;
    if (pid >= MAX_PROCESSES || process_table[pid] != NULL) {
        // Find first available PID
        for (pid = 1; pid < MAX_PROCESSES; pid++) {
            if (process_table[pid] == NULL) {
                break;
            }
        }
        if (pid >= MAX_PROCESSES) {
            KERROR("No available PIDs");
            return NULL;
        }
    }
    
    // Allocate process structure
    process_t* process = (process_t*)memory_alloc(sizeof(process_t));
    if (!process) {
        return NULL;
    }
    
    memset(process, 0, sizeof(process_t));
    
    // Initialize process
    process->pid = pid;
    process->parent_pid = current_process ? current_process->pid : 0;
    process->state = PROCESS_STATE_NEW;
    process->base_priority = priority;
    process->uid = uid;
    process->euid = uid;
    process->gid = gid;
    process->egid = gid;
    process->creation_time = hal_get_timestamp();
    process->thread_count = 0;
    process->memory_usage = 0;
    process->zombie = false;
    process->game_mode_enabled = false;
    
    // Set process name
    if (name) {
        strncpy(process->name, name, sizeof(process->name) - 1);
        process->name[sizeof(process->name) - 1] = '\0';
    } else {
        snprintf(process->name, sizeof(process->name), "process_%u", pid);
    }
    
    // Create address space
    process->address_space = vmm_create_address_space();
    if (!process->address_space) {
        memory_free(process);
        return NULL;
    }
    
    // Create security token
    security_context_t context = (uid == 0) ? SEC_CONTEXT_SYSTEM : SEC_CONTEXT_USER;
    process->security_token = security_create_token(uid, gid, context);
    if (!process->security_token) {
        address_space_destroy(process->address_space);
        memory_free(process);
        return NULL;
    }
    
    // Set resource limits
    process->max_memory = (uid == 0) ? (1ULL << 32) : (512ULL << 20); // 4GB for root, 512MB for users
    process->max_cpu_time = 0; // Unlimited
    process->max_open_files = MAX_OPEN_FILES;
    process->max_threads = MAX_THREADS_PER_PROCESS;
    
    // Initialize file descriptor table
    memset(process->fd_table, 0, sizeof(process->fd_table));
    
    // Set working directory
    strcpy(process->working_directory, "/");
    process->umask = 0022;
    
    // Add to process table and list
    process_table[pid] = process;
    
    if (process_list_head) {
        process_list_head->prev_process = process;
    }
    process->next_process = process_list_head;
    process->prev_process = NULL;
    process_list_head = process;
    
    // Update statistics
    scheduler_statistics.total_processes_created++;
    scheduler_statistics.processes_running++;
    
    KDEBUG("Created process '%s' (PID=%u, UID=%u, GID=%u)", 
           process->name, process->pid, process->uid, process->gid);
    
    return process;
}

error_t process_destroy(process_t* process) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("Destroying process '%s' (PID=%u)", process->name, process->pid);
    
    // Terminate all threads
    thread_t* thread = process->thread_list;
    while (thread) {
        thread_t* next = thread->next_thread;
        thread_destroy(thread);
        thread = next;
    }
    
    // Close all file descriptors
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (process->fd_table[i]) {
            // TODO: Close file descriptor
            process->fd_table[i] = NULL;
        }
    }
    
    // Destroy address space
    if (process->address_space) {
        vmm_destroy_address_space(process->address_space);
    }
    
    // Destroy security token
    if (process->security_token) {
        security_destroy_token(process->security_token);
    }
    
    // Remove from process table
    if (process->pid < MAX_PROCESSES) {
        process_table[process->pid] = NULL;
    }
    
    // Remove from process list
    if (process->prev_process) {
        process->prev_process->next_process = process->next_process;
    } else {
        process_list_head = process->next_process;
    }
    
    if (process->next_process) {
        process->next_process->prev_process = process->prev_process;
    }
    
    // Update statistics
    scheduler_statistics.processes_running--;
    
    // Free process structure
    memory_free(process);
    
    return SUCCESS;
}

error_t process_exit(process_t* process, int exit_code) {
    if (!process) {
        return E_INVAL;
    }
    
    KDEBUG("Process '%s' (PID=%u) exiting with code %d", 
           process->name, process->pid, exit_code);
    
    // Set exit code and mark as zombie
    process->exit_code = exit_code;
    process->zombie = true;
    process->state = PROCESS_STATE_ZOMBIE;
    
    // Terminate all threads except current
    thread_t* thread = process->thread_list;
    while (thread) {
        thread_t* next = thread->next_thread;
        if (thread != current_thread) {
            thread_exit(thread, exit_code);
        }
        thread = next;
    }
    
    // TODO: Notify parent process (send SIGCHLD)
    
    // If this is the current process, schedule another thread
    if (process == current_process) {
        scheduler_yield();
    }
    
    return SUCCESS;
}

thread_t* thread_create(process_t* process, void* entry_point, void* arg, size_t stack_size) {
    if (!process || !entry_point) {
        return NULL;
    }
    
    // Find available TID
    tid_t tid = next_tid++;
    if (tid >= MAX_PROCESSES * MAX_THREADS_PER_PROCESS || thread_table[tid] != NULL) {
        // Find first available TID
        for (tid = 1; tid < MAX_PROCESSES * MAX_THREADS_PER_PROCESS; tid++) {
            if (thread_table[tid] == NULL) {
                break;
            }
        }
        if (tid >= MAX_PROCESSES * MAX_THREADS_PER_PROCESS) {
            KERROR("No available TIDs");
            return NULL;
        }
    }
    
    // Check thread limit
    if (process->thread_count >= process->max_threads) {
        KERROR("Process %u has reached thread limit", process->pid);
        return NULL;
    }
    
    // Allocate thread structure
    thread_t* thread = (thread_t*)memory_alloc(sizeof(thread_t));
    if (!thread) {
        return NULL;
    }
    
    memset(thread, 0, sizeof(thread_t));
    
    // Initialize thread
    thread->tid = tid;
    thread->pid = process->pid;
    thread->state = THREAD_STATE_NEW;
    thread->priority = process->base_priority;
    thread->parent_process = process;
    thread->cpu_affinity = 0xFFFFFFFF; // All CPUs by default
    
    // Allocate kernel stack
    if (stack_size < KERNEL_STACK_SIZE) {
        stack_size = KERNEL_STACK_SIZE;
    }
    
    thread->kernel_stack = memory_alloc(stack_size);
    if (!thread->kernel_stack) {
        memory_free(thread);
        return NULL;
    }
    thread->kernel_stack_size = stack_size;
    
    // Initialize CPU context
    memset(&thread->context, 0, sizeof(thread->context));
    thread->context.rip = (u64)entry_point;
    thread->context.rsp = (u64)thread->kernel_stack + stack_size - 16; // Leave space for alignment
    thread->context.rflags = 0x202; // Interrupts enabled
    thread->context.cs = 0x08; // Kernel code segment
    thread->context.ds = 0x10; // Kernel data segment
    thread->context.es = 0x10;
    thread->context.fs = 0x10;
    thread->context.gs = 0x10;
    thread->context.ss = 0x10;
    
    // Set first argument (in RDI for System V ABI)
    thread->context.rdi = (u64)arg;
    
    // Set time slice based on priority
    switch (thread->priority) {
        case PRIORITY_CRITICAL: thread->time_slice = TIME_SLICE_CRITICAL; break;
        case PRIORITY_HIGH:     thread->time_slice = TIME_SLICE_HIGH; break;
        case PRIORITY_NORMAL:   thread->time_slice = TIME_SLICE_NORMAL; break;
        case PRIORITY_LOW:      thread->time_slice = TIME_SLICE_LOW; break;
        case PRIORITY_IDLE:     thread->time_slice = TIME_SLICE_IDLE; break;
    }
    
    // Create security token (inherit from process)
    thread->security_token = security_create_token(process->uid, process->gid, 
                                                   process->security_token->context);
    if (!thread->security_token) {
        memory_free(thread->kernel_stack);
        memory_free(thread);
        return NULL;
    }
    
    // Add to thread table
    thread_table[tid] = thread;
    
    // Add to process thread list
    if (process->thread_list) {
        process->thread_list->prev_thread = thread;
    }
    thread->next_thread = process->thread_list;
    thread->prev_thread = NULL;
    process->thread_list = thread;
    process->thread_count++;
    
    // Update statistics
    scheduler_statistics.total_threads_created++;
    scheduler_statistics.threads_running++;
    
    KDEBUG("Created thread TID=%u for process PID=%u", thread->tid, process->pid);
    
    return thread;
}

error_t thread_destroy(thread_t* thread) {
    if (!thread) {
        return E_INVAL;
    }
    
    KDEBUG("Destroying thread TID=%u", thread->tid);
    
    // Remove from scheduler if running/ready
    // TODO: scheduler_remove_thread(thread);
    
    // Free kernel stack
    if (thread->kernel_stack) {
        memory_free(thread->kernel_stack);
    }
    
    // Free user stack if allocated
    if (thread->user_stack) {
        // TODO: Free user stack from address space
    }
    
    // Destroy security token
    if (thread->security_token) {
        security_destroy_token(thread->security_token);
    }
    
    // Remove from thread table
    if (thread->tid < MAX_PROCESSES * MAX_THREADS_PER_PROCESS) {
        thread_table[thread->tid] = NULL;
    }
    
    // Remove from process thread list
    if (thread->parent_process) {
        if (thread->prev_thread) {
            thread->prev_thread->next_thread = thread->next_thread;
        } else {
            thread->parent_process->thread_list = thread->next_thread;
        }
        
        if (thread->next_thread) {
            thread->next_thread->prev_thread = thread->prev_thread;
        }
        
        thread->parent_process->thread_count--;
        
        // If this was the main thread, mark process for cleanup
        if (thread == thread->parent_process->main_thread) {
            thread->parent_process->main_thread = NULL;
            if (thread->parent_process->thread_count == 0) {
                thread->parent_process->state = PROCESS_STATE_ZOMBIE;
            }
        }
    }
    
    // Update statistics
    scheduler_statistics.threads_running--;
    
    // Free thread structure
    memory_free(thread);
    
    return SUCCESS;
}

error_t thread_exit(thread_t* thread, int exit_code) {
    if (!thread) {
        return E_INVAL;
    }
    
    KDEBUG("Thread TID=%u exiting with code %d", thread->tid, exit_code);
    
    thread->exit_code = exit_code;
    thread->state = THREAD_STATE_ZOMBIE;
    
    // If this is the current thread, yield to scheduler
    if (thread == current_thread) {
        scheduler_yield();
    }
    
    return SUCCESS;
}

process_t* process_find_by_pid(pid_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}

thread_t* thread_find_by_tid(tid_t tid) {
    if (tid >= MAX_PROCESSES * MAX_THREADS_PER_PROCESS) {
        return NULL;
    }
    return thread_table[tid];
}

thread_t* thread_current(void) {
    return current_thread;
}

static void cleanup_zombie_processes(void) {
    process_t* process = process_list_head;
    
    while (process) {
        process_t* next = process->next_process;
        
        if (process->state == PROCESS_STATE_ZOMBIE && process->thread_count == 0) {
            KDEBUG("Cleaning up zombie process PID=%u", process->pid);
            process_destroy(process);
        }
        
        process = next;
    }
}

// Game Mode support
error_t process_enable_game_mode(process_t* process) {
    if (!process) {
        return E_INVAL;
    }
    
    if (process->game_mode_enabled) {
        return SUCCESS; // Already enabled
    }
    
    KINFO("Enabling Game Mode for process '%s' (PID=%u)", process->name, process->pid);
    
    // Save current priority
    process->saved_priority = process->base_priority;
    
    // Boost to high priority
    process->base_priority = PRIORITY_HIGH;
    process->game_mode_enabled = true;
    process->game_mode_flags = GAME_MODE_HIGH_PRIORITY | GAME_MODE_CPU_BOOST | 
                              GAME_MODE_MEMORY_PRIORITY | GAME_MODE_IO_PRIORITY;
    
    // Apply to all threads
    thread_t* thread = process->thread_list;
    while (thread) {
        thread->priority = PRIORITY_HIGH;
        // TODO: Move thread to high priority queue
        thread = thread->next_thread;
    }
    
    return SUCCESS;
}

error_t process_disable_game_mode(process_t* process) {
    if (!process || !process->game_mode_enabled) {
        return E_INVAL;
    }
    
    KINFO("Disabling Game Mode for process '%s' (PID=%u)", process->name, process->pid);
    
    // Restore original priority
    process->base_priority = process->saved_priority;
    process->game_mode_enabled = false;
    process->game_mode_flags = 0;
    
    // Apply to all threads
    thread_t* thread = process->thread_list;
    while (thread) {
        thread->priority = process->base_priority;
        // TODO: Move thread to appropriate priority queue
        thread = thread->next_thread;
    }
    
    return SUCCESS;
}

bool process_is_game_mode(process_t* process) {
    return process ? process->game_mode_enabled : false;
}

// Statistics and debugging
scheduler_stats_t* scheduler_get_stats(void) {
    return &scheduler_statistics;
}

void process_dump_info(process_t* process) {
    if (!process) {
        return;
    }
    
    hal_debug_print("Process Information:\n");
    hal_debug_print("  PID: %u\n", process->pid);
    hal_debug_print("  Name: %s\n", process->name);
    hal_debug_print("  State: %d\n", process->state);
    hal_debug_print("  Priority: %d\n", process->base_priority);
    hal_debug_print("  UID/GID: %u/%u\n", process->uid, process->gid);
    hal_debug_print("  Thread count: %u\n", process->thread_count);
    hal_debug_print("  Memory usage: %llu KB\n", process->memory_usage / 1024);
    hal_debug_print("  Game Mode: %s\n", process->game_mode_enabled ? "Enabled" : "Disabled");
    hal_debug_print("  Creation time: %llu\n", process->creation_time);
    hal_debug_print("  Total CPU time: %llu ms\n", process->total_cpu_time);
}

void thread_dump_info(thread_t* thread) {
    if (!thread) {
        return;
    }
    
    hal_debug_print("Thread Information:\n");
    hal_debug_print("  TID: %u\n", thread->tid);
    hal_debug_print("  PID: %u\n", thread->pid);
    hal_debug_print("  State: %d\n", thread->state);
    hal_debug_print("  Priority: %d\n", thread->priority);
    hal_debug_print("  Time slice: %llu ms\n", thread->time_slice);
    hal_debug_print("  Total runtime: %llu ms\n", thread->total_runtime);
    hal_debug_print("  Context switches: %llu\n", thread->context_switches);
    hal_debug_print("  Page faults: %llu\n", thread->page_faults);
    hal_debug_print("  System calls: %llu\n", thread->system_calls);
}