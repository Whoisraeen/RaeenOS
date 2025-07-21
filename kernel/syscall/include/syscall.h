#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../core/include/types.h"
#include "../../core/include/error.h"
#include "../../core/include/interrupts.h"
#include "../../process/include/process.h"

// Production System Call Interface for RaeenOS
// Complete POSIX-compatible syscalls with RaeenOS extensions

// System call numbers (Linux-compatible for easy porting)
#define SYS_read                0
#define SYS_write               1
#define SYS_open                2
#define SYS_close               3
#define SYS_stat                4
#define SYS_fstat               5
#define SYS_lstat               6
#define SYS_poll                7
#define SYS_lseek               8
#define SYS_mmap                9
#define SYS_mprotect            10
#define SYS_munmap              11
#define SYS_brk                 12
#define SYS_rt_sigaction        13
#define SYS_rt_sigprocmask      14
#define SYS_rt_sigreturn        15
#define SYS_ioctl               16
#define SYS_pread64             17
#define SYS_pwrite64            18
#define SYS_readv               19
#define SYS_writev              20
#define SYS_access              21
#define SYS_pipe                22
#define SYS_select              23
#define SYS_sched_yield         24
#define SYS_mremap              25
#define SYS_msync               26
#define SYS_mincore             27
#define SYS_madvise             28
#define SYS_shmget              29
#define SYS_shmat               30
#define SYS_shmctl              31
#define SYS_dup                 32
#define SYS_dup2                33
#define SYS_pause               34
#define SYS_nanosleep           35
#define SYS_getitimer           36
#define SYS_alarm               37
#define SYS_setitimer           38
#define SYS_getpid              39
#define SYS_sendfile            40
#define SYS_socket              41
#define SYS_connect             42
#define SYS_accept              43
#define SYS_sendto              44
#define SYS_recvfrom            45
#define SYS_sendmsg             46
#define SYS_recvmsg             47
#define SYS_shutdown            48
#define SYS_bind                49
#define SYS_listen              50
#define SYS_getsockname         51
#define SYS_getpeername         52
#define SYS_socketpair          53
#define SYS_setsockopt          54
#define SYS_getsockopt          55
#define SYS_clone               56
#define SYS_fork                57
#define SYS_vfork               58
#define SYS_execve              59
#define SYS_exit                60
#define SYS_wait4               61
#define SYS_kill                62
#define SYS_uname               63
#define SYS_semget              64
#define SYS_semop               65
#define SYS_semctl              66
#define SYS_shmdt               67
#define SYS_msgget              68
#define SYS_msgsnd              69
#define SYS_msgrcv              70
#define SYS_msgctl              71
#define SYS_fcntl               72
#define SYS_flock               73
#define SYS_fsync               74
#define SYS_fdatasync           75
#define SYS_truncate            76
#define SYS_ftruncate           77
#define SYS_getdents            78
#define SYS_getcwd              79
#define SYS_chdir               80
#define SYS_fchdir              81
#define SYS_rename              82
#define SYS_mkdir               83
#define SYS_rmdir               84
#define SYS_creat               85
#define SYS_link                86
#define SYS_unlink              87
#define SYS_symlink             88
#define SYS_readlink            89
#define SYS_chmod               90
#define SYS_fchmod              91
#define SYS_chown               92
#define SYS_fchown              93
#define SYS_lchown              94
#define SYS_umask               95
#define SYS_gettimeofday        96
#define SYS_getrlimit           97
#define SYS_getrusage           98
#define SYS_sysinfo             99
#define SYS_times               100
#define SYS_ptrace              101
#define SYS_getuid              102
#define SYS_syslog              103
#define SYS_getgid              104
#define SYS_setuid              105
#define SYS_setgid              106
#define SYS_geteuid             107
#define SYS_getegid             108
#define SYS_setpgid             109
#define SYS_getppid             110
#define SYS_getpgrp             111
#define SYS_setsid              112
#define SYS_setreuid            113
#define SYS_setregid            114
#define SYS_getgroups           115
#define SYS_setgroups           116
#define SYS_setresuid           117
#define SYS_getresuid           118
#define SYS_setresgid           119
#define SYS_getresgid           120

// RaeenOS-specific system calls (starting from 1000)
#define SYS_raeen_game_mode     1000
#define SYS_raeen_set_priority  1001
#define SYS_raeen_get_stats     1002
#define SYS_raeen_memory_info   1003
#define SYS_raeen_process_info  1004
#define SYS_raeen_device_info   1005
#define SYS_raeen_security_set  1006
#define SYS_raeen_ai_optimize   1007
#define SYS_raeen_theme_set     1008
#define SYS_raeen_compatibility 1009

// Maximum system call number
#define MAX_SYSCALLS            1010

// System call error codes
#define SYSCALL_SUCCESS         0
#define SYSCALL_ERROR          -1

// System call argument structure
typedef struct {
    u64 rdi;    // arg1
    u64 rsi;    // arg2
    u64 rdx;    // arg3
    u64 r10;    // arg4
    u64 r8;     // arg5
    u64 r9;     // arg6
} syscall_args_t;

// System call handler function type
typedef long (*syscall_handler_t)(syscall_args_t* args);

// System call information structure
typedef struct {
    const char* name;
    syscall_handler_t handler;
    u32 arg_count;
    bool requires_permission;
    u32 required_permission;
} syscall_info_t;

// System call statistics
typedef struct {
    u64 total_syscalls;
    u64 successful_syscalls;
    u64 failed_syscalls;
    u64 syscall_counts[MAX_SYSCALLS];
    u64 avg_execution_time_ns;
    u64 max_execution_time_ns;
    u32 most_used_syscall;
} syscall_stats_t;

// Function prototypes

// System call initialization
error_t syscall_init(void);
void syscall_shutdown(void);

// System call registration
error_t syscall_register(u32 syscall_num, const char* name, syscall_handler_t handler, 
                         u32 arg_count, bool requires_permission, u32 permission);
error_t syscall_unregister(u32 syscall_num);

// Main system call handler (called from interrupt handler)
void syscall_handler(interrupt_context_t* context);

// System call implementations

// File system operations
long sys_read(syscall_args_t* args);
long sys_write(syscall_args_t* args);
long sys_open(syscall_args_t* args);
long sys_close(syscall_args_t* args);
long sys_stat(syscall_args_t* args);
long sys_lseek(syscall_args_t* args);
long sys_access(syscall_args_t* args);
long sys_mkdir(syscall_args_t* args);
long sys_rmdir(syscall_args_t* args);
long sys_unlink(syscall_args_t* args);
long sys_chmod(syscall_args_t* args);
long sys_chown(syscall_args_t* args);
long sys_getcwd(syscall_args_t* args);
long sys_chdir(syscall_args_t* args);

// Process management
long sys_fork(syscall_args_t* args);
long sys_execve(syscall_args_t* args);
long sys_exit(syscall_args_t* args);
long sys_wait4(syscall_args_t* args);
long sys_getpid(syscall_args_t* args);
long sys_getppid(syscall_args_t* args);
long sys_kill(syscall_args_t* args);
long sys_sched_yield(syscall_args_t* args);

// Memory management
long sys_mmap(syscall_args_t* args);
long sys_munmap(syscall_args_t* args);
long sys_mprotect(syscall_args_t* args);
long sys_brk(syscall_args_t* args);

// User/Group management
long sys_getuid(syscall_args_t* args);
long sys_getgid(syscall_args_t* args);
long sys_geteuid(syscall_args_t* args);
long sys_getegid(syscall_args_t* args);
long sys_setuid(syscall_args_t* args);
long sys_setgid(syscall_args_t* args);

// Time operations
long sys_gettimeofday(syscall_args_t* args);
long sys_nanosleep(syscall_args_t* args);
long sys_time(syscall_args_t* args);

// System information
long sys_uname(syscall_args_t* args);
long sys_sysinfo(syscall_args_t* args);
long sys_getrusage(syscall_args_t* args);

// RaeenOS-specific system calls
long sys_raeen_game_mode(syscall_args_t* args);
long sys_raeen_set_priority(syscall_args_t* args);
long sys_raeen_get_stats(syscall_args_t* args);
long sys_raeen_memory_info(syscall_args_t* args);
long sys_raeen_process_info(syscall_args_t* args);
long sys_raeen_device_info(syscall_args_t* args);
long sys_raeen_security_set(syscall_args_t* args);
long sys_raeen_ai_optimize(syscall_args_t* args);
long sys_raeen_theme_set(syscall_args_t* args);
long sys_raeen_compatibility(syscall_args_t* args);

// Utility functions
bool syscall_is_valid(u32 syscall_num);
const char* syscall_get_name(u32 syscall_num);
syscall_stats_t* syscall_get_stats(void);
void syscall_dump_stats(void);

// Permission checking
bool syscall_has_permission(process_t* process, u32 permission);
error_t syscall_check_permission(process_t* process, u32 syscall_num);

// Memory validation for system calls
bool syscall_validate_user_pointer(const void* ptr, size_t size, bool write);
bool syscall_validate_user_string(const char* str, size_t max_len);

// Global variables (extern)
extern syscall_info_t syscall_table[MAX_SYSCALLS];
extern syscall_stats_t syscall_statistics;
extern bool syscall_initialized;

// Constants
#define SYSCALL_MAX_ARGS        6
#define SYSCALL_MAX_STRING_LEN  4096
#define SYSCALL_STACK_SIZE      (16 * 1024)    // 16KB

// Permissions
#define PERM_NONE               0x0000
#define PERM_FILE_READ          0x0001
#define PERM_FILE_WRITE         0x0002
#define PERM_FILE_EXECUTE       0x0004
#define PERM_PROCESS_CREATE     0x0008
#define PERM_PROCESS_KILL       0x0010
#define PERM_MEMORY_MANAGE      0x0020
#define PERM_SYSTEM_ADMIN       0x0040
#define PERM_NETWORK_ACCESS     0x0080
#define PERM_DEVICE_ACCESS      0x0100
#define PERM_KERNEL_ACCESS      0x0200

#endif // SYSCALL_H