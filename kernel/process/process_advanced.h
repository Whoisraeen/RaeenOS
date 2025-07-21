#ifndef RAEEEN_PROCESS_ADVANCED_H
#define RAEEEN_PROCESS_ADVANCED_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"
#include "process.h"
#include "vm.h"

// Process states
#define PROCESS_STATE_NEW          0
#define PROCESS_STATE_READY        1
#define PROCESS_STATE_RUNNING      2
#define PROCESS_STATE_WAITING      3
#define PROCESS_STATE_SLEEPING     4
#define PROCESS_STATE_STOPPED      5
#define PROCESS_STATE_ZOMBIE       6
#define PROCESS_STATE_DEAD         7

// Process priorities
#define PRIORITY_IDLE              0
#define PRIORITY_LOW               1
#define PRIORITY_NORMAL            2
#define PRIORITY_HIGH              3
#define PRIORITY_REALTIME          4
#define PRIORITY_CRITICAL          5

// Scheduling policies
#define SCHED_POLICY_NORMAL        0
#define SCHED_POLICY_FIFO          1
#define SCHED_POLICY_RR            2
#define SCHED_POLICY_BATCH         3
#define SCHED_POLICY_IDLE          4
#define SCHED_POLICY_DEADLINE      5

// User-level thread states
#define THREAD_STATE_NEW           0
#define THREAD_STATE_READY         1
#define THREAD_STATE_RUNNING       2
#define THREAD_STATE_WAITING       3
#define THREAD_STATE_SLEEPING      4
#define THREAD_STATE_TERMINATED    5

// IPC mechanisms
#define IPC_TYPE_PIPE             0x1
#define IPC_TYPE_MSGQUEUE         0x2
#define IPC_TYPE_SHMEM            0x4
#define IPC_TYPE_SEMAPHORE        0x8
#define IPC_TYPE_SOCKET           0x10

// Signal numbers
#define SIGABRT                   6
#define SIGALRM                   14
#define SIGBUS                    7
#define SIGCHLD                   17
#define SIGCONT                   18
#define SIGFPE                    8
#define SIGHUP                    1
#define SIGILL                    4
#define SIGINT                    2
#define SIGKILL                   9
#define SIGPIPE                   13
#define SIGQUIT                   3
#define SIGSEGV                   11
#define SIGSTOP                   19
#define SIGTERM                   15
#define SIGTSTP                   20
#define SIGTTIN                   21
#define SIGTTOU                   22
#define SIGUSR1                   10
#define SIGUSR2                   12
#define MAX_SIGNALS               32

// Signal actions
#define SIG_DFL                   0
#define SIG_IGN                   1
#define SIG_ERR                   -1

// User-level thread structure
typedef struct user_thread {
    tid_t tid;
    uintptr_t stack;
    size_t stack_size;
    void *(*start_routine)(void *);
    void *arg;
    void *retval;
    int state;
    int priority;
    int policy;
    uint64_t cpu_time;
    uint64_t user_time;
    uint64_t system_time;
    struct user_thread *next;
    struct user_thread *prev;
    process_t *process;
    vm_space_t *vm_space;
    void *tls;  // Thread-local storage
    bool detached;
    bool canceled;
    void *cleanup_handlers;
} user_thread_t;

// Process group structure
typedef struct process_group {
    pid_t pgid;
    process_t *leader;
    process_t *processes;
    int ref_count;
    spinlock_t lock;
} process_group_t;

// Session structure
typedef struct session {
    pid_t sid;
    process_t *leader;
    process_group_t *groups;
    int ref_count;
    spinlock_t lock;
} session_t;

// IPC Pipe structure
typedef struct ipc_pipe {
    int id;
    char *buffer;
    size_t size;
    size_t read_pos;
    size_t write_pos;
    int readers;
    int writers;
    spinlock_t lock;
    wait_queue_t read_queue;
    wait_queue_t write_queue;
    bool named;
    char name[256];
} ipc_pipe_t;

// IPC Message Queue structure
typedef struct ipc_msg {
    long type;
    size_t size;
    char data[1024];
} ipc_msg_t;

typedef struct ipc_msgqueue {
    int id;
    ipc_msg_t *messages;
    size_t max_messages;
    size_t message_size;
    size_t read_pos;
    size_t write_pos;
    size_t count;
    spinlock_t lock;
    wait_queue_t read_queue;
    wait_queue_t write_queue;
} ipc_msgqueue_t;

// IPC Shared Memory structure
typedef struct ipc_shmem {
    int id;
    void *addr;
    size_t size;
    int ref_count;
    spinlock_t lock;
    process_t **mapped_processes;
    int num_processes;
} ipc_shmem_t;

// IPC Semaphore structure
typedef struct ipc_semaphore {
    int id;
    int value;
    int max_value;
    spinlock_t lock;
    wait_queue_t wait_queue;
} ipc_semaphore_t;

// Signal handler structure
typedef struct sigaction {
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t sa_mask;
    int sa_flags;
} sigaction_t;

// Signal context structure
typedef struct signal_context {
    int signo;
    siginfo_t info;
    void *context;
    uintptr_t old_sp;
    uintptr_t old_ip;
    uintptr_t old_flags;
} signal_context_t;

// Process limits structure
typedef struct rlimit {
    uint64_t rlim_cur;
    uint64_t rlim_max;
} rlimit_t;

// Process resource usage structure
typedef struct rusage {
    struct timeval ru_utime;
    struct timeval ru_stime;
    uint64_t ru_maxrss;
    uint64_t ru_ixrss;
    uint64_t ru_idrss;
    uint64_t ru_isrss;
    uint64_t ru_minflt;
    uint64_t ru_majflt;
    uint64_t ru_nswap;
    uint64_t ru_inblock;
    uint64_t ru_oublock;
    uint64_t ru_msgsnd;
    uint64_t ru_msgrcv;
    uint64_t ru_nsignals;
    uint64_t ru_nvcsw;
    uint64_t ru_nivcsw;
} rusage_t;

// Advanced process structure (extends basic process_t)
typedef struct process_advanced {
    process_t base;
    
    // User-level threading
    user_thread_t *threads;
    user_thread_t *current_thread;
    int thread_count;
    tid_t next_tid;
    
    // Process hierarchy
    process_t *parent;
    process_t *children;
    process_t *siblings;
    process_group_t *group;
    session_t *session;
    
    // IPC resources
    ipc_pipe_t **pipes;
    ipc_msgqueue_t **msgqueues;
    ipc_shmem_t **shmems;
    ipc_semaphore_t **semaphores;
    int num_pipes;
    int num_msgqueues;
    int num_shmems;
    int num_semaphores;
    
    // Signal handling
    sigaction_t signal_handlers[MAX_SIGNALS];
    sigset_t signal_mask;
    sigset_t pending_signals;
    signal_context_t *signal_context;
    bool in_signal_handler;
    
    // Resource limits
    rlimit_t limits[RLIMIT_NLIMITS];
    rusage_t usage;
    
    // Scheduling
    int priority;
    int policy;
    struct sched_param sched_param;
    uint64_t time_slice;
    uint64_t last_run;
    uint64_t total_cpu_time;
    
    // Memory management
    vm_space_t *vm_space;
    uintptr_t brk;
    uintptr_t stack_top;
    uintptr_t stack_bottom;
    
    // File descriptors
    int *file_descriptors;
    int max_files;
    int num_files;
    
    // Working directory
    char *cwd;
    size_t cwd_size;
    
    // Environment
    char **envp;
    int env_count;
    
    // Arguments
    char **argv;
    int argc;
    
    // Exit information
    int exit_code;
    int exit_signal;
    bool exited;
    
    // Security
    uid_t uid;
    gid_t gid;
    uid_t euid;
    gid_t egid;
    uid_t suid;
    gid_t sgid;
    
    // Capabilities
    uint64_t effective_caps;
    uint64_t permitted_caps;
    uint64_t inheritable_caps;
    
    // Namespaces
    int pid_namespace;
    int mount_namespace;
    int network_namespace;
    int user_namespace;
    int ipc_namespace;
    int uts_namespace;
    
    // CPU affinity
    cpu_set_t cpu_affinity;
    int cpu_id;
    
    // Real-time
    struct timespec deadline;
    uint64_t period;
    uint64_t execution_time;
    
    // Debugging
    bool traced;
    process_t *tracer;
    int ptrace_options;
    
    // Performance monitoring
    uint64_t page_faults;
    uint64_t context_switches;
    uint64_t voluntary_switches;
    uint64_t involuntary_switches;
    
} process_advanced_t;

// Function prototypes

// Advanced process creation and management
process_advanced_t *process_create_advanced(const char *name, int priority);
int process_load_executable(process_advanced_t *proc, const char *path, char *const argv[], char *const envp[]);
int process_setup_user_space(process_advanced_t *proc);
int process_setup_stack(process_advanced_t *proc);
int process_setup_heap(process_advanced_t *proc);
void process_destroy_advanced(process_advanced_t *proc);

// User-level threading
user_thread_t *thread_create(process_advanced_t *proc, void *(*start_routine)(void *), void *arg);
int thread_join(user_thread_t *thread, void **retval);
int thread_detach(user_thread_t *thread);
int thread_cancel(user_thread_t *thread);
void thread_exit(void *retval);
int thread_set_priority(user_thread_t *thread, int priority);
int thread_set_policy(user_thread_t *thread, int policy);
void thread_yield(void);

// Process hierarchy management
int process_fork_advanced(process_advanced_t *parent, process_advanced_t **child);
int process_exec_advanced(process_advanced_t *proc, const char *path, char *const argv[], char *const envp[]);
int process_wait_advanced(pid_t pid, int *wstatus, int options);
int process_kill_advanced(pid_t pid, int sig);
int process_setpgid(pid_t pid, pid_t pgid);
int process_setsid(void);
int process_getsid(pid_t pid);

// IPC mechanisms
int ipc_pipe_create(ipc_pipe_t **pipe, size_t size);
int ipc_pipe_destroy(ipc_pipe_t *pipe);
int ipc_pipe_read(ipc_pipe_t *pipe, void *buf, size_t count);
int ipc_pipe_write(ipc_pipe_t *pipe, const void *buf, size_t count);
int ipc_pipe_named_create(const char *name, ipc_pipe_t **pipe);
int ipc_pipe_named_open(const char *name, ipc_pipe_t **pipe);

int ipc_msgqueue_create(ipc_msgqueue_t **queue, size_t max_messages, size_t message_size);
int ipc_msgqueue_destroy(ipc_msgqueue_t *queue);
int ipc_msgqueue_send(ipc_msgqueue_t *queue, const ipc_msg_t *msg, int flags);
int ipc_msgqueue_receive(ipc_msgqueue_t *queue, ipc_msg_t *msg, int flags);

int ipc_shmem_create(ipc_shmem_t **shm, size_t size);
int ipc_shmem_destroy(ipc_shmem_t *shm);
int ipc_shmem_attach(ipc_shmem_t *shm, process_advanced_t *proc, void **addr);
int ipc_shmem_detach(ipc_shmem_t *shm, process_advanced_t *proc);

int ipc_semaphore_create(ipc_semaphore_t **sem, int value, int max_value);
int ipc_semaphore_destroy(ipc_semaphore_t *sem);
int ipc_semaphore_wait(ipc_semaphore_t *sem);
int ipc_semaphore_post(ipc_semaphore_t *sem);
int ipc_semaphore_trywait(ipc_semaphore_t *sem);

// Signal handling
int signal_register_handler(process_advanced_t *proc, int sig, const sigaction_t *act, sigaction_t *oact);
int signal_send(process_advanced_t *proc, int sig);
int signal_block(process_advanced_t *proc, const sigset_t *set, sigset_t *oldset);
int signal_unblock(process_advanced_t *proc, const sigset_t *set, sigset_t *oldset);
int signal_suspend(process_advanced_t *proc, const sigset_t *mask);
void signal_deliver(process_advanced_t *proc, int sig);
int signal_setup_handler(process_advanced_t *proc, signal_context_t *context);

// Resource management
int process_set_rlimit(process_advanced_t *proc, int resource, const rlimit_t *rlim);
int process_get_rlimit(process_advanced_t *proc, int resource, rlimit_t *rlim);
int process_get_rusage(process_advanced_t *proc, rusage_t *usage);
int process_set_priority(process_advanced_t *proc, int priority);
int process_get_priority(process_advanced_t *proc);

// Security and permissions
int process_setuid(process_advanced_t *proc, uid_t uid);
int process_setgid(process_advanced_t *proc, gid_t gid);
int process_seteuid(process_advanced_t *proc, uid_t euid);
int process_setegid(process_advanced_t *proc, gid_t egid);
bool process_has_permission(process_advanced_t *proc, int permission);

// SMP support
int process_set_cpu_affinity(process_advanced_t *proc, const cpu_set_t *set);
int process_get_cpu_affinity(process_advanced_t *proc, cpu_set_t *set);
void process_migrate_to_cpu(process_advanced_t *proc, int cpu_id);
int process_load_balance(void);

// Real-time scheduling
int process_set_deadline(process_advanced_t *proc, const struct timespec *deadline, uint64_t period);
int process_get_deadline(process_advanced_t *proc, struct timespec *deadline, uint64_t *period);
bool process_missed_deadline(process_advanced_t *proc);

// Debugging and tracing
int process_ptrace_attach(process_advanced_t *tracer, process_advanced_t *tracee);
int process_ptrace_detach(process_advanced_t *tracee);
int process_ptrace_read(process_advanced_t *tracee, uintptr_t addr, void *data, size_t size);
int process_ptrace_write(process_advanced_t *tracee, uintptr_t addr, const void *data, size_t size);
int process_ptrace_continue(process_advanced_t *tracee, int sig);
int process_ptrace_singlestep(process_advanced_t *tracee);

// Performance monitoring
void process_record_page_fault(process_advanced_t *proc);
void process_record_context_switch(process_advanced_t *proc, bool voluntary);
void process_get_performance_stats(process_advanced_t *proc, rusage_t *usage);

// Process lifecycle
int process_suspend(process_advanced_t *proc);
int process_resume(process_advanced_t *proc);
int process_stop(process_advanced_t *proc);
int process_continue(process_advanced_t *proc);
void process_cleanup_zombies(void);

// Process groups and sessions
process_group_t *process_group_create(pid_t pgid);
void process_group_destroy(process_group_t *group);
int process_group_add(process_group_t *group, process_advanced_t *proc);
int process_group_remove(process_group_t *group, process_advanced_t *proc);

session_t *session_create(pid_t sid);
void session_destroy(session_t *session);
int session_add_group(session_t *session, process_group_t *group);
int session_remove_group(session_t *session, process_group_t *group);

// Utility functions
process_advanced_t *process_find_by_pid(pid_t pid);
process_advanced_t *process_find_by_name(const char *name);
void process_list_all(void);
void process_dump_info(process_advanced_t *proc);

#endif // RAEEEN_PROCESS_ADVANCED_H 