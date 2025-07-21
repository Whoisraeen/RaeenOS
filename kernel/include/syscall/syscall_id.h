#ifndef SYSCALL_ID_H
#define SYSCALL_ID_H

// System call numbers for RaeenOS.
// This list is cleaned up to remove redundancies and add modern features.
typedef enum {
    // Process Management
    SYS_EXIT = 0,
    SYS_FORK = 1,
    SYS_EXECVE = 2,
    SYS_WAITPID = 3,
    SYS_GETPID = 4,
    SYS_GETPPID = 5,
    SYS_GETUID = 6,
    SYS_SETUID = 7,
    SYS_SETPRIORITY = 8,

    // Scheduling
    SYS_SCHED_YIELD = 10,
    SYS_NANOSLEEP = 11,

    // Memory Management
    SYS_BRK = 20,
    SYS_MMAP = 21,
    SYS_MUNMAP = 22,

    // File I/O
    SYS_OPEN = 30,
    SYS_CLOSE = 31,
    SYS_READ = 32,
    SYS_WRITE = 33,
    SYS_LSEEK = 34,
    SYS_STAT = 35,
    SYS_FSTAT = 36,
    SYS_ACCESS = 37,
    SYS_RENAME = 38,
    SYS_MKDIR = 39,

    // Advanced Concurrency & I/O
    SYS_FUTEX = 50,         // Fast user-space mutexes
    SYS_IO_URING_SETUP = 51, // Asynchronous I/O

    // Security
    SYS_SECCOMP = 60,       // Syscall filtering for sandboxing

    // RaeenOS Specific Extensions
    SYS_GAME_MODE_CTL = 100, // Game mode resource prioritization
    SYS_AI_ACCELERATE = 101, // Hint for AI workload acceleration

    MAX_SYSCALLS
} syscall_id_t;

#endif // SYSCALL_ID_H