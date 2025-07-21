#include "syscall.h"
#include "kernel.h"
#include "process/process.h"
#include "memory/memory.h"
#include "filesystem/vfs.h"
#include "network/network.h"
#include <string.h>

// Syscall table
syscall_handler_t syscall_table[MAX_SYSCALL_NUM + 1] = {NULL};

// Syscall tracing
static bool syscall_tracing_enabled = false;

// Global syscall state
static bool syscall_initialized = false;

// Syscall handler function signature
typedef u64 (*syscall_handler_t)(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);

// Syscall handler table
static syscall_handler_t syscall_handlers[256] = {0};

// Forward declarations for syscall handlers
static u64 sys_read(u64 fd, u64 buf, u64 count, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_write(u64 fd, u64 buf, u64 count, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_open(u64 pathname, u64 flags, u64 mode, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_close(u64 fd, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fork(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_execve(u64 filename, u64 argv, u64 envp, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_exit(u64 status, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getppid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_wait4(u64 pid, u64 status, u64 options, u64 rusage, u64 arg5, u64 arg6);
static u64 sys_brk(u64 addr, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_mmap(u64 addr, u64 length, u64 prot, u64 flags, u64 fd, u64 offset);
static u64 sys_munmap(u64 addr, u64 length, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_mprotect(u64 addr, u64 length, u64 prot, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_nanosleep(u64 req, u64 rem, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_gettimeofday(u64 tv, u64 tz, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_time(u64 tloc, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_kill(u64 pid, u64 sig, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_signal(u64 sig, u64 handler, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sigaction(u64 sig, u64 act, u64 oact, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_socket(u64 domain, u64 type, u64 protocol, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_bind(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_listen(u64 sockfd, u64 backlog, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_accept(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_connect(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sendto(u64 sockfd, u64 buf, u64 len, u64 flags, u64 dest_addr, u64 addrlen);
static u64 sys_recvfrom(u64 sockfd, u64 buf, u64 len, u64 flags, u64 src_addr, u64 addrlen);
static u64 sys_sendmsg(u64 sockfd, u64 msg, u64 flags, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_recvmsg(u64 sockfd, u64 msg, u64 flags, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_shutdown(u64 sockfd, u64 how, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setsockopt(u64 sockfd, u64 level, u64 optname, u64 optval, u64 optlen, u64 arg6);
static u64 sys_getsockopt(u64 sockfd, u64 level, u64 optname, u64 optval, u64 optlen, u64 arg6);
static u64 sys_pipe(u64 pipefd, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_dup(u64 oldfd, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_dup2(u64 oldfd, u64 newfd, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fcntl(u64 fd, u64 cmd, u64 arg, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_ioctl(u64 fd, u64 request, u64 arg, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_lseek(u64 fd, u64 offset, u64 whence, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_stat(u64 pathname, u64 statbuf, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fstat(u64 fd, u64 statbuf, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_mkdir(u64 pathname, u64 mode, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_rmdir(u64 pathname, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_unlink(u64 pathname, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_rename(u64 oldpath, u64 newpath, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_chdir(u64 path, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getcwd(u64 buf, u64 size, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_chmod(u64 pathname, u64 mode, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fchmod(u64 fd, u64 mode, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_chown(u64 pathname, u64 owner, u64 group, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fchown(u64 fd, u64 owner, u64 group, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_umask(u64 mask, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_access(u64 pathname, u64 mode, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_symlink(u64 target, u64 linkpath, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_readlink(u64 pathname, u64 buf, u64 bufsiz, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_truncate(u64 path, u64 length, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_ftruncate(u64 fd, u64 length, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fsync(u64 fd, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_fdatasync(u64 fd, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sync(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_select(u64 nfds, u64 readfds, u64 writefds, u64 exceptfds, u64 timeout, u64 arg6);
static u64 sys_poll(u64 fds, u64 nfds, u64 timeout, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_yield(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_setparam(u64 pid, u64 param, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_getparam(u64 pid, u64 param, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_setscheduler(u64 pid, u64 policy, u64 param, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_getscheduler(u64 pid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_get_priority_max(u64 policy, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_get_priority_min(u64 policy, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_rr_get_interval(u64 pid, u64 interval, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_setaffinity(u64 pid, u64 len, u64 user_mask_ptr, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sched_getaffinity(u64 pid, u64 len, u64 user_mask_ptr, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setpriority(u64 which, u64 who, u64 prio, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpriority(u64 which, u64 who, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setuid(u64 uid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setgid(u64 gid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getgid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_seteuid(u64 euid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_geteuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setegid(u64 egid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getegid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setreuid(u64 ruid, u64 euid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setregid(u64 rgid, u64 egid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setresuid(u64 ruid, u64 euid, u64 suid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getresuid(u64 ruid, u64 euid, u64 suid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setresgid(u64 rgid, u64 egid, u64 sgid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getresgid(u64 rgid, u64 egid, u64 sgid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setfsuid(u64 fsuid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setfsgid(u64 fsgid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setpgid(u64 pid, u64 pgid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpgid(u64 pid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpgrp(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setsid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getsid(u64 pid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getgroups(u64 size, u64 list, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setgroups(u64 size, u64 list, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_capget(u64 header, u64 dataptr, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_capset(u64 header, u64 data, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_prctl(u64 option, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_arch_prctl(u64 code, u64 addr, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setrlimit(u64 resource, u64 rlim, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getrlimit(u64 resource, u64 rlim, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getrusage(u64 who, u64 usage, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_times(u64 tbuf, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_ptrace(u64 request, u64 pid, u64 addr, u64 data, u64 arg5, u64 arg6);
static u64 sys_gettimeofday(u64 tv, u64 tz, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_settimeofday(u64 tv, u64 tz, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_adjtimex(u64 buf, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getitimer(u64 which, u64 value, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setitimer(u64 which, u64 value, u64 ovalue, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_alarm(u64 seconds, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getppid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_geteuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getgid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getegid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_gettid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sysinfo(u64 info, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_syslog(u64 type, u64 buf, u64 len, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setuid(u64 uid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setgid(u64 gid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_seteuid(u64 euid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setegid(u64 egid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setreuid(u64 ruid, u64 euid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setregid(u64 rgid, u64 egid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setresuid(u64 ruid, u64 euid, u64 suid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getresuid(u64 ruid, u64 euid, u64 suid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setresgid(u64 rgid, u64 egid, u64 sgid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getresgid(u64 rgid, u64 egid, u64 sgid, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setfsuid(u64 fsuid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setfsgid(u64 fsgid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setpgid(u64 pid, u64 pgid, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpgid(u64 pid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpgrp(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setsid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getsid(u64 pid, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getgroups(u64 size, u64 list, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setgroups(u64 size, u64 list, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_capget(u64 header, u64 dataptr, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_capset(u64 header, u64 data, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_prctl(u64 option, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_arch_prctl(u64 code, u64 addr, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setrlimit(u64 resource, u64 rlim, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getrlimit(u64 resource, u64 rlim, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getrusage(u64 who, u64 usage, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_times(u64 tbuf, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_ptrace(u64 request, u64 pid, u64 addr, u64 data, u64 arg5, u64 arg6);
static u64 sys_gettimeofday(u64 tv, u64 tz, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_settimeofday(u64 tv, u64 tz, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_adjtimex(u64 buf, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getitimer(u64 which, u64 value, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_setitimer(u64 which, u64 value, u64 ovalue, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_alarm(u64 seconds, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getpid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getppid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_geteuid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getgid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_getegid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_gettid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_sysinfo(u64 info, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
static u64 sys_syslog(u64 type, u64 buf, u64 len, u64 arg4, u64 arg5, u64 arg6);

error_t syscall_init(void) {
    if (syscall_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing system call interface...");
    
    // Initialize architecture-specific syscall setup
    error_t result = syscall_arch_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize architecture-specific syscall support");
        return result;
    }
    
    // Register syscall handlers
    syscall_handlers[1] = sys_exit;      // SYS_EXIT
    syscall_handlers[2] = sys_fork;      // SYS_FORK
    syscall_handlers[3] = sys_read;      // SYS_READ
    syscall_handlers[4] = sys_write;     // SYS_WRITE
    syscall_handlers[5] = sys_open;      // SYS_OPEN
    syscall_handlers[6] = sys_close;     // SYS_CLOSE
    syscall_handlers[7] = sys_execve;    // SYS_EXECVE
    syscall_handlers[8] = sys_wait4;     // SYS_WAIT4
    syscall_handlers[9] = sys_kill;      // SYS_KILL
    syscall_handlers[10] = sys_getpid;   // SYS_GETPID
    syscall_handlers[11] = sys_getppid;  // SYS_GETPPID
    syscall_handlers[12] = sys_brk;      // SYS_BRK
    syscall_handlers[13] = sys_mmap;     // SYS_MMAP
    syscall_handlers[14] = sys_munmap;   // SYS_MUNMAP
    syscall_handlers[15] = sys_stat;     // SYS_STAT
    syscall_handlers[16] = sys_fstat;    // SYS_FSTAT
    syscall_handlers[17] = sys_lseek;    // SYS_LSEEK
    syscall_handlers[18] = sys_mkdir;    // SYS_MKDIR
    syscall_handlers[19] = sys_rmdir;    // SYS_RMDIR
    syscall_handlers[20] = sys_unlink;   // SYS_UNLINK
    syscall_handlers[21] = sys_rename;   // SYS_RENAME
    syscall_handlers[24] = sys_socket;   // SYS_SOCKET
    syscall_handlers[25] = sys_bind;     // SYS_BIND
    syscall_handlers[26] = sys_listen;   // SYS_LISTEN
    syscall_handlers[27] = sys_accept;   // SYS_ACCEPT
    syscall_handlers[28] = sys_connect;  // SYS_CONNECT
    syscall_handlers[29] = sys_send;     // SYS_SEND
    syscall_handlers[30] = sys_recv;     // SYS_RECV
    
    syscall_initialized = true;
    
    KINFO("System call interface initialized successfully");
    return SUCCESS;
}

u64 syscall_dispatcher(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!syscall_initialized) {
        KERROR("Syscall system not initialized");
        return syscall_return_error(E_FAULT);
    }
    
    if (syscall_tracing_enabled) {
        syscall_log(syscall_num, arg1, arg2, arg3, arg4, arg5, arg6);
    }
    
    if (syscall_num >= 256 || !syscall_handlers[syscall_num]) {
        KWARN("Invalid or unimplemented syscall: %llu", syscall_num);
        return syscall_return_error(E_NOSYS);
    }
    
    // Dispatch to handler
    syscall_handler_t handler = syscall_handlers[syscall_num];
    u64 result = handler(arg1, arg2, arg3, arg4, arg5, arg6);
    
    if (syscall_tracing_enabled) {
        KDEBUG("Syscall %llu returned: %llx", syscall_num, result);
    }
    
    return result;
}

bool syscall_validate_number(u64 syscall_num) {
    return syscall_num <= MAX_SYSCALL_NUM;
}

bool syscall_validate_parameters(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    // Basic parameter validation - in a real system, this would be more comprehensive
    // For now, just check that pointers are in user space
    if (syscall_num == SYS_READ || syscall_num == SYS_WRITE) {
        if (arg2 && !memory_is_user_address(arg2)) {
            return false;
        }
    }
    
    if (syscall_num == SYS_OPEN || syscall_num == SYS_STAT || syscall_num == SYS_MKDIR || 
        syscall_num == SYS_RMDIR || syscall_num == SYS_UNLINK || syscall_num == SYS_RENAME ||
        syscall_num == SYS_CHDIR || syscall_num == SYS_GETCWD || syscall_num == SYS_CHMOD ||
        syscall_num == SYS_CHOWN || syscall_num == SYS_ACCESS || syscall_num == SYS_SYMLINK ||
        syscall_num == SYS_READLINK || syscall_num == SYS_TRUNCATE) {
        if (arg1 && !memory_is_user_address(arg1)) {
            return false;
        }
    }
    
    return true;
}

error_t copy_from_user(void* kernel_dst, const void* user_src, size_t size) {
    if (!memory_is_user_address((u64)user_src)) {
        return E_FAULT;
    }
    
    // TODO: Implement proper user space memory access
    // For now, just copy directly (unsafe)
    memcpy(kernel_dst, user_src, size);
    return SUCCESS;
}

error_t copy_to_user(void* user_dst, const void* kernel_src, size_t size) {
    if (!memory_is_user_address((u64)user_dst)) {
        return E_FAULT;
    }
    
    // TODO: Implement proper user space memory access
    // For now, just copy directly (unsafe)
    memcpy(user_dst, kernel_src, size);
    return SUCCESS;
}

error_t strncpy_from_user(char* kernel_dst, const char* user_src, size_t size) {
    if (!memory_is_user_address((u64)user_src)) {
        return E_FAULT;
    }
    
    // TODO: Implement proper user space string copy
    strncpy(kernel_dst, user_src, size);
    return SUCCESS;
}

error_t strncpy_to_user(char* user_dst, const char* kernel_src, size_t size) {
    if (!memory_is_user_address((u64)user_dst)) {
        return E_FAULT;
    }
    
    // TODO: Implement proper user space string copy
    strncpy(user_dst, kernel_src, size);
    return SUCCESS;
}

u64 syscall_return_error(error_t error) {
    return (u64)(-(s64)error);
}

u64 syscall_return_success(u64 value) {
    return value;
}

void syscall_log(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    KDEBUG("Syscall %u: args=(%llx, %llx, %llx, %llx, %llx, %llx)", 
           (u32)syscall_num, arg1, arg2, arg3, arg4, arg5, arg6);
}

void syscall_trace_enable(void) {
    syscall_tracing_enabled = true;
    KINFO("Syscall tracing enabled");
}

void syscall_trace_disable(void) {
    syscall_tracing_enabled = false;
    KINFO("Syscall tracing disabled");
}

// Process management syscalls
static u64 sys_exit(u64 status, u64, u64, u64, u64, u64) {
    process_t* current = process_get_current();
    if (current) {
        process_exit(current, (int)status);
    }
    return 0; // Should never return
}

static u64 sys_fork(u64, u64, u64, u64, u64, u64) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_FAULT);
    }
    
    pid_t new_pid = process_fork(current);
    if (new_pid < 0) {
        return syscall_return_error(E_NOMEM);
    }
    
    return syscall_return_success(new_pid);
}

static u64 sys_execve(u64 pathname, u64 argv, u64 envp, u64, u64, u64) {
    if (!pathname) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    char** args = (char**)argv;
    char** env = (char**)envp;
    
    error_t result = process_execve(path, args, env);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return 0; // Should never return
}

static u64 sys_wait4(u64 pid, u64 status, u64 options, u64 rusage, u64 arg5, u64 arg6) {
    KDEBUG("sys_wait4 called for PID %u", (pid_t)pid);
    
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_INVAL);
    }
    
    int exit_code;
    error_t result = process_wait((pid_t)pid, &exit_code);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    // Copy exit status to userspace if provided
    if (status != 0) {
        result = copy_to_user((void*)status, &exit_code, sizeof(exit_code));
        if (result != SUCCESS) {
            return syscall_return_error(result);
        }
    }
    
    return syscall_return_success((pid_t)pid);
}

static u64 sys_getpid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_INVAL);
    }
    return syscall_return_success(current->pid);
}

static u64 sys_getppid(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_INVAL);
    }
    return syscall_return_success(current->ppid);
}

// File operation syscalls
static u64 sys_open(u64 pathname, u64 flags, u64 mode, u64, u64, u64) {
    if (!pathname) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    file_handle_t file;
    
    error_t result = vfs_open(path, (int)flags, &file);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(file.fd);
}

static u64 sys_close(u64 fd, u64, u64, u64, u64, u64) {
    file_handle_t file = {.fd = (int)fd};
    error_t result = vfs_close(&file);
    
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_read(u64 fd, u64 buf, u64 count, u64, u64, u64) {
    if (!buf) {
        return syscall_return_error(E_INVAL);
    }
    
    file_handle_t file = {.fd = (int)fd};
    void* buffer = (void*)buf;
    
    size_t bytes_read = vfs_read(&file, buffer, count);
    if (bytes_read == (size_t)-1) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(bytes_read);
}

static u64 sys_write(u64 fd, u64 buf, u64 count, u64, u64, u64) {
    if (!buf) {
        return syscall_return_error(E_INVAL);
    }
    
    file_handle_t file = {.fd = (int)fd};
    const void* buffer = (const void*)buf;
    
    size_t bytes_written = vfs_write(&file, buffer, count);
    if (bytes_written == (size_t)-1) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(bytes_written);
}

static u64 sys_lseek(u64 fd, u64 offset, u64 whence, u64 arg4, u64 arg5, u64 arg6) {
    file_handle_t file = {.fd = (int)fd};
    off_t result = vfs_lseek(&file, (off_t)offset, (int)whence);
    
    if (result == (off_t)-1) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(result);
}

static u64 sys_stat(u64 pathname, u64 statbuf, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!pathname || !statbuf) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    struct stat* st = (struct stat*)statbuf;
    
    error_t result = vfs_stat(path, st);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_fstat(u64 fd, u64 statbuf, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!statbuf) {
        return syscall_return_error(E_INVAL);
    }
    
    file_handle_t file = {.fd = (int)fd};
    struct stat* st = (struct stat*)statbuf;
    
    error_t result = vfs_fstat(&file, st);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_mkdir(u64 pathname, u64 mode, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!pathname) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    error_t result = vfs_mkdir(path, (mode_t)mode);
    
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_rmdir(u64 pathname, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!pathname) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    error_t result = vfs_rmdir(path);
    
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_unlink(u64 pathname, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!pathname) {
        return syscall_return_error(E_INVAL);
    }
    
    char* path = (char*)pathname;
    error_t result = vfs_unlink(path);
    
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_rename(u64 oldpath, u64 newpath, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (!oldpath || !newpath) {
        return syscall_return_error(E_INVAL);
    }
    
    char* old = (char*)oldpath;
    char* new = (char*)newpath;
    error_t result = vfs_rename(old, new);
    
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

// Memory management syscalls
static u64 sys_brk(u64 addr, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_FAULT);
    }
    
    void* result = memory_brk(current, (void*)addr);
    if (result == (void*)-1) {
        return syscall_return_error(E_NOMEM);
    }
    
    return syscall_return_success((u64)result);
}

static u64 sys_mmap(u64 addr, u64 length, u64 prot, u64 flags, u64 fd, u64 offset) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_FAULT);
    }
    
    void* result = memory_mmap(current, (void*)addr, length, (int)prot, (int)flags, (int)fd, (off_t)offset);
    if (result == MAP_FAILED) {
        return syscall_return_error(E_NOMEM);
    }
    
    return syscall_return_success((u64)result);
}

static u64 sys_munmap(u64 addr, u64 length, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    process_t* current = process_get_current();
    if (!current) {
        return syscall_return_error(E_FAULT);
    }
    
    error_t result = memory_munmap(current, (void*)addr, length);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

// Network syscalls
static u64 sys_socket(u64 domain, u64 type, u64 protocol, u64 arg4, u64 arg5, u64 arg6) {
    int sockfd = network_socket((int)domain, (int)type, (int)protocol);
    if (sockfd < 0) {
        return syscall_return_error(E_INVAL);
    }
    
    return syscall_return_success(sockfd);
}

static u64 sys_bind(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6) {
    error_t result = network_bind((int)sockfd, (struct sockaddr*)addr, (size_t)addrlen);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_listen(u64 sockfd, u64 backlog, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    error_t result = network_listen((int)sockfd, (int)backlog);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_accept(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6) {
    int newfd = network_accept((int)sockfd, (struct sockaddr*)addr, (size_t*)addrlen);
    if (newfd < 0) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(newfd);
}

static u64 sys_connect(u64 sockfd, u64 addr, u64 addrlen, u64 arg4, u64 arg5, u64 arg6) {
    error_t result = network_connect((int)sockfd, (struct sockaddr*)addr, (size_t)addrlen);
    if (result != SUCCESS) {
        return syscall_return_error(result);
    }
    
    return syscall_return_success(0);
}

static u64 sys_send(u64 sockfd, u64 buf, u64 len, u64 flags, u64, u64) {
    ssize_t result = network_send((int)sockfd, (const void*)buf, (size_t)len, (int)flags);
    if (result < 0) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(result);
}

static u64 sys_recv(u64 sockfd, u64 buf, u64 len, u64 flags, u64, u64) {
    ssize_t result = network_recv((int)sockfd, (void*)buf, (size_t)len, (int)flags);
    if (result < 0) {
        return syscall_return_error(E_IO);
    }
    
    return syscall_return_success(result);
}

// ... existing code ... 