#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include <stddef.h>

// Process management constants
#define MAX_PROCESSES 1024
#define MAX_THREADS_PER_PROCESS 256
#define MAX_PRIORITY_LEVELS 32
#define DEFAULT_STACK_SIZE (1024 * 1024)  // 1MB

// Process states
typedef enum {
    PROCESS_STATE_NEW,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_SLEEPING,
    PROCESS_STATE_TERMINATED,
    PROCESS_STATE_ZOMBIE
} process_state_t;

// Process types
typedef enum {
    PROCESS_TYPE_NATIVE,
    PROCESS_TYPE_COMPAT_WINDOWS,
    PROCESS_TYPE_COMPAT_MACOS,
    PROCESS_TYPE_SYSTEM,
    PROCESS_TYPE_SERVICE
} process_type_t;

// Thread states
typedef enum {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_WAITING,
    THREAD_STATE_SLEEPING,
    THREAD_STATE_TERMINATED
} thread_state_t;

// Process priority levels
typedef enum {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 8,
    PRIORITY_NORMAL = 16,
    PRIORITY_HIGH = 24,
    PRIORITY_REALTIME = 31
} priority_t;

// Process flags
#define PROCESS_FLAG_SYSTEM (1 << 0)
#define PROCESS_FLAG_PRIVILEGED (1 << 1)
#define PROCESS_FLAG_SANDBOXED (1 << 2)
#define PROCESS_FLAG_SIGNALED (1 << 3)
#define PROCESS_FLAG_DEBUGGED (1 << 4)

// Thread flags
#define THREAD_FLAG_DETACHED (1 << 0)
#define THREAD_FLAG_SIGNALED (1 << 1)
#define THREAD_FLAG_DEBUGGED (1 << 2)

// Process structure
typedef struct process {
    pid_t pid;                      // Process ID
    pid_t ppid;                     // Parent process ID
    char name[32];                  // Process name
    process_state_t state;          // Current state
    process_type_t type;            // Process type
    priority_t priority;            // Priority level
    u32 flags;                      // Process flags
    
    // Memory management
    virt_addr_t code_start;         // Code segment start
    virt_addr_t code_end;           // Code segment end
    virt_addr_t data_start;         // Data segment start
    virt_addr_t data_end;           // Data segment end
    virt_addr_t stack_start;        // Stack start
    virt_addr_t stack_end;          // Stack end
    virt_addr_t heap_start;         // Heap start
    virt_addr_t heap_end;           // Heap end
    
    // File descriptors
    void** file_descriptors;        // Array of file descriptors
    u32 max_file_descriptors;       // Maximum number of file descriptors
    u32 num_file_descriptors;       // Number of open file descriptors
    
    // Threading
    struct thread* threads;         // Array of threads
    u32 num_threads;                // Number of threads
    u32 max_threads;                // Maximum number of threads
    tid_t main_thread_id;           // Main thread ID
    
    // Scheduling
    u64 cpu_time;                   // CPU time used
    u64 start_time;                 // Process start time
    u64 last_run_time;              // Last time process ran
    u32 quantum;                    // Time quantum
    
    // Resource limits
    u64 memory_limit;               // Memory limit
    u64 cpu_limit;                  // CPU limit
    u32 file_limit;                 // File descriptor limit
    
    // Security
    uid_t uid;                      // User ID
    gid_t gid;                      // Group ID
    u32* capabilities;              // Process capabilities
    void* security_context;         // Security context
    
    // Compatibility layer
    void* compat_context;           // Compatibility context
    
    // Process tree
    struct process* parent;         // Parent process
    struct process* children;       // Child processes
    struct process* next_sibling;   // Next sibling
    struct process* prev_sibling;   // Previous sibling
    
    // Exit information
    int exit_code;                  // Exit code
    u32 exit_signal;                // Exit signal
    
    // Statistics
    u64 memory_usage;               // Current memory usage
    u32 cpu_usage_percent;          // CPU usage percentage
    u32 io_operations;              // I/O operations count
    
} process_t;

// Thread structure
typedef struct thread {
    tid_t tid;                      // Thread ID
    pid_t pid;                      // Process ID
    char name[32];                  // Thread name
    thread_state_t state;           // Current state
    priority_t priority;            // Priority level
    u32 flags;                      // Thread flags
    
    // Execution context
    void* stack;                    // Thread stack
    size_t stack_size;              // Stack size
    void* entry_point;              // Thread entry point
    void* arg;                      // Thread argument
    
    // CPU context
    void* cpu_context;              // CPU registers context
    size_t context_size;            // Context size
    
    // Scheduling
    u64 cpu_time;                   // CPU time used
    u64 start_time;                 // Thread start time
    u64 last_run_time;              // Last time thread ran
    u32 quantum;                    // Time quantum
    
    // Thread-specific data
    void* tls_data;                 // Thread-local storage
    size_t tls_size;                // TLS size
    
    // Signal handling
    void* signal_handlers;          // Signal handlers
    u32 pending_signals;            // Pending signals
    
    // Thread list
    struct thread* next;            // Next thread in process
    struct thread* prev;            // Previous thread in process
    
} thread_t;

// CPU context structure (architecture-specific)
#ifdef __x86_64__
typedef struct {
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11, r12, r13, r14, r15;
    u64 rip, rflags;
    u64 cs, ds, es, fs, gs, ss;
} cpu_context_t;
#else
typedef struct {
    u32 eax, ebx, ecx, edx;
    u32 esi, edi, ebp, esp;
    u32 eip, eflags;
    u32 cs, ds, es, fs, gs, ss;
} cpu_context_t;
#endif

// Scheduler structure
typedef struct scheduler {
    process_t* current_process;     // Currently running process
    thread_t* current_thread;       // Currently running thread
    
    // Priority queues
    struct process** ready_queues;  // Ready process queues
    struct process** waiting_queues; // Waiting process queues
    
    // Statistics
    u64 total_processes;            // Total processes created
    u64 total_threads;              // Total threads created
    u64 context_switches;           // Number of context switches
    u64 idle_time;                  // Total idle time
    
    // Configuration
    u32 time_slice;                 // Default time slice
    u32 max_priority;               // Maximum priority level
    bool preemptive;                // Preemptive scheduling
    
} scheduler_t;

// Process management functions
error_t process_init(void);
void process_shutdown(void);

// Helper functions
tid_t get_next_thread_id(void);

// Process creation and destruction
error_t process_create(const char* name, void* entry_point, void* arg, 
                      process_type_t type, priority_t priority, process_t** process);
error_t process_destroy(pid_t pid);
error_t process_terminate(pid_t pid, int exit_code);
error_t process_wait(pid_t pid, int* exit_code);

// Process information
process_t* process_get_by_pid(pid_t pid);
process_t* process_get_current(void);
error_t process_get_info(pid_t pid, process_t* info);
error_t process_get_list(process_t* processes, u32* count);

// Process control
error_t process_suspend(pid_t pid);
error_t process_resume(pid_t pid);
error_t process_set_priority(pid_t pid, priority_t priority);
error_t process_set_affinity(pid_t pid, u32 cpu_mask);

// Thread management
error_t thread_create(pid_t pid, const char* name, void* entry_point, 
                     void* arg, priority_t priority, tid_t* tid);
error_t thread_destroy(tid_t tid);
error_t thread_terminate(tid_t tid);
error_t thread_join(tid_t tid, void** result);

// Thread information
thread_t* thread_get_by_tid(tid_t tid);
thread_t* thread_get_current(void);
error_t thread_get_info(tid_t tid, thread_t* info);

// Thread control
error_t thread_suspend(tid_t tid);
error_t thread_resume(tid_t tid);
error_t thread_set_priority(tid_t tid, priority_t priority);
error_t thread_yield(void);

// Scheduler functions
error_t scheduler_init(void);
void scheduler_shutdown(void);
void scheduler_schedule(void);
void scheduler_yield(void);
void scheduler_tick(void);
void schedule_next_process(void);

// Context switching
void context_switch(thread_t* from, thread_t* to);
void context_save(cpu_context_t* context);
void context_restore(cpu_context_t* context);

// Process tree management
error_t process_fork(process_t* parent, process_t** child);
error_t process_exec(const char* path, char* const argv[], char* const envp[]);
error_t process_exit(int exit_code);

// Signal handling
error_t process_send_signal(pid_t pid, u32 signal);
error_t process_set_signal_handler(pid_t pid, u32 signal, void* handler);
error_t process_handle_signal(process_t* process, u32 signal);

// Resource management
error_t process_set_memory_limit(pid_t pid, u64 limit);
error_t process_set_cpu_limit(pid_t pid, u64 limit);
error_t process_set_file_limit(pid_t pid, u32 limit);

// Process debugging
void process_dump_info(process_t* process);
void process_dump_all(void);
void process_validate_integrity(void);

// Architecture-specific functions
#ifdef __x86_64__
error_t process_init_x64(void);
void context_switch_x64(thread_t* from, thread_t* to);
#else
error_t process_init_x86(void);
void context_switch_x86(thread_t* from, thread_t* to);
#endif

// Process management macros
#define PROCESS_CREATE(name, entry, arg, type, priority, process) \
    process_create(name, entry, arg, type, priority, process)

#define PROCESS_DESTROY(pid) process_destroy(pid)
#define PROCESS_TERMINATE(pid, code) process_terminate(pid, code)

#define THREAD_CREATE(pid, name, entry, arg, priority, tid) \
    thread_create(pid, name, entry, arg, priority, tid)

#define THREAD_DESTROY(tid) thread_destroy(tid)
#define THREAD_TERMINATE(tid) thread_terminate(tid)

#endif // PROCESS_H 