#include "syscall.h"
#include "process.h"
#include "memory.h"
#include "filesystem.h"
#include "network.h"
#include "interrupts.h"
#include "hal.h"
#include <string.h>
#include <stdio.h>
#include "../include/raeenos_stubs.h"

// System call table
static syscall_entry_t syscall_table[MAX_SYSCALL + 1];

// Current process context for system calls
extern process_t *current_process;

void syscall_init(void) {
    // Initialize system call table
    memset(syscall_table, 0, sizeof(syscall_table));
    
    // Register all system calls
    syscall_register(SYS_EXIT, sys_exit, "exit", 1);
    syscall_register(SYS_FORK, sys_fork, "fork", 0);
    syscall_register(SYS_READ, sys_read, "read", 3);
    syscall_register(SYS_WRITE, sys_write, "write", 3);
    syscall_register(SYS_OPEN, sys_open, "open", 3);
    syscall_register(SYS_CLOSE, sys_close, "close", 1);
    syscall_register(SYS_EXECVE, sys_execve, "execve", 3);
    syscall_register(SYS_WAITPID, sys_waitpid, "waitpid", 3);
    syscall_register(SYS_KILL, sys_kill, "kill", 2);
    syscall_register(SYS_GETPID, sys_getpid, "getpid", 0);
    syscall_register(SYS_SLEEP, sys_sleep, "sleep", 1);
    syscall_register(SYS_YIELD, sys_yield, "yield", 0);
    syscall_register(SYS_MMAP, sys_mmap, "mmap", 6);
    syscall_register(SYS_MUNMAP, sys_munmap, "munmap", 2);
    syscall_register(SYS_BRK, sys_brk, "brk", 1);
    syscall_register(SYS_SOCKET, sys_socket, "socket", 3);
    syscall_register(SYS_CONNECT, sys_connect, "connect", 3);
    syscall_register(SYS_ACCEPT, sys_accept, "accept", 3);
    syscall_register(SYS_SEND, sys_send, "send", 4);
    syscall_register(SYS_RECV, sys_recv, "recv", 4);
    syscall_register(SYS_PIPE, sys_pipe, "pipe", 1);
    syscall_register(SYS_DUP2, sys_dup2, "dup2", 2);
    syscall_register(SYS_CHDIR, sys_chdir, "chdir", 1);
    syscall_register(SYS_GETCWD, sys_getcwd, "getcwd", 2);
    syscall_register(SYS_MKDIR, sys_mkdir, "mkdir", 2);
    syscall_register(SYS_RMDIR, sys_rmdir, "rmdir", 1);
    syscall_register(SYS_LINK, sys_link, "link", 2);
    syscall_register(SYS_UNLINK, sys_unlink, "unlink", 1);
    syscall_register(SYS_STAT, sys_stat, "stat", 2);
    syscall_register(SYS_FSTAT, sys_fstat, "fstat", 2);
    syscall_register(SYS_FCNTL, sys_fcntl, "fcntl", 3);
    syscall_register(SYS_IOCTL, sys_ioctl, "ioctl", 3);
    syscall_register(SYS_SIGACTION, sys_sigaction, "sigaction", 3);
    syscall_register(SYS_SIGPROCMASK, sys_sigprocmask, "sigprocmask", 3);
    syscall_register(SYS_SIGSUSPEND, sys_sigsuspend, "sigsuspend", 1);
    syscall_register(SYS_ALARM, sys_alarm, "alarm", 1);
    syscall_register(SYS_GETTIMEOFDAY, sys_gettimeofday, "gettimeofday", 2);
    syscall_register(SYS_SETTIMEOFDAY, sys_settimeofday, "settimeofday", 2);
    syscall_register(SYS_GETUID, sys_getuid, "getuid", 0);
    syscall_register(SYS_SETUID, sys_setuid, "setuid", 1);
    syscall_register(SYS_GETGID, sys_getgid, "getgid", 0);
    syscall_register(SYS_SETGID, sys_setgid, "setgid", 1);
    syscall_register(SYS_CHMOD, sys_chmod, "chmod", 2);
    syscall_register(SYS_CHOWN, sys_chown, "chown", 3);
    syscall_register(SYS_UMASK, sys_umask, "umask", 1);
    syscall_register(SYS_GETPPID, sys_getppid, "getppid", 0);
    syscall_register(SYS_SETSID, sys_setsid, "setsid", 0);
    syscall_register(SYS_GETSID, sys_getsid, "getsid", 1);
    syscall_register(SYS_SYNC, sys_sync, "sync", 0);
    syscall_register(SYS_FSYNC, sys_fsync, "fsync", 1);
    syscall_register(SYS_TRUNCATE, sys_truncate, "truncate", 2);
    syscall_register(SYS_FTRUNCATE, sys_ftruncate, "ftruncate", 2);
    syscall_register(SYS_READDIR, sys_readdir, "readdir", 2);
    syscall_register(SYS_MKNOD, sys_mknod, "mknod", 3);
    syscall_register(SYS_MOUNT, sys_mount, "mount", 5);
    syscall_register(SYS_UMOUNT, sys_umount, "umount", 2);
    syscall_register(SYS_SYSCONF, sys_sysconf, "sysconf", 1);
    syscall_register(SYS_GETRLIMIT, sys_getrlimit, "getrlimit", 2);
    syscall_register(SYS_SETRLIMIT, sys_setrlimit, "setrlimit", 2);
    syscall_register(SYS_GETPRIORITY, sys_getpriority, "getpriority", 2);
    syscall_register(SYS_SETPRIORITY, sys_setpriority, "setpriority", 3);
    syscall_register(SYS_SCHED_YIELD, sys_sched_yield, "sched_yield", 0);
    syscall_register(SYS_SCHED_GETPARAM, sys_sched_getparam, "sched_getparam", 2);
    syscall_register(SYS_SCHED_SETPARAM, sys_sched_setparam, "sched_setparam", 2);
    syscall_register(SYS_SCHED_GETSCHEDULER, sys_sched_getscheduler, "sched_getscheduler", 1);
    syscall_register(SYS_SCHED_SETSCHEDULER, sys_sched_setscheduler, "sched_setscheduler", 3);
    syscall_register(SYS_CLONE, sys_clone, "clone", 5);
    syscall_register(SYS_VFORK, sys_vfork, "vfork", 0);
    syscall_register(SYS_EXIT_GROUP, sys_exit_group, "exit_group", 1);
    syscall_register(SYS_WAIT4, sys_wait4, "wait4", 4);
    
    printf("System call interface initialized with %d system calls\n", MAX_SYSCALL);
}

void syscall_register(int number, syscall_handler_t handler, const char *name, int num_args) {
    if (number >= 0 && number <= MAX_SYSCALL) {
        syscall_table[number].handler = handler;
        syscall_table[number].name = name;
        syscall_table[number].num_args = num_args;
    }
}

int64_t syscall_handler(uint64_t number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    // Validate system call number
    if (number > MAX_SYSCALL || !syscall_table[number].handler) {
        printf("Invalid system call: %lu\n", number);
        return SYS_EINVAL;
    }
    
    // Create argument structure
    uint64_t args[6] = {arg1, arg2, arg3, arg4, arg5, arg6};
    
    // Call the appropriate handler
    int64_t result = syscall_table[number].handler(args);
    
    // Set errno if there was an error
    if (result < 0 && current_process) {
        current_process->errno = -result;
    }
    
    return result;
}

// Basic system call implementations
int64_t sys_exit(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int status = (int)arg_array[0];
    
    if (current_process) {
        current_process->exit_code = status;
        process_exit(current_process);
    }
    
    return SYS_SUCCESS;
}

int64_t sys_fork(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    process_t *child = process_fork(current_process);
    if (!child) {
        return SYS_ENOMEM;
    }
    
    return child->pid;
}

int64_t sys_read(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    void *buf = (void *)arg_array[1];
    size_t count = (size_t)arg_array[2];
    
    if (!current_process || !buf) {
        return SYS_EFAULT;
    }
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return SYS_EBADF;
    }
    
    // Call filesystem read
    return filesystem_read(fd, buf, count);
}

int64_t sys_write(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    const void *buf = (const void *)arg_array[1];
    size_t count = (size_t)arg_array[2];
    
    if (!current_process || !buf) {
        return SYS_EFAULT;
    }
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return SYS_EBADF;
    }
    
    // Call filesystem write
    return filesystem_write(fd, buf, count);
}

int64_t sys_open(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *pathname = (const char *)arg_array[0];
    int flags = (int)arg_array[1];
    mode_t mode = (mode_t)arg_array[2];
    
    if (!current_process || !pathname) {
        return SYS_EFAULT;
    }
    
    // Call filesystem open
    return filesystem_open(pathname, flags, mode);
}

int64_t sys_close(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return SYS_EBADF;
    }
    
    // Call filesystem close
    return filesystem_close(fd);
}

int64_t sys_execve(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *filename = (const char *)arg_array[0];
    char *const *argv = (char *const *)arg_array[1];
    char *const *envp = (char *const *)arg_array[2];
    
    if (!current_process || !filename) {
        return SYS_EFAULT;
    }
    
    // Call process exec
    return process_exec(current_process, filename, argv, envp);
}

int64_t sys_waitpid(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    int *wstatus = (int *)arg_array[1];
    int options = (int)arg_array[2];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call process wait
    return process_wait(pid, wstatus, options);
}

int64_t sys_kill(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    int sig = (int)arg_array[1];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call process kill
    return process_kill(pid, sig);
}

int64_t sys_getpid(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    return current_process->pid;
}

int64_t sys_sleep(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    unsigned int seconds = (unsigned int)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call process sleep
    return process_sleep(current_process, seconds);
}

int64_t sys_yield(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Yield CPU to another process
    process_yield(current_process);
    return SYS_SUCCESS;
}

int64_t sys_mmap(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    void *addr = (void *)arg_array[0];
    size_t length = (size_t)arg_array[1];
    int prot = (int)arg_array[2];
    int flags = (int)arg_array[3];
    int fd = (int)arg_array[4];
    off_t offset = (off_t)arg_array[5];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call memory management mmap
    return memory_mmap(current_process, addr, length, prot, flags, fd, offset);
}

int64_t sys_munmap(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    void *addr = (void *)arg_array[0];
    size_t length = (size_t)arg_array[1];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call memory management munmap
    return memory_munmap(current_process, addr, length);
}

int64_t sys_brk(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    void *addr = (void *)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call memory management brk
    return memory_brk(current_process, addr);
}

// Network system calls
int64_t sys_socket(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int domain = (int)arg_array[0];
    int type = (int)arg_array[1];
    int protocol = (int)arg_array[2];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call network socket
    return network_socket(domain, type, protocol);
}

int64_t sys_connect(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int sockfd = (int)arg_array[0];
    const struct sockaddr *addr = (const struct sockaddr *)arg_array[1];
    socklen_t addrlen = (socklen_t)arg_array[2];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call network connect
    return network_connect(sockfd, addr, addrlen);
}

int64_t sys_accept(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int sockfd = (int)arg_array[0];
    struct sockaddr *addr = (struct sockaddr *)arg_array[1];
    socklen_t *addrlen = (socklen_t *)arg_array[2];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call network accept
    return network_accept(sockfd, addr, addrlen);
}

int64_t sys_send(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int sockfd = (int)arg_array[0];
    const void *buf = (const void *)arg_array[1];
    size_t len = (size_t)arg_array[2];
    int flags = (int)arg_array[3];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call network send
    return network_send(sockfd, buf, len, flags);
}

int64_t sys_recv(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int sockfd = (int)arg_array[0];
    void *buf = (void *)arg_array[1];
    size_t len = (size_t)arg_array[2];
    int flags = (int)arg_array[3];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Call network recv
    return network_recv(sockfd, buf, len, flags);
}

// Placeholder implementations for remaining system calls
int64_t sys_pipe(void *args) { return SYS_ENOSYS; }
int64_t sys_dup2(void *args) { return SYS_ENOSYS; }
int64_t sys_chdir(void *args) { return SYS_ENOSYS; }
int64_t sys_getcwd(void *args) { return SYS_ENOSYS; }
int64_t sys_mkdir(void *args) { return SYS_ENOSYS; }
int64_t sys_rmdir(void *args) { return SYS_ENOSYS; }
int64_t sys_link(void *args) { return SYS_ENOSYS; }
int64_t sys_unlink(void *args) { return SYS_ENOSYS; }
int64_t sys_stat(void *args) { return SYS_ENOSYS; }
int64_t sys_fstat(void *args) { return SYS_ENOSYS; }
int64_t sys_fcntl(void *args) { return SYS_ENOSYS; }
int64_t sys_ioctl(void *args) { return SYS_ENOSYS; }
int64_t sys_sigaction(void *args) { return SYS_ENOSYS; }
int64_t sys_sigprocmask(void *args) { return SYS_ENOSYS; }
int64_t sys_sigsuspend(void *args) { return SYS_ENOSYS; }
int64_t sys_alarm(void *args) { return SYS_ENOSYS; }
int64_t sys_gettimeofday(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    struct timeval *tv = (struct timeval *)arg_array[0];
    void *tz = (void *)arg_array[1];
    
    if (!tv) {
        return SYS_EFAULT;
    }
    
    // Get current timestamp
    uint64_t timestamp = hal_get_timestamp();
    
    tv->tv_sec = timestamp / 1000000;  // Convert to seconds
    tv->tv_usec = timestamp % 1000000; // Microseconds
    
    if (tz) {
        // Timezone not implemented
        memset(tz, 0, sizeof(struct timezone));
    }
    
    return SYS_SUCCESS;
}

int64_t sys_settimeofday(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const struct timeval *tv = (const struct timeval *)arg_array[0];
    const void *tz = (const void *)arg_array[1];
    
    if (!tv) {
        return SYS_EFAULT;
    }
    
    // Setting time not implemented yet
    return SYS_EPERM;
}

int64_t sys_getuid(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    return current_process->uid;
}

int64_t sys_setuid(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    uid_t uid = (uid_t)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Only root can change UID
    if (current_process->uid != 0) {
        return SYS_EPERM;
    }
    
    current_process->uid = uid;
    return SYS_SUCCESS;
}

int64_t sys_getgid(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    return current_process->gid;
}

int64_t sys_setgid(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    gid_t gid = (gid_t)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Only root can change GID
    if (current_process->uid != 0) {
        return SYS_EPERM;
    }
    
    current_process->gid = gid;
    return SYS_SUCCESS;
}

int64_t sys_chmod(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *path = (const char *)arg_array[0];
    mode_t mode = (mode_t)arg_array[1];
    
    if (!path) {
        return SYS_EFAULT;
    }
    
    // Call filesystem chmod
    return filesystem_chmod(path, mode);
}

int64_t sys_chown(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *path = (const char *)arg_array[0];
    uid_t owner = (uid_t)arg_array[1];
    gid_t group = (gid_t)arg_array[2];
    
    if (!path) {
        return SYS_EFAULT;
    }
    
    // Call filesystem chown
    return filesystem_chown(path, owner, group);
}

int64_t sys_umask(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    mode_t mask = (mode_t)arg_array[0];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    mode_t old_mask = current_process->umask;
    current_process->umask = mask;
    
    return old_mask;
}

int64_t sys_getppid(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    return current_process->ppid;
}

int64_t sys_setsid(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // Create new session
    current_process->sid = current_process->pid;
    current_process->pgid = current_process->pid;
    
    return current_process->sid;
}

int64_t sys_getsid(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    
    if (pid == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return current_process->sid;
    }
    
    process_t *process = process_get_by_pid(pid);
    if (!process) {
        return SYS_ESRCH;
    }
    
    return process->sid;
}

int64_t sys_sync(void *args) {
    // Sync all filesystems
    return filesystem_sync();
}

int64_t sys_fsync(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return SYS_EBADF;
    }
    
    // Call filesystem fsync
    return filesystem_fsync(fd);
}

int64_t sys_truncate(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *path = (const char *)arg_array[0];
    off_t length = (off_t)arg_array[1];
    
    if (!path) {
        return SYS_EFAULT;
    }
    
    // Call filesystem truncate
    return filesystem_truncate(path, length);
}

int64_t sys_ftruncate(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    off_t length = (off_t)arg_array[1];
    
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return SYS_EBADF;
    }
    
    // Call filesystem ftruncate
    return filesystem_ftruncate(fd, length);
}

int64_t sys_readdir(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int fd = (int)arg_array[0];
    struct dirent *dirp = (struct dirent *)arg_array[1];
    
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS || !dirp) {
        return SYS_EBADF;
    }
    
    // Call filesystem readdir
    return filesystem_readdir(fd, dirp);
}

int64_t sys_mknod(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *pathname = (const char *)arg_array[0];
    mode_t mode = (mode_t)arg_array[1];
    dev_t dev = (dev_t)arg_array[2];
    
    if (!pathname) {
        return SYS_EFAULT;
    }
    
    // Call filesystem mknod
    return filesystem_mknod(pathname, mode, dev);
}

int64_t sys_mount(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *source = (const char *)arg_array[0];
    const char *target = (const char *)arg_array[1];
    const char *filesystemtype = (const char *)arg_array[2];
    unsigned long mountflags = (unsigned long)arg_array[3];
    const void *data = (const void *)arg_array[4];
    
    if (!source || !target || !filesystemtype) {
        return SYS_EFAULT;
    }
    
    // Call filesystem mount
    return filesystem_mount(source, target, filesystemtype, mountflags, data);
}

int64_t sys_umount(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    const char *target = (const char *)arg_array[0];
    int flags = (int)arg_array[1];
    
    if (!target) {
        return SYS_EFAULT;
    }
    
    // Call filesystem umount
    return filesystem_umount(target, flags);
}

int64_t sys_sysconf(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int name = (int)arg_array[0];
    
    switch (name) {
        case _SC_PAGESIZE:
            return PAGE_SIZE;
        case _SC_NPROCESSORS_ONLN:
            return 1; // Single CPU for now
        case _SC_OPEN_MAX:
            return MAX_FILE_DESCRIPTORS;
        case _SC_CLK_TCK:
            return 100; // 100 Hz timer
        default:
            return SYS_EINVAL;
    }
}

int64_t sys_getrlimit(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int resource = (int)arg_array[0];
    struct rlimit *rlim = (struct rlimit *)arg_array[1];
    
    if (!rlim) {
        return SYS_EFAULT;
    }
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    switch (resource) {
        case RLIMIT_AS:
            rlim->rlim_cur = current_process->memory_limit;
            rlim->rlim_max = current_process->memory_limit;
            break;
        case RLIMIT_NOFILE:
            rlim->rlim_cur = current_process->file_limit;
            rlim->rlim_max = current_process->file_limit;
            break;
        default:
            return SYS_EINVAL;
    }
    
    return SYS_SUCCESS;
}

int64_t sys_setrlimit(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int resource = (int)arg_array[0];
    const struct rlimit *rlim = (const struct rlimit *)arg_array[1];
    
    if (!rlim) {
        return SYS_EFAULT;
    }
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    switch (resource) {
        case RLIMIT_AS:
            current_process->memory_limit = rlim->rlim_cur;
            break;
        case RLIMIT_NOFILE:
            current_process->file_limit = rlim->rlim_cur;
            break;
        default:
            return SYS_EINVAL;
    }
    
    return SYS_SUCCESS;
}

int64_t sys_getpriority(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int which = (int)arg_array[0];
    id_t who = (id_t)arg_array[1];
    
    if (which != PRIO_PROCESS) {
        return SYS_EINVAL;
    }
    
    if (who == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return current_process->priority;
    }
    
    process_t *process = process_get_by_pid(who);
    if (!process) {
        return SYS_ESRCH;
    }
    
    return process->priority;
}

int64_t sys_setpriority(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int which = (int)arg_array[0];
    id_t who = (id_t)arg_array[1];
    int prio = (int)arg_array[2];
    
    if (which != PRIO_PROCESS) {
        return SYS_EINVAL;
    }
    
    if (who == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return process_set_priority(current_process->pid, prio);
    }
    
    return process_set_priority(who, prio);
}

int64_t sys_sched_yield(void *args) {
    if (!current_process) {
        return SYS_EPERM;
    }
    
    scheduler_yield();
    return SYS_SUCCESS;
}

int64_t sys_sched_getparam(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    struct sched_param *param = (struct sched_param *)arg_array[1];
    
    if (!param) {
        return SYS_EFAULT;
    }
    
    if (pid == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        param->sched_priority = current_process->priority;
        return SYS_SUCCESS;
    }
    
    process_t *process = process_get_by_pid(pid);
    if (!process) {
        return SYS_ESRCH;
    }
    
    param->sched_priority = process->priority;
    return SYS_SUCCESS;
}

int64_t sys_sched_setparam(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    const struct sched_param *param = (const struct sched_param *)arg_array[1];
    
    if (!param) {
        return SYS_EFAULT;
    }
    
    if (pid == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return process_set_priority(current_process->pid, param->sched_priority);
    }
    
    return process_set_priority(pid, param->sched_priority);
}

int64_t sys_sched_getscheduler(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    
    if (pid == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return SCHED_OTHER; // Default scheduler
    }
    
    process_t *process = process_get_by_pid(pid);
    if (!process) {
        return SYS_ESRCH;
    }
    
    return SCHED_OTHER; // Default scheduler
}

int64_t sys_sched_setscheduler(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    int policy = (int)arg_array[1];
    const struct sched_param *param = (const struct sched_param *)arg_array[2];
    
    if (!param) {
        return SYS_EFAULT;
    }
    
    if (policy != SCHED_OTHER) {
        return SYS_EINVAL;
    }
    
    if (pid == 0) {
        if (!current_process) {
            return SYS_EPERM;
        }
        return process_set_priority(current_process->pid, param->sched_priority);
    }
    
    return process_set_priority(pid, param->sched_priority);
}

int64_t sys_clone(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    unsigned long flags = (unsigned long)arg_array[0];
    void *child_stack = (void *)arg_array[1];
    void *ptid = (void *)arg_array[2];
    void *tls = (void *)arg_array[3];
    void *ctid = (void *)arg_array[4];
    
    if (!current_process) {
        return SYS_EPERM;
    }
    
    // For now, just do a simple fork
    process_t *child = process_fork(current_process);
    if (!child) {
        return SYS_ENOMEM;
    }
    
    return child->pid;
}

int64_t sys_vfork(void *args) {
    // vfork is similar to fork but with shared memory
    return sys_fork(args);
}

int64_t sys_exit_group(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    int status = (int)arg_array[0];
    
    if (current_process) {
        current_process->exit_code = status;
        process_exit(current_process);
    }
    
    return SYS_SUCCESS;
}

int64_t sys_wait4(void *args) {
    uint64_t *arg_array = (uint64_t *)args;
    pid_t pid = (pid_t)arg_array[0];
    int *status = (int *)arg_array[1];
    int options = (int)arg_array[2];
    struct rusage *rusage = (struct rusage *)arg_array[3];
    
    if (pid == -1) {
        // Wait for any child
        // For now, just return success
        if (status) {
            *status = 0;
        }
        return 0;
    }
    
    return process_wait(pid, status);
}
int64_t sys_setsockopt(void *args) { return SYS_ENOSYS; }
int64_t sys_getsockopt(void *args) { return SYS_ENOSYS; }
int64_t sys_shutdown(void *args) { return SYS_ENOSYS; }
int64_t sys_bind(void *args) { return SYS_ENOSYS; }
int64_t sys_listen(void *args) { return SYS_ENOSYS; }
int64_t sys_getsockname(void *args) { return SYS_ENOSYS; }
int64_t sys_getpeername(void *args) { return SYS_ENOSYS; }
int64_t sys_sendto(void *args) { return SYS_ENOSYS; }
int64_t sys_recvfrom(void *args) { return SYS_ENOSYS; }
int64_t sys_sendmsg(void *args) { return SYS_ENOSYS; }
int64_t sys_recvmsg(void *args) { return SYS_ENOSYS; }
int64_t sys_epoll_create(void *args) { return SYS_ENOSYS; }
int64_t sys_epoll_ctl(void *args) { return SYS_ENOSYS; }
int64_t sys_epoll_wait(void *args) { return SYS_ENOSYS; }
int64_t sys_signalfd(void *args) { return SYS_ENOSYS; }
int64_t sys_timerfd_create(void *args) { return SYS_ENOSYS; }
int64_t sys_timerfd_settime(void *args) { return SYS_ENOSYS; }
int64_t sys_timerfd_gettime(void *args) { return SYS_ENOSYS; }
int64_t sys_eventfd(void *args) { return SYS_ENOSYS; }
int64_t sys_fallocate(void *args) { return SYS_ENOSYS; }
int64_t sys_timer_create(void *args) { return SYS_ENOSYS; }
int64_t sys_timer_settime(void *args) { return SYS_ENOSYS; }
int64_t sys_timer_gettime(void *args) { return SYS_ENOSYS; }
int64_t sys_timer_delete(void *args) { return SYS_ENOSYS; }
int64_t sys_clock_gettime(void *args) { return SYS_ENOSYS; }
int64_t sys_clock_settime(void *args) { return SYS_ENOSYS; }
int64_t sys_clock_getres(void *args) { return SYS_ENOSYS; }
int64_t sys_nanosleep(void *args) { return SYS_ENOSYS; }
int64_t sys_getrandom(void *args) { return SYS_ENOSYS; }
int64_t sys_memfd_create(void *args) { return SYS_ENOSYS; } 