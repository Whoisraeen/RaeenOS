#ifndef RAEENOS_SYSCALL_H
#define RAEENOS_SYSCALL_H

#include <stdint.h>
#include "process.h"

// Syscall numbers
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_GETPID  3
#define SYS_READ    4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_FORK    7
#define SYS_EXEC    8
#define SYS_WAIT    9
#define SYS_BRK     10
#define SYS_MMAP    11
#define SYS_MUNMAP  12
#define SYS_GETTIMEOFDAY 13
#define SYS_SLEEP   14

// Syscall handler function type
typedef int64_t (*syscall_handler_t)(process_t* process, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

// Syscall system functions
void syscall_init(void);
void syscall_handler(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t syscall_register(int syscall_number, syscall_handler_t handler);

// Individual syscall implementations
int64_t sys_exit(process_t* process, uint64_t exit_code, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_write(process_t* process, uint64_t fd, uint64_t buf, uint64_t count, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_getpid(process_t* process, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_read(process_t* process, uint64_t fd, uint64_t buf, uint64_t count, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_open(process_t* process, uint64_t pathname, uint64_t flags, uint64_t mode, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_close(process_t* process, uint64_t fd, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_fork(process_t* process, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_exec(process_t* process, uint64_t pathname, uint64_t argv, uint64_t envp, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_wait(process_t* process, uint64_t pid, uint64_t status, uint64_t options, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_brk(process_t* process, uint64_t addr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_mmap(process_t* process, uint64_t addr, uint64_t length, uint64_t prot, uint64_t flags, uint64_t fd, uint64_t offset);
int64_t sys_munmap(process_t* process, uint64_t addr, uint64_t length, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_gettimeofday(process_t* process, uint64_t tv, uint64_t tz, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t sys_sleep(process_t* process, uint64_t seconds, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

#endif // RAEENOS_SYSCALL_H 