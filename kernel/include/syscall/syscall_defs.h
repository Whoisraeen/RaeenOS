#ifndef SYSCALL_DEFS_H
#define SYSCALL_DEFS_H

#include "types.h"

// Syscall argument structures for type safety.
// Every syscall has a corresponding struct, even if it's empty.

#pragma pack(push, 1)

// Process Management
typedef struct { int status; } syscall_exit_t;
typedef struct { const char* pathname; char* const* argv; char* const* envp; } syscall_execve_t;
typedef struct { pid_t pid; int* wstatus; int options; } syscall_waitpid_t;
typedef struct { int which; int who; int priority; } syscall_setpriority_t;

// Scheduling
typedef struct { const struct timespec* req; struct timespec* rem; } syscall_nanosleep_t;

// Memory Management
typedef struct { void* addr; } syscall_brk_t;
typedef struct { void* addr; size_t length; int prot; int flags; int fd; off_t offset; } syscall_mmap_t;
typedef struct { void* addr; size_t length; } syscall_munmap_t;

// File I/O
typedef struct { const char* pathname; int flags; mode_t mode; } syscall_open_t;
typedef struct { int fd; } syscall_close_t;
typedef struct { int fd; void* buf; size_t count; } syscall_read_t;
typedef struct { int fd; const void* buf; size_t count; } syscall_write_t;
typedef struct { int fd; off_t offset; int whence; } syscall_lseek_t;
typedef struct { const char* pathname; struct stat* statbuf; } syscall_stat_t;
typedef struct { int fd; struct stat* statbuf; } syscall_fstat_t;
typedef struct { const char* pathname; int mode; } syscall_access_t;
typedef struct { const char* oldpath; const char* newpath; } syscall_rename_t;
typedef struct { const char* pathname; mode_t mode; } syscall_mkdir_t;

// Advanced Concurrency & I/O
typedef struct { int* uaddr; int futex_op; int val; const struct timespec* timeout; int* uaddr2; int val3; } syscall_futex_t;
typedef struct { u32 entries; struct io_uring_params* params; } syscall_io_uring_setup_t;

// Security
typedef struct { unsigned int operation; unsigned int flags; void* args; } syscall_seccomp_t;

// RaeenOS Specific Extensions
typedef struct { int mode; void* params; } syscall_game_mode_ctl_t;
typedef struct { int operation; void* workload_info; } syscall_ai_accelerate_t;

#pragma pack(pop)

#endif // SYSCALL_DEFS_H