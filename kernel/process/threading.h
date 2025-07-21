#ifndef RAEENOS_THREADING_H
#define RAEENOS_THREADING_H

#include "process/process.h"
#include "types.h"

// Thread states
typedef enum {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_TERMINATED
} thread_state_t;

// Thread priorities
typedef enum {
    THREAD_PRIORITY_IDLE = 0,
    THREAD_PRIORITY_LOW = 1,
    THREAD_PRIORITY_NORMAL = 2,
    THREAD_PRIORITY_HIGH = 3,
    THREAD_PRIORITY_REALTIME = 4
} thread_priority_t;

// Thread context (x86-64 registers)
typedef struct {
    // General purpose registers
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    
    // Instruction pointer
    uint64_t rip;
    
    // Flags
    uint64_t rflags;
    
    // Segment registers
    uint16_t cs, ds, es, fs, gs, ss;
    
    // FPU/SSE registers (simplified)
    uint8_t fpu_state[512];
} thread_context_t;

// Thread structure
typedef struct thread {
    tid_t tid;                          // Thread ID
    process_t* process;                 // Parent process
    thread_state_t state;               // Current state
    thread_priority_t priority;         // Priority level
    thread_context_t context;           // CPU context
    
    // Entry point and arguments
    uintptr_t entry_point;              // Thread entry point
    void* arg;                          // Thread argument
    
    // Stack information
    void* stack;                        // Stack pointer
    size_t stack_size;                  // Stack size
    
    // Thread-local storage
    void* tls;                          // TLS pointer
    
    // Scheduling information
    uint64_t quantum;                   // Time quantum
    uint64_t last_run;                  // Last run time
    uint64_t total_cpu_time;            // Total CPU time used
    
    // Linked list pointers
    struct thread* next;                // Next thread in global list
    struct thread* prev;                // Previous thread in global list
    struct thread* next_in_process;     // Next thread in process
    struct thread* prev_in_process;     // Previous thread in process
} thread_t;

// Mutex structure
typedef struct mutex {
    mutex_id_t id;                      // Mutex ID
    bool locked;                        // Locked state
    tid_t owner;                        // Owning thread ID
    thread_t* wait_queue;               // Waiting threads
    struct mutex* next;                 // Next mutex in list
} mutex_t;

// Semaphore structure
typedef struct semaphore {
    semaphore_id_t id;                  // Semaphore ID
    int value;                          // Current value
    thread_t* wait_queue;               // Waiting threads
    struct semaphore* next;             // Next semaphore in list
} semaphore_t;

// Condition variable structure
typedef struct condition {
    condition_id_t id;                  // Condition ID
    thread_t* wait_queue;               // Waiting threads
    struct condition* next;             // Next condition in list
} condition_t;

// Type definitions
typedef uint32_t tid_t;                 // Thread ID type
typedef uint32_t mutex_id_t;            // Mutex ID type
typedef uint32_t semaphore_id_t;        // Semaphore ID type
typedef uint32_t condition_id_t;        // Condition ID type

// Thread management functions
error_t threading_init(void);
void threading_shutdown(void);
error_t thread_create(process_t* process, void* entry_point, void* arg, 
                     thread_priority_t priority, thread_t** thread);
error_t thread_destroy(tid_t tid);
thread_t* thread_get_by_tid(tid_t tid);
thread_t* thread_get_current(void);
void thread_switch(thread_t* from, thread_t* to);
void thread_yield(void);
thread_t* thread_get_next_ready(void);
tid_t get_next_thread_id(void);
bool threading_is_initialized(void);

// Thread context functions
error_t thread_setup_context(thread_t* thread);
void thread_save_context(thread_t* thread);
void thread_restore_context(thread_t* thread);

// Thread-local storage functions
void* thread_alloc_tls(void);
void thread_free_tls(void* tls);

// Mutex functions
mutex_id_t mutex_create(void);
error_t mutex_destroy(mutex_id_t id);
error_t mutex_lock(mutex_id_t id);
error_t mutex_unlock(mutex_id_t id);
mutex_t* mutex_get_by_id(mutex_id_t id);

// Semaphore functions
semaphore_id_t semaphore_create(int initial_value);
error_t semaphore_destroy(semaphore_id_t id);
error_t semaphore_wait(semaphore_id_t id);
error_t semaphore_signal(semaphore_id_t id);
semaphore_t* semaphore_get_by_id(semaphore_id_t id);

// Condition variable functions
condition_id_t condition_create(void);
error_t condition_destroy(condition_id_t id);
error_t condition_wait(condition_id_t id, mutex_id_t mutex_id);
error_t condition_signal(condition_id_t id);
error_t condition_broadcast(condition_id_t id);
condition_t* condition_get_by_id(condition_id_t id);

// Thread syscalls (to be implemented in syscall.c)
u64 sys_clone(u64 flags, u64 child_stack, u64 ptid, u64 tls, u64 ctid, u64 arg6);
u64 sys_futex(u64 uaddr, u64 futex_op, u64 val, u64 timeout, u64 uaddr2, u64 val3);

// Thread-related constants
#define CLONE_VM          0x00000100
#define CLONE_FS          0x00000200
#define CLONE_FILES       0x00000400
#define CLONE_SIGHAND     0x00000800
#define CLONE_PIDFD       0x00001000
#define CLONE_PTRACE      0x00002000
#define CLONE_VFORK       0x00004000
#define CLONE_PARENT      0x00008000
#define CLONE_THREAD      0x00010000
#define CLONE_NEWNS       0x00020000
#define CLONE_SYSVSEM     0x00040000
#define CLONE_SETTLS      0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED    0x00400000
#define CLONE_UNTRACED    0x00800000
#define CLONE_CHILD_SETTID 0x01000000
#define CLONE_NEWCGROUP   0x02000000
#define CLONE_NEWUTS      0x04000000
#define CLONE_NEWIPC      0x08000000
#define CLONE_NEWUSER     0x10000000
#define CLONE_NEWPID      0x20000000
#define CLONE_NEWNET      0x40000000
#define CLONE_IO          0x80000000

// Futex operations
#define FUTEX_WAIT        0
#define FUTEX_WAKE        1
#define FUTEX_FD          2
#define FUTEX_REQUEUE     3
#define FUTEX_CMP_REQUEUE 4
#define FUTEX_WAKE_OP     5
#define FUTEX_LOCK_PI     6
#define FUTEX_UNLOCK_PI   7
#define FUTEX_TRYLOCK_PI  8
#define FUTEX_WAIT_BITSET 9
#define FUTEX_WAKE_BITSET 10
#define FUTEX_WAIT_REQUEUE_PI 11
#define FUTEX_CMP_REQUEUE_PI 12

// Thread limits
#define MAX_THREADS_PER_PROCESS 1024
#define MAX_TOTAL_THREADS 8192
#define DEFAULT_THREAD_STACK_SIZE 8192

#endif // RAEENOS_THREADING_H 