#include "syscall/syscall.h"
#include "syscall/syscall_id.h"
#include "syscall/syscall_defs.h"
#include "kernel.h"
#include "process.h"
#include "hal/hal.h"

// Syscall handler function signature
typedef u64 (*syscall_handler_t)(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);

// Syscall handler prototypes (stubs)
static u64 sys_exit(u64 status, u64, u64, u64, u64, u64);
static u64 sys_sched_yield(u64, u64, u64, u64, u64, u64);
static u64 sys_getpid(u64, u64, u64, u64, u64, u64);

// Syscall handler table
static syscall_handler_t syscall_handlers[MAX_SYSCALLS];

void syscall_init(void) {
    KINFO("Initializing system call interface...");
    x64_syscall_init(); // Architecture-specific setup (MSRs)

    // Initialize the handler table with all syscall handlers
    memset(syscall_handlers, 0, sizeof(syscall_handlers));
    
    // Process management syscalls
    syscall_handlers[SYS_EXIT] = sys_exit;
    syscall_handlers[SYS_FORK] = sys_fork;
    syscall_handlers[SYS_EXECVE] = sys_execve;
    syscall_handlers[SYS_WAIT] = sys_wait;
    syscall_handlers[SYS_GETPID] = sys_getpid;
    syscall_handlers[SYS_GETPPID] = sys_getppid;
    syscall_handlers[SYS_KILL] = sys_kill;
    
    // File operation syscalls
    syscall_handlers[SYS_OPEN] = sys_open;
    syscall_handlers[SYS_CLOSE] = sys_close;
    syscall_handlers[SYS_READ] = sys_read;
    syscall_handlers[SYS_WRITE] = sys_write;
    syscall_handlers[SYS_LSEEK] = sys_lseek;
    syscall_handlers[SYS_STAT] = sys_stat;
    syscall_handlers[SYS_FSTAT] = sys_fstat;
    syscall_handlers[SYS_UNLINK] = sys_unlink;
    syscall_handlers[SYS_RENAME] = sys_rename;
    
    // Directory operation syscalls
    syscall_handlers[SYS_MKDIR] = sys_mkdir;
    syscall_handlers[SYS_RMDIR] = sys_rmdir;
    syscall_handlers[SYS_CHDIR] = sys_chdir;
    syscall_handlers[SYS_GETCWD] = sys_getcwd;
    
    // Memory management syscalls
    syscall_handlers[SYS_BRK] = sys_brk;
    syscall_handlers[SYS_MMAP] = sys_mmap;
    syscall_handlers[SYS_MUNMAP] = sys_munmap;
    syscall_handlers[SYS_MPROTECT] = sys_mprotect;
    
    // Time syscalls
    syscall_handlers[SYS_TIME] = sys_time;
    syscall_handlers[SYS_GETTIMEOFDAY] = sys_gettimeofday;
    syscall_handlers[SYS_NANOSLEEP] = sys_nanosleep;
    
    // Signal syscalls
    syscall_handlers[SYS_SIGACTION] = sys_sigaction;
    syscall_handlers[SYS_SIGPROCMASK] = sys_sigprocmask;
    syscall_handlers[SYS_SIGSUSPEND] = sys_sigsuspend;
    
    // Network syscalls
    syscall_handlers[SYS_SOCKET] = sys_socket;
    syscall_handlers[SYS_BIND] = sys_bind;
    syscall_handlers[SYS_LISTEN] = sys_listen;
    syscall_handlers[SYS_ACCEPT] = sys_accept;
    syscall_handlers[SYS_CONNECT] = sys_connect;
    syscall_handlers[SYS_SEND] = sys_send;
    syscall_handlers[SYS_RECV] = sys_recv;
    
    // Process scheduling syscalls
    syscall_handlers[SYS_SCHED_YIELD] = sys_sched_yield;
    syscall_handlers[SYS_SCHED_GETPARAM] = sys_sched_getparam;
    syscall_handlers[SYS_SCHED_SETPARAM] = sys_sched_setparam;
    
    // System information syscalls
    syscall_handlers[SYS_SYSCONF] = sys_sysconf;
    syscall_handlers[SYS_GETRLIMIT] = sys_getrlimit;
    syscall_handlers[SYS_SETRLIMIT] = sys_setrlimit;
    syscall_handlers[SYS_GETRUSAGE] = sys_getrusage;

    KINFO("System call interface initialized with %d handlers.", MAX_SYSCALLS);
}

u64 syscall_dispatcher(syscall_frame_t* frame) {
    syscall_id_t syscall_num = frame->rax;

    if (syscall_num >= MAX_SYSCALLS || !syscall_handlers[syscall_num]) {
        KWARN("Invalid or unimplemented syscall: %llu", syscall_num);
        return -ENOSYS; // Return "Function not implemented"
    }

    // Dispatch to the correct handler
    syscall_handler_t handler = syscall_handlers[syscall_num];
    return handler(frame->rdi, frame->rsi, frame->rdx, frame->r10, frame->r8, frame->r9);
}


/*
 * =====================================================================================
 *  Syscall Handler Implementations (Stubs)
 * =====================================================================================
 */

static u64 sys_exit(u64 status, u64, u64, u64, u64, u64) {
    // Cast to the proper struct type for clarity, even if not strictly needed here
    syscall_exit_t args = { .status = status };

    KINFO("PID %u exiting with status %d", get_current_process()->pid, args.status);
    
    // TODO: Implement process termination logic.
    // This should clean up all process resources and never return.
    hal_halt_cpu();
    return 0; // Should be unreachable
}

static u64 sys_sched_yield(u64, u64, u64, u64, u64, u64) {
    // TODO: Properly implement schedule() to be callable from here.
    // For now, this is a placeholder.
    KDEBUG("sys_sched_yield called");
    schedule();
    return 0;
}

static u64 sys_getpid(u64, u64, u64, u64, u64, u64) {
    pcb_t* current = get_current_process();
    if (!current) {
        return -ESRCH;
    }
    return current->pid;
}

// Add missing syscall handler prototypes
static u64 sys_fork(u64, u64, u64, u64, u64, u64);
static u64 sys_execve(u64, u64, u64, u64, u64, u64);
static u64 sys_wait(u64, u64, u64, u64, u64, u64);
static u64 sys_getppid(u64, u64, u64, u64, u64, u64);
static u64 sys_kill(u64, u64, u64, u64, u64, u64);
static u64 sys_open(u64, u64, u64, u64, u64, u64);
static u64 sys_close(u64, u64, u64, u64, u64, u64);
static u64 sys_read(u64, u64, u64, u64, u64, u64);
static u64 sys_write(u64, u64, u64, u64, u64, u64);
static u64 sys_lseek(u64, u64, u64, u64, u64, u64);
static u64 sys_stat(u64, u64, u64, u64, u64, u64);
static u64 sys_fstat(u64, u64, u64, u64, u64, u64);
static u64 sys_unlink(u64, u64, u64, u64, u64, u64);
static u64 sys_rename(u64, u64, u64, u64, u64, u64);
static u64 sys_mkdir(u64, u64, u64, u64, u64, u64);
static u64 sys_rmdir(u64, u64, u64, u64, u64, u64);
static u64 sys_chdir(u64, u64, u64, u64, u64, u64);
static u64 sys_getcwd(u64, u64, u64, u64, u64, u64);
static u64 sys_brk(u64, u64, u64, u64, u64, u64);
static u64 sys_mmap(u64, u64, u64, u64, u64, u64);
static u64 sys_munmap(u64, u64, u64, u64, u64, u64);
static u64 sys_mprotect(u64, u64, u64, u64, u64, u64);
static u64 sys_time(u64, u64, u64, u64, u64, u64);
static u64 sys_gettimeofday(u64, u64, u64, u64, u64, u64);
static u64 sys_nanosleep(u64, u64, u64, u64, u64, u64);
static u64 sys_sigaction(u64, u64, u64, u64, u64, u64);
static u64 sys_sigprocmask(u64, u64, u64, u64, u64, u64);
static u64 sys_sigsuspend(u64, u64, u64, u64, u64, u64);
static u64 sys_socket(u64, u64, u64, u64, u64, u64);
static u64 sys_bind(u64, u64, u64, u64, u64, u64);
static u64 sys_listen(u64, u64, u64, u64, u64, u64);
static u64 sys_accept(u64, u64, u64, u64, u64, u64);
static u64 sys_connect(u64, u64, u64, u64, u64, u64);
static u64 sys_send(u64, u64, u64, u64, u64, u64);
static u64 sys_recv(u64, u64, u64, u64, u64, u64);
static u64 sys_sched_getparam(u64, u64, u64, u64, u64, u64);
static u64 sys_sched_setparam(u64, u64, u64, u64, u64, u64);
static u64 sys_sysconf(u64, u64, u64, u64, u64, u64);
static u64 sys_getrlimit(u64, u64, u64, u64, u64, u64);
static u64 sys_setrlimit(u64, u64, u64, u64, u64, u64);
static u64 sys_getrusage(u64, u64, u64, u64, u64, u64);

// You would continue to implement stubs for all other syscalls...
//
// static u64 sys_open(u64 pathname_ptr, u64 flags, u64 mode, ...) {
//     syscall_open_t args = { .pathname = (const char*)pathname_ptr, ... };
//     
//     // TODO: Validate user-space pointer 'args.pathname' is accessible and valid.
//     // TODO: Implement file opening logic.
//
//     return -ENOSYS;
// }

/*
 * =====================================================================================
 *  Syscall Handler Implementations
 * =====================================================================================
 */

// Process management syscalls
static u64 sys_fork(u64, u64, u64, u64, u64, u64) {
    KDEBUG("sys_fork called");
    
    // TODO: Implement actual process forking
    // This should:
    // 1. Create new process
    // 2. Copy parent's memory (copy-on-write)
    // 3. Set up child's registers
    // 4. Return child PID to parent, 0 to child
    
    // For now, return a placeholder PID
    static pid_t next_pid = 1000;
    return next_pid++;
}

static u64 sys_execve(u64 pathname, u64 argv, u64 envp, u64, u64, u64) {
    KDEBUG("sys_execve: pathname=%p", (void*)pathname);
    
    // TODO: Implement actual program execution
    // This should:
    // 1. Validate pathname pointer
    // 2. Load executable file
    // 3. Set up new address space
    // 4. Set up arguments and environment
    // 5. Jump to new program
    
    // For now, just return success
    return 0;
}

static u64 sys_wait(u64 status, u64, u64, u64, u64, u64) {
    KDEBUG("sys_wait called");
    
    // TODO: Implement actual process waiting
    // This should:
    // 1. Wait for child process to exit
    // 2. Collect exit status
    // 3. Clean up child process
    // 4. Return child PID
    
    // For now, return a placeholder PID
    return 1001;
}

static u64 sys_getppid(u64, u64, u64, u64, u64, u64) {
    pcb_t* current = get_current_process();
    if (!current) {
        return -ESRCH;
    }
    return current->parent_pid;
}

static u64 sys_kill(u64 pid, u64 sig, u64, u64, u64, u64) {
    KDEBUG("sys_kill: pid=%llu, sig=%llu", pid, sig);
    // TODO: Implement signal sending
    return -ENOSYS;
}

// File operation syscalls
static u64 sys_open(u64 pathname, u64 flags, u64 mode, u64, u64, u64) {
    KDEBUG("sys_open: pathname=%p, flags=%llu, mode=%llu", (void*)pathname, flags, mode);
    
    // TODO: Implement actual file opening
    // For now, return a placeholder file descriptor
    static int next_fd = 3; // 0,1,2 are stdin,stdout,stderr
    return next_fd++;
}

static u64 sys_close(u64 fd, u64, u64, u64, u64, u64) {
    KDEBUG("sys_close: fd=%llu", fd);
    
    // TODO: Implement actual file closing
    // For now, just return success
    return 0;
}

static u64 sys_read(u64 fd, u64 buf, u64 count, u64, u64, u64) {
    KDEBUG("sys_read: fd=%llu, buf=%p, count=%llu", fd, (void*)buf, count);
    
    // Handle special file descriptors
    if (fd == 0) { // stdin
        // Read from keyboard
        int c = keyboard_read_char();
        if (c != -1 && count > 0) {
            // TODO: Copy to user buffer
            return 1;
        }
        return 0;
    }
    
    // TODO: Implement actual file reading
    return -ENOSYS;
}

static u64 sys_write(u64 fd, u64 buf, u64 count, u64, u64, u64) {
    KDEBUG("sys_write: fd=%llu, buf=%p, count=%llu", fd, (void*)buf, count);
    
    // Handle special file descriptors
    if (fd == 1 || fd == 2) { // stdout or stderr
        // Write to VGA console
        // TODO: Copy from user buffer and write to console
        vga_printf("Write to console: %llu bytes\n", count);
        return count;
    }
    
    // TODO: Implement actual file writing
    return -ENOSYS;
}

static u64 sys_lseek(u64 fd, u64 offset, u64 whence, u64, u64, u64) {
    KDEBUG("sys_lseek: fd=%llu, offset=%lld, whence=%llu", fd, (long long)offset, whence);
    // TODO: Implement file seeking
    return -ENOSYS;
}

static u64 sys_stat(u64 pathname, u64 statbuf, u64, u64, u64, u64) {
    KDEBUG("sys_stat: pathname=%p, statbuf=%p", (void*)pathname, (void*)statbuf);
    // TODO: Implement file status
    return -ENOSYS;
}

static u64 sys_fstat(u64 fd, u64 statbuf, u64, u64, u64, u64) {
    KDEBUG("sys_fstat: fd=%llu, statbuf=%p", fd, (void*)statbuf);
    // TODO: Implement file descriptor status
    return -ENOSYS;
}

static u64 sys_unlink(u64 pathname, u64, u64, u64, u64, u64) {
    KDEBUG("sys_unlink: pathname=%p", (void*)pathname);
    // TODO: Implement file removal
    return -ENOSYS;
}

static u64 sys_rename(u64 oldpath, u64 newpath, u64, u64, u64, u64) {
    KDEBUG("sys_rename: oldpath=%p, newpath=%p", (void*)oldpath, (void*)newpath);
    // TODO: Implement file rename
    return -ENOSYS;
}

// Directory operation syscalls
static u64 sys_mkdir(u64 pathname, u64 mode, u64, u64, u64, u64) {
    KDEBUG("sys_mkdir: pathname=%p, mode=%llu", (void*)pathname, mode);
    
    // TODO: Implement actual directory creation
    // For now, just return success
    return 0;
}

static u64 sys_rmdir(u64 pathname, u64, u64, u64, u64, u64) {
    KDEBUG("sys_rmdir: pathname=%p", (void*)pathname);
    // TODO: Implement directory removal
    return -ENOSYS;
}

static u64 sys_chdir(u64 path, u64, u64, u64, u64, u64) {
    KDEBUG("sys_chdir: path=%p", (void*)path);
    
    // TODO: Implement actual directory change
    // For now, just return success
    return 0;
}

static u64 sys_getcwd(u64 buf, u64 size, u64, u64, u64, u64) {
    KDEBUG("sys_getcwd: buf=%p, size=%llu", (void*)buf, size);
    
    // TODO: Implement actual current directory get
    // For now, return "/" as current directory
    const char* cwd = "/";
    size_t len = strlen(cwd) + 1;
    
    if (len > size) {
        return -ERANGE;
    }
    
    // TODO: Copy to user space
    // copy_to_user((char*)buf, cwd, len);
    
    return len - 1;
}

// Memory management syscalls
static u64 sys_brk(u64 addr, u64, u64, u64, u64, u64) {
    KDEBUG("sys_brk: addr=%p", (void*)addr);
    
    // TODO: Implement actual heap expansion
    // For now, return a placeholder break address
    static void* current_brk = (void*)0x1000000;
    if (addr != 0) {
        current_brk = (void*)addr;
    }
    return (u64)current_brk;
}

static u64 sys_mmap(u64 addr, u64 length, u64 prot, u64 flags, u64 fd, u64 offset) {
    KDEBUG("sys_mmap: addr=%p, length=%llu, prot=%llu, flags=%llu, fd=%llu, offset=%lld", 
           (void*)addr, length, prot, flags, fd, (long long)offset);
    
    // TODO: Implement actual memory mapping
    // For now, return a placeholder mapped address
    static void* next_mmap_addr = (void*)0x2000000;
    void* mapped_addr = next_mmap_addr;
    next_mmap_addr = (void*)((u64)next_mmap_addr + length);
    return (u64)mapped_addr;
}

static u64 sys_munmap(u64 addr, u64 length, u64, u64, u64, u64) {
    KDEBUG("sys_munmap: addr=%p, length=%llu", (void*)addr, length);
    
    // TODO: Implement actual memory unmapping
    // For now, just return success
    return 0;
}

static u64 sys_mprotect(u64 addr, u64 length, u64 prot, u64, u64, u64) {
    KDEBUG("sys_mprotect: addr=%p, length=%llu, prot=%llu", (void*)addr, length, prot);
    // TODO: Implement memory protection
    return -ENOSYS;
}

// Time syscalls
static u64 sys_time(u64 tloc, u64, u64, u64, u64, u64) {
    KDEBUG("sys_time called");
    
    // TODO: Implement actual system time
    // For now, return a placeholder timestamp
    time_t current_time = 1700000000; // Placeholder timestamp
    
    if (tloc) {
        // TODO: Copy to user space
        // copy_to_user((time_t*)tloc, &current_time, sizeof(time_t));
    }
    return current_time;
}

static u64 sys_gettimeofday(u64 tv, u64 tz, u64, u64, u64, u64) {
    KDEBUG("sys_gettimeofday called");
    
    // TODO: Implement actual microsecond time
    if (tv) {
        struct timeval timeval = {
            .tv_sec = 1700000000,  // Placeholder
            .tv_usec = 0           // Placeholder
        };
        // TODO: Copy to user space
        // copy_to_user((struct timeval*)tv, &timeval, sizeof(struct timeval));
    }
    return 0;
}

static u64 sys_nanosleep(u64 req, u64 rem, u64, u64, u64, u64) {
    KDEBUG("sys_nanosleep called");
    // TODO: Implement nanosecond sleep
    return -ENOSYS;
}

// Signal syscalls
static u64 sys_sigaction(u64 sig, u64 act, u64 oact, u64, u64, u64) {
    KDEBUG("sys_sigaction: sig=%llu", sig);
    // TODO: Implement signal action
    return -ENOSYS;
}

static u64 sys_sigprocmask(u64 how, u64 set, u64 oldset, u64, u64, u64) {
    KDEBUG("sys_sigprocmask: how=%llu", how);
    // TODO: Implement signal mask
    return -ENOSYS;
}

static u64 sys_sigsuspend(u64 mask, u64, u64, u64, u64, u64) {
    KDEBUG("sys_sigsuspend called");
    // TODO: Implement signal suspend
    return -ENOSYS;
}

// Network syscalls
static u64 sys_socket(u64 domain, u64 type, u64 protocol, u64, u64, u64) {
    KDEBUG("sys_socket: domain=%llu, type=%llu, protocol=%llu", domain, type, protocol);
    // TODO: Implement socket creation
    return -ENOSYS;
}

static u64 sys_bind(u64 sockfd, u64 addr, u64 addrlen, u64, u64, u64) {
    KDEBUG("sys_bind: sockfd=%llu", sockfd);
    // TODO: Implement socket binding
    return -ENOSYS;
}

static u64 sys_listen(u64 sockfd, u64 backlog, u64, u64, u64, u64) {
    KDEBUG("sys_listen: sockfd=%llu, backlog=%llu", sockfd, backlog);
    // TODO: Implement socket listening
    return -ENOSYS;
}

static u64 sys_accept(u64 sockfd, u64 addr, u64 addrlen, u64, u64, u64) {
    KDEBUG("sys_accept: sockfd=%llu", sockfd);
    // TODO: Implement socket acceptance
    return -ENOSYS;
}

static u64 sys_connect(u64 sockfd, u64 addr, u64 addrlen, u64, u64, u64) {
    KDEBUG("sys_connect: sockfd=%llu", sockfd);
    // TODO: Implement socket connection
    return -ENOSYS;
}

static u64 sys_send(u64 sockfd, u64 buf, u64 len, u64 flags, u64, u64) {
    KDEBUG("sys_send: sockfd=%llu, len=%llu", sockfd, len);
    // TODO: Implement socket sending
    return -ENOSYS;
}

static u64 sys_recv(u64 sockfd, u64 buf, u64 len, u64 flags, u64, u64) {
    KDEBUG("sys_recv: sockfd=%llu, len=%llu", sockfd, len);
    // TODO: Implement socket receiving
    return -ENOSYS;
}

// Process scheduling syscalls
static u64 sys_sched_getparam(u64 pid, u64 param, u64, u64, u64, u64) {
    KDEBUG("sys_sched_getparam: pid=%llu", pid);
    // TODO: Implement scheduler parameter get
    return -ENOSYS;
}

static u64 sys_sched_setparam(u64 pid, u64 param, u64, u64, u64, u64) {
    KDEBUG("sys_sched_setparam: pid=%llu", pid);
    // TODO: Implement scheduler parameter set
    return -ENOSYS;
}

// System information syscalls
static u64 sys_sysconf(u64 name, u64, u64, u64, u64, u64) {
    KDEBUG("sys_sysconf: name=%llu", name);
    // TODO: Implement system configuration
    return -ENOSYS;
}

static u64 sys_getrlimit(u64 resource, u64 rlim, u64, u64, u64, u64) {
    KDEBUG("sys_getrlimit: resource=%llu", resource);
    // TODO: Implement resource limit get
    return -ENOSYS;
}

static u64 sys_setrlimit(u64 resource, u64 rlim, u64, u64, u64, u64) {
    KDEBUG("sys_setrlimit: resource=%llu", resource);
    // TODO: Implement resource limit set
    return -ENOSYS;
}

static u64 sys_getrusage(u64 who, u64 usage, u64, u64, u64, u64) {
    KDEBUG("sys_getrusage: who=%llu", who);
    // TODO: Implement resource usage get
    return -ENOSYS;
}