#include "include/syscall.h"
#include "../core/include/kernel.h"
#include "../memory/include/memory.h"
#include "../process/include/process.h"
#include "../hal/include/hal.h"
#include "../memory/memory_integration.h"

// Global system call state
syscall_info_t syscall_table[MAX_SYSCALLS];
syscall_stats_t syscall_statistics = {0};
bool syscall_initialized = false;

// Helper macros for extracting syscall arguments
#define ARG1(args) ((args)->rdi)
#define ARG2(args) ((args)->rsi)
#define ARG3(args) ((args)->rdx)
#define ARG4(args) ((args)->r10)
#define ARG5(args) ((args)->r8)
#define ARG6(args) ((args)->r9)

// Initialize system call interface
error_t syscall_init(void) {
    if (syscall_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing production system call interface");
    
    // Clear syscall table and statistics
    memset(syscall_table, 0, sizeof(syscall_table));
    memset(&syscall_statistics, 0, sizeof(syscall_statistics));
    
    // Register core POSIX system calls
    
    // File system operations
    syscall_register(SYS_read, "read", sys_read, 3, true, PERM_FILE_READ);
    syscall_register(SYS_write, "write", sys_write, 3, true, PERM_FILE_WRITE);
    syscall_register(SYS_open, "open", sys_open, 3, true, PERM_FILE_READ);
    syscall_register(SYS_close, "close", sys_close, 1, false, PERM_NONE);
    syscall_register(SYS_stat, "stat", sys_stat, 2, true, PERM_FILE_READ);
    syscall_register(SYS_lseek, "lseek", sys_lseek, 3, false, PERM_NONE);
    syscall_register(SYS_access, "access", sys_access, 2, true, PERM_FILE_READ);
    syscall_register(SYS_mkdir, "mkdir", sys_mkdir, 2, true, PERM_FILE_WRITE);
    syscall_register(SYS_rmdir, "rmdir", sys_rmdir, 1, true, PERM_FILE_WRITE);
    syscall_register(SYS_unlink, "unlink", sys_unlink, 1, true, PERM_FILE_WRITE);
    syscall_register(SYS_chmod, "chmod", sys_chmod, 2, true, PERM_FILE_WRITE);
    syscall_register(SYS_chown, "chown", sys_chown, 3, true, PERM_SYSTEM_ADMIN);
    syscall_register(SYS_getcwd, "getcwd", sys_getcwd, 2, false, PERM_NONE);
    syscall_register(SYS_chdir, "chdir", sys_chdir, 1, true, PERM_FILE_READ);
    
    // Process management
    syscall_register(SYS_fork, "fork", sys_fork, 0, true, PERM_PROCESS_CREATE);
    syscall_register(SYS_execve, "execve", sys_execve, 3, true, PERM_PROCESS_CREATE);
    syscall_register(SYS_exit, "exit", sys_exit, 1, false, PERM_NONE);
    syscall_register(SYS_wait4, "wait4", sys_wait4, 4, false, PERM_NONE);
    syscall_register(SYS_getpid, "getpid", sys_getpid, 0, false, PERM_NONE);
    syscall_register(SYS_getppid, "getppid", sys_getppid, 0, false, PERM_NONE);
    syscall_register(SYS_kill, "kill", sys_kill, 2, true, PERM_PROCESS_KILL);
    syscall_register(SYS_sched_yield, "sched_yield", sys_sched_yield, 0, false, PERM_NONE);
    
    // Memory management
    syscall_register(SYS_mmap, "mmap", sys_mmap, 6, true, PERM_MEMORY_MANAGE);
    syscall_register(SYS_munmap, "munmap", sys_munmap, 2, true, PERM_MEMORY_MANAGE);
    syscall_register(SYS_mprotect, "mprotect", sys_mprotect, 3, true, PERM_MEMORY_MANAGE);
    syscall_register(SYS_brk, "brk", sys_brk, 1, true, PERM_MEMORY_MANAGE);
    
    // User/Group management
    syscall_register(SYS_getuid, "getuid", sys_getuid, 0, false, PERM_NONE);
    syscall_register(SYS_getgid, "getgid", sys_getgid, 0, false, PERM_NONE);
    syscall_register(SYS_geteuid, "geteuid", sys_geteuid, 0, false, PERM_NONE);
    syscall_register(SYS_getegid, "getegid", sys_getegid, 0, false, PERM_NONE);
    syscall_register(SYS_setuid, "setuid", sys_setuid, 1, true, PERM_SYSTEM_ADMIN);
    syscall_register(SYS_setgid, "setgid", sys_setgid, 1, true, PERM_SYSTEM_ADMIN);
    
    // Time operations
    syscall_register(SYS_gettimeofday, "gettimeofday", sys_gettimeofday, 2, false, PERM_NONE);
    syscall_register(SYS_nanosleep, "nanosleep", sys_nanosleep, 2, false, PERM_NONE);
    
    // System information
    syscall_register(SYS_uname, "uname", sys_uname, 1, false, PERM_NONE);
    syscall_register(SYS_sysinfo, "sysinfo", sys_sysinfo, 1, false, PERM_NONE);
    syscall_register(SYS_getrusage, "getrusage", sys_getrusage, 2, false, PERM_NONE);
    
    // RaeenOS-specific system calls
    syscall_register(SYS_raeen_game_mode, "raeen_game_mode", sys_raeen_game_mode, 2, false, PERM_NONE);
    syscall_register(SYS_raeen_set_priority, "raeen_set_priority", sys_raeen_set_priority, 2, true, PERM_PROCESS_KILL);
    syscall_register(SYS_raeen_get_stats, "raeen_get_stats", sys_raeen_get_stats, 2, false, PERM_NONE);
    syscall_register(SYS_raeen_memory_info, "raeen_memory_info", sys_raeen_memory_info, 1, false, PERM_NONE);
    syscall_register(SYS_raeen_process_info, "raeen_process_info", sys_raeen_process_info, 2, false, PERM_NONE);
    syscall_register(SYS_raeen_device_info, "raeen_device_info", sys_raeen_device_info, 2, false, PERM_NONE);
    syscall_register(SYS_raeen_security_set, "raeen_security_set", sys_raeen_security_set, 2, true, PERM_SYSTEM_ADMIN);
    syscall_register(SYS_raeen_ai_optimize, "raeen_ai_optimize", sys_raeen_ai_optimize, 1, false, PERM_NONE);
    syscall_register(SYS_raeen_theme_set, "raeen_theme_set", sys_raeen_theme_set, 2, false, PERM_NONE);
    syscall_register(SYS_raeen_compatibility, "raeen_compatibility", sys_raeen_compatibility, 3, false, PERM_NONE);
    
    syscall_initialized = true;
    
    KINFO("System call interface initialized with %d registered syscalls", MAX_SYSCALLS);
    return SUCCESS;
}

// Register a system call
error_t syscall_register(u32 syscall_num, const char* name, syscall_handler_t handler, 
                         u32 arg_count, bool requires_permission, u32 permission) {
    if (syscall_num >= MAX_SYSCALLS) {
        return E_INVAL;
    }
    
    if (!name || !handler) {
        return E_INVAL;
    }
    
    syscall_info_t* info = &syscall_table[syscall_num];
    info->name = name;
    info->handler = handler;
    info->arg_count = arg_count;
    info->requires_permission = requires_permission;
    info->required_permission = permission;
    
    KDEBUG("Registered syscall %u: %s (args=%u, perm=0x%x)", 
           syscall_num, name, arg_count, permission);
    
    return SUCCESS;
}

// Main system call handler (called from interrupt)
void syscall_handler(interrupt_context_t* context) {
    u64 start_time = hal_get_timestamp();
    
    // Extract system call number from RAX
    u32 syscall_num = (u32)context->rax;
    
    // Extract arguments from registers (System V ABI)
    syscall_args_t args = {
        .rdi = context->rdi,  // arg1
        .rsi = context->rsi,  // arg2
        .rdx = context->rdx,  // arg3
        .r10 = context->r10,  // arg4 (note: r10 instead of rcx for syscalls)
        .r8  = context->r8,   // arg5
        .r9  = context->r9    // arg6
    };
    
    // Update statistics
    syscall_statistics.total_syscalls++;
    if (syscall_num < MAX_SYSCALLS) {
        syscall_statistics.syscall_counts[syscall_num]++;
    }
    
    // Validate system call number
    if (!syscall_is_valid(syscall_num)) {
        KWARN("Invalid system call number: %u", syscall_num);
        context->rax = -ENOSYS;  // No such system call
        syscall_statistics.failed_syscalls++;
        return;
    }
    
    // Get current process
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        KERROR("System call from process with no current process context");
        context->rax = -ESRCH;   // No such process
        syscall_statistics.failed_syscalls++;
        return;
    }
    
    // Check permissions
    error_t perm_result = syscall_check_permission(current_proc, syscall_num);
    if (perm_result != SUCCESS) {
        KWARN("Permission denied for syscall %u (%s) by process PID=%u", 
              syscall_num, syscall_get_name(syscall_num), current_proc->pid);
        context->rax = -EPERM;   // Operation not permitted
        syscall_statistics.failed_syscalls++;
        return;
    }
    
    // Get system call handler
    syscall_info_t* info = &syscall_table[syscall_num];
    if (!info->handler) {
        KWARN("No handler for system call %u", syscall_num);
        context->rax = -ENOSYS;
        syscall_statistics.failed_syscalls++;
        return;
    }
    
    // Log system call for debugging (only in debug mode)
    KDEBUG("Syscall %u (%s) by PID=%u: args=[0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx]",
           syscall_num, info->name, current_proc->pid,
           args.rdi, args.rsi, args.rdx, args.r10, args.r8, args.r9);
    
    // Call the system call handler
    long result = info->handler(&args);
    
    // Store result in RAX
    context->rax = result;
    
    // Update statistics
    if (result >= 0) {
        syscall_statistics.successful_syscalls++;
    } else {
        syscall_statistics.failed_syscalls++;
    }
    
    // Update timing statistics
    u64 end_time = hal_get_timestamp();
    u64 execution_time = end_time - start_time;
    
    if (execution_time > syscall_statistics.max_execution_time_ns) {
        syscall_statistics.max_execution_time_ns = execution_time;
    }
    
    // Update average (simple moving average)
    syscall_statistics.avg_execution_time_ns = 
        (syscall_statistics.avg_execution_time_ns + execution_time) / 2;
    
    // Update current thread statistics
    thread_t* current = get_current_thread();
    if (current) {
        current->system_calls++;
    }
}