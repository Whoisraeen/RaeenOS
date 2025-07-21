#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "memory/include/memory.h"
#include "security/security.h"
#include <stddef.h>
#include <stdbool.h>

// Process states
typedef enum {
    PROCESS_STATE_NEW,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_ZOMBIE,
    PROCESS_STATE_TERMINATED
} process_state_t;

// Thread states
typedef enum {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_SLEEPING,
    THREAD_STATE_ZOMBIE,
    THREAD_STATE_TERMINATED
} thread_state_t;

// Process priorities (lower number = higher priority)
typedef enum {
    PRIORITY_CRITICAL = 0,   // Kernel critical threads
    PRIORITY_HIGH = 1,       // Game Mode processes
    PRIORITY_NORMAL = 2,     // Regular user processes
    PRIORITY_LOW = 3,        // Background tasks
    PRIORITY_IDLE = 4        // Idle processes
} process_priority_t;

// CPU context for x64
typedef struct cpu_context {
    // General purpose registers
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11;
    u64 r12, r13, r14, r15;
    
    // Control registers
    u64 rip;
    u64 rflags;
    
    // Segment registers
    u16 cs, ds, es, fs, gs, ss;
    
    // Extended state (SSE, AVX, etc.)
    u8 fpu_state[512]; // FXSAVE area
} __attribute__((packed)) cpu_context_t;

// Thread Control Block
typedef struct thread {
    tid_t tid;
    pid_t pid;
    thread_state_t state;
    process_priority_t priority;
    
    // CPU context
    cpu_context_t context;
    void* kernel_stack;
    size_t kernel_stack_size;
    void* user_stack;
    size_t user_stack_size;
    
    // Scheduling
    u64 time_slice;
    u64 time_used;
    u64 total_runtime;
    u64 last_scheduled;
    u32 cpu_affinity;
    
    // Wait queues and synchronization
    struct thread* next_in_queue;
    struct thread* prev_in_queue;
    void* wait_object;
    u64 wake_time;
    
    // Links
    struct thread* next_thread;
    struct thread* prev_thread;
    struct process* parent_process;
    
    // Security context
    security_token_t* security_token;
    
    // Statistics
    u64 context_switches;
    u64 page_faults;
    u64 system_calls;
    
    // Exit information
    int exit_code;
} thread_t;

// Process Control Block
typedef struct process {
    pid_t pid;
    pid_t parent_pid;
    process_state_t state;
    process_priority_t base_priority;
    
    // Memory management
    address_space_t* address_space;
    void* heap_start;
    void* heap_end;
    size_t memory_usage;
    
    // Threads
    thread_t* main_thread;
    thread_t* thread_list;
    u32 thread_count;
    
    // File system
    struct file_descriptor* fd_table[256];
    char working_directory[256];
    mode_t umask;
    
    // Security
    uid_t uid, euid;  // User ID, Effective User ID
    gid_t gid, egid;  // Group ID, Effective Group ID
    security_token_t* security_token;
    
    // Process tree
    struct process* parent;
    struct process* first_child;
    struct process* next_sibling;
    struct process* prev_sibling;
    
    // Process image
    char name[64];
    char command_line[256];
    void* executable_base;
    size_t executable_size;
    
    // Resource limits
    u64 max_memory;
    u64 max_cpu_time;
    u32 max_open_files;
    u32 max_threads;
    
    // Statistics
    u64 creation_time;
    u64 total_cpu_time;
    u64 total_system_time;
    u32 page_faults;
    u32 context_switches;
    
    // Game Mode specific
    bool game_mode_enabled;
    process_priority_t saved_priority;
    u32 game_mode_flags;
    
    // Exit information
    int exit_code;
    bool zombie;
    
    // Links
    struct process* next_process;
    struct process* prev_process;
} process_t;

// Scheduler statistics
typedef struct scheduler_stats {
    u64 total_context_switches;
    u64 total_processes_created;
    u64 total_threads_created;
    u64 processes_running;
    u64 threads_running;
    u64 idle_time;
    u64 system_time;
    u64 user_time;
    u32 load_average_1min;
    u32 load_average_5min;
    u32 load_average_15min;
} scheduler_stats_t;

// Wait queue
typedef struct wait_queue {
    thread_t* head;
    thread_t* tail;
    u32 count;
    char name[32];
} wait_queue_t;

// Core process management functions
error_t process_init(void);
void process_shutdown(void);

// Process creation and destruction
process_t* process_create(const char* name, process_priority_t priority, uid_t uid, gid_t gid);
error_t process_destroy(process_t* process);
error_t process_exit(process_t* process, int exit_code);
error_t process_kill(pid_t pid, int signal);
process_t* process_find_by_pid(pid_t pid);

// Thread management
thread_t* thread_create(process_t* process, void* entry_point, void* arg, size_t stack_size);
error_t thread_destroy(thread_t* thread);
error_t thread_exit(thread_t* thread, int exit_code);
thread_t* thread_find_by_tid(tid_t tid);
thread_t* thread_current(void);

// Scheduler
error_t scheduler_init(void);
void scheduler_start(void);
void scheduler_tick(void);
thread_t* scheduler_next_thread(void);
void scheduler_yield(void);
void scheduler_preempt(void);
error_t scheduler_add_thread(thread_t* thread);
error_t scheduler_remove_thread(thread_t* thread);

// Context switching
void context_switch(thread_t* old_thread, thread_t* new_thread);
void save_cpu_context(cpu_context_t* context);
void restore_cpu_context(cpu_context_t* context);

// Process execution
error_t process_load_executable(process_t* process, const void* executable_data, size_t size);
error_t process_start(process_t* process);
error_t process_exec(process_t* process, const char* path, char* const argv[], char* const envp[]);

// Process synchronization
error_t process_wait(process_t* parent, pid_t child_pid, int* exit_code);
error_t process_waitpid(pid_t pid, int* status, int options);

// Thread synchronization
error_t thread_sleep(u64 milliseconds);
error_t thread_block(wait_queue_t* wait_queue);
error_t thread_wake(thread_t* thread);
error_t thread_wake_all(wait_queue_t* wait_queue);

// Wait queues
wait_queue_t* wait_queue_create(const char* name);
void wait_queue_destroy(wait_queue_t* queue);
error_t wait_queue_add(wait_queue_t* queue, thread_t* thread);
thread_t* wait_queue_remove(wait_queue_t* queue);

// Priority and scheduling policy
error_t process_set_priority(process_t* process, process_priority_t priority);
error_t thread_set_priority(thread_t* thread, process_priority_t priority);
process_priority_t process_get_priority(process_t* process);

// Game Mode support
error_t process_enable_game_mode(process_t* process);
error_t process_disable_game_mode(process_t* process);
bool process_is_game_mode(process_t* process);
error_t game_mode_boost_process(process_t* process);

// CPU affinity
error_t thread_set_affinity(thread_t* thread, u32 cpu_mask);
u32 thread_get_affinity(thread_t* thread);

// Process information
error_t process_get_info(pid_t pid, process_t* info);
error_t process_list(pid_t* pid_list, size_t max_count, size_t* actual_count);

// Resource management
error_t process_set_memory_limit(process_t* process, u64 limit);
error_t process_set_cpu_limit(process_t* process, u64 limit);
u64 process_get_memory_usage(process_t* process);
u64 process_get_cpu_usage(process_t* process);

// Statistics and debugging
scheduler_stats_t* scheduler_get_stats(void);
void scheduler_dump_stats(void);
void process_dump_info(process_t* process);
void thread_dump_info(thread_t* thread);
void scheduler_dump_queues(void);

// Kernel threads
thread_t* kernel_thread_create(void (*function)(void*), void* arg, const char* name);
error_t kernel_thread_stop(thread_t* thread);

// Idle process
extern process_t* idle_process;
extern thread_t* idle_thread;

// Current process and thread
extern thread_t* current_thread;
extern process_t* current_process;

// Process list
extern process_t* process_list_head;

// Next available PIDs and TIDs
extern pid_t next_pid;
extern tid_t next_tid;

// Scheduler ready queues (one per priority level)
extern wait_queue_t* ready_queues[5];

// Game Mode flags
#define GAME_MODE_HIGH_PRIORITY     (1 << 0)
#define GAME_MODE_CPU_BOOST         (1 << 1)
#define GAME_MODE_MEMORY_PRIORITY   (1 << 2)
#define GAME_MODE_IO_PRIORITY       (1 << 3)
#define GAME_MODE_DISABLE_SWAP      (1 << 4)
#define GAME_MODE_EXCLUSIVE_CPU     (1 << 5)

// Time slice constants (in milliseconds)
#define TIME_SLICE_CRITICAL     5
#define TIME_SLICE_HIGH        10
#define TIME_SLICE_NORMAL      20
#define TIME_SLICE_LOW         50
#define TIME_SLICE_IDLE       100

// Stack sizes
#define KERNEL_STACK_SIZE      (16 * 1024)   // 16KB
#define USER_STACK_SIZE        (1024 * 1024) // 1MB

// Process limits
#define MAX_PROCESSES          1024
#define MAX_THREADS_PER_PROCESS 256
#define MAX_OPEN_FILES         256

#endif // PROCESS_H