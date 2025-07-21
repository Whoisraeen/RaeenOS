#include "include/syscall.h"
#include "../core/include/kernel.h"
#include "../memory/include/memory.h"
#include "../process/include/process.h"
#include "../hal/include/hal.h"
#include "../memory/memory_integration.h"

// Helper macros
#define ARG1(args) ((args)->rdi)
#define ARG2(args) ((args)->rsi)
#define ARG3(args) ((args)->rdx)
#define ARG4(args) ((args)->r10)
#define ARG5(args) ((args)->r8)
#define ARG6(args) ((args)->r9)

// Utility functions
bool syscall_is_valid(u32 syscall_num) {
    return (syscall_num < MAX_SYSCALLS && syscall_table[syscall_num].handler != NULL);
}

const char* syscall_get_name(u32 syscall_num) {
    if (syscall_num >= MAX_SYSCALLS) {
        return "invalid";
    }
    
    const char* name = syscall_table[syscall_num].name;
    return name ? name : "unknown";
}

// Permission checking
bool syscall_has_permission(process_t* process, u32 permission) {
    if (!process || !process->security_token) {
        return false;
    }
    
    // Root user has all permissions
    if (process->uid == 0) {
        return true;
    }
    
    // Check if process has the required permission
    return (process->security_token->permissions & permission) != 0;
}

error_t syscall_check_permission(process_t* process, u32 syscall_num) {
    if (syscall_num >= MAX_SYSCALLS) {
        return E_INVAL;
    }
    
    syscall_info_t* info = &syscall_table[syscall_num];
    
    if (!info->requires_permission) {
        return SUCCESS;  // No permission required
    }
    
    if (syscall_has_permission(process, info->required_permission)) {
        return SUCCESS;
    }
    
    return E_PERM;  // Permission denied
}

// Memory validation for system calls
bool syscall_validate_user_pointer(const void* ptr, size_t size, bool write) {
    if (!ptr || size == 0) {
        return false;
    }
    
    virt_addr_t addr = (virt_addr_t)ptr;
    
    // Check if address is in user space
    if (addr >= KERNEL_SPACE_START) {
        return false;  // Kernel space access not allowed
    }
    
    // Check if the entire range is valid
    process_t* current_proc = get_current_process();
    if (!current_proc || !current_proc->address_space) {
        return false;
    }
    
    // Find VMA that contains this address
    vma_t* vma = vmm_find_vma(current_proc->address_space, addr);
    if (!vma) {
        return false;  // No VMA found
    }
    
    // Check if the entire range is within the VMA
    if (addr + size > vma->end) {
        return false;  // Range extends beyond VMA
    }
    
    // Check permissions
    if (write && !(vma->protection & VMA_PROT_WRITE)) {
        return false;  // Write access not allowed
    }
    
    if (!(vma->protection & VMA_PROT_READ)) {
        return false;  // Read access not allowed
    }
    
    return true;
}

bool syscall_validate_user_string(const char* str, size_t max_len) {
    if (!str) {
        return false;
    }
    
    // Check if string pointer is valid
    if (!syscall_validate_user_pointer(str, 1, false)) {
        return false;
    }
    
    // Find the actual length and validate each page
    size_t len = 0;
    const char* ptr = str;
    
    while (len < max_len) {
        // Validate current page
        if (!syscall_validate_user_pointer(ptr, 1, false)) {
            return false;
        }
        
        // Check for null terminator
        if (*ptr == '\0') {
            return true;
        }
        
        ptr++;
        len++;
        
        // Check page boundary
        if (((virt_addr_t)ptr & (PAGE_SIZE - 1)) == 0) {
            // Crossed page boundary, revalidate
            if (!syscall_validate_user_pointer(ptr, 1, false)) {
                return false;
            }
        }
    }
    
    return false;  // String too long
}

// Process management system calls
long sys_getpid(syscall_args_t* args) {
    (void)args;  // Unused
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->pid;
}

long sys_getppid(syscall_args_t* args) {
    (void)args;  // Unused
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->parent_pid;
}

long sys_exit(syscall_args_t* args) {
    int exit_code = (int)ARG1(args);
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    KDEBUG("Process PID=%u exiting with code %d", current_proc->pid, exit_code);
    
    // Exit the process
    process_exit(current_proc, exit_code);
    
    // This should not return
    return 0;
}

long sys_fork(syscall_args_t* args) {
    (void)args;  // Unused
    
    process_t* parent = get_current_process();
    if (!parent) {
        return -ESRCH;
    }
    
    // Create child process
    process_t* child = process_create(parent->name, parent->base_priority, 
                                      parent->uid, parent->gid);
    if (!child) {
        return -ENOMEM;
    }
    
    // Clone address space
    child->address_space = vmm_clone_address_space(parent->address_space);
    if (!child->address_space) {
        process_destroy(child);
        return -ENOMEM;
    }
    
    // Create main thread for child
    thread_t* child_thread = thread_create(child, NULL, NULL, KERNEL_STACK_SIZE);
    if (!child_thread) {
        process_destroy(child);
        return -ENOMEM;
    }
    
    // Copy thread context from current thread
    thread_t* current_thread = get_current_thread();
    if (current_thread) {
        child_thread->context = current_thread->context;
        // Child returns 0, parent returns child PID
        child_thread->context.rax = 0;
    }
    
    child->main_thread = child_thread;
    child_thread->state = THREAD_STATE_READY;
    
    // Add child to scheduler
    scheduler_add_thread(child_thread);
    
    KDEBUG("Forked process: parent PID=%u, child PID=%u", parent->pid, child->pid);
    
    return child->pid;  // Return child PID to parent
}

long sys_sched_yield(syscall_args_t* args) {
    (void)args;  // Unused
    
    scheduler_yield();
    return 0;
}

// User/Group ID system calls
long sys_getuid(syscall_args_t* args) {
    (void)args;
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->uid;
}

long sys_getgid(syscall_args_t* args) {
    (void)args;
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->gid;
}

long sys_geteuid(syscall_args_t* args) {
    (void)args;
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->euid;
}

long sys_getegid(syscall_args_t* args) {
    (void)args;
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    return current_proc->egid;
}

long sys_setuid(syscall_args_t* args) {
    uid_t uid = (uid_t)ARG1(args);
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    // Only root can change UID arbitrarily
    if (current_proc->uid != 0 && current_proc->uid != uid) {
        return -EPERM;
    }
    
    current_proc->uid = uid;
    current_proc->euid = uid;
    
    return 0;
}

long sys_setgid(syscall_args_t* args) {
    gid_t gid = (gid_t)ARG1(args);
    
    process_t* current_proc = get_current_process();
    if (!current_proc) {
        return -ESRCH;
    }
    
    // Only root can change GID arbitrarily
    if (current_proc->uid != 0 && current_proc->gid != gid) {
        return -EPERM;
    }
    
    current_proc->gid = gid;
    current_proc->egid = gid;
    
    return 0;
}

// Time operations
long sys_gettimeofday(syscall_args_t* args) {
    struct timeval* tv = (struct timeval*)ARG1(args);
    struct timezone* tz = (struct timezone*)ARG2(args);
    
    if (tv && !syscall_validate_user_pointer(tv, sizeof(struct timeval), true)) {
        return -EFAULT;
    }
    
    if (tz && !syscall_validate_user_pointer(tz, sizeof(struct timezone), true)) {
        return -EFAULT;
    }
    
    u64 timestamp = hal_get_timestamp();
    
    if (tv) {
        tv->tv_sec = timestamp / 1000000;
        tv->tv_usec = timestamp % 1000000;
    }
    
    if (tz) {
        tz->tz_minuteswest = 0;  // UTC
        tz->tz_dsttime = 0;      // No DST
    }
    
    return 0;
}

long sys_nanosleep(syscall_args_t* args) {
    const struct timespec* req = (const struct timespec*)ARG1(args);
    struct timespec* rem = (struct timespec*)ARG2(args);
    
    if (!req || !syscall_validate_user_pointer(req, sizeof(struct timespec), false)) {
        return -EFAULT;
    }
    
    if (rem && !syscall_validate_user_pointer(rem, sizeof(struct timespec), true)) {
        return -EFAULT;
    }
    
    u64 sleep_ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;
    
    if (sleep_ms == 0) {
        return 0;  // No sleep needed
    }
    
    error_t result = thread_sleep(sleep_ms);
    if (result != SUCCESS) {
        return -EINTR;  // Interrupted
    }
    
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    
    return 0;
}

// RaeenOS-specific system calls
long sys_raeen_game_mode(syscall_args_t* args) {
    pid_t pid = (pid_t)ARG1(args);
    bool enable = (bool)ARG2(args);
    
    process_t* target_proc;
    
    if (pid == 0) {
        target_proc = get_current_process();
    } else {
        target_proc = process_find_by_pid(pid);
    }
    
    if (!target_proc) {
        return -ESRCH;
    }
    
    error_t result;
    if (enable) {
        result = process_enable_game_mode(target_proc);
    } else {
        result = process_disable_game_mode(target_proc);
    }
    
    return (result == SUCCESS) ? 0 : -EINVAL;
}

long sys_raeen_set_priority(syscall_args_t* args) {
    pid_t pid = (pid_t)ARG1(args);
    int priority = (int)ARG2(args);
    
    if (priority < PRIORITY_CRITICAL || priority > PRIORITY_IDLE) {
        return -EINVAL;
    }
    
    process_t* target_proc;
    
    if (pid == 0) {
        target_proc = get_current_process();
    } else {
        target_proc = process_find_by_pid(pid);
    }
    
    if (!target_proc) {
        return -ESRCH;
    }
    
    error_t result = process_set_priority(target_proc, (process_priority_t)priority);
    return (result == SUCCESS) ? 0 : -EPERM;
}

long sys_raeen_get_stats(syscall_args_t* args) {
    void* stats_buffer = (void*)ARG1(args);
    size_t buffer_size = (size_t)ARG2(args);
    
    if (!stats_buffer || !syscall_validate_user_pointer(stats_buffer, buffer_size, true)) {
        return -EFAULT;
    }
    
    // Get system statistics
    scheduler_stats_t* sched_stats = scheduler_get_stats();
    
    if (buffer_size < sizeof(scheduler_stats_t)) {
        return -EINVAL;
    }
    
    memcpy(stats_buffer, sched_stats, sizeof(scheduler_stats_t));
    return sizeof(scheduler_stats_t);
}

long sys_raeen_memory_info(syscall_args_t* args) {
    void* info_buffer = (void*)ARG1(args);
    
    if (!info_buffer || !syscall_validate_user_pointer(info_buffer, sizeof(vmm_stats_t), true)) {
        return -EFAULT;
    }
    
    vmm_stats_t stats;
    error_t result = vmm_get_stats(&stats);
    if (result != SUCCESS) {
        return -EINVAL;
    }
    
    memcpy(info_buffer, &stats, sizeof(vmm_stats_t));
    return sizeof(vmm_stats_t);
}

long sys_raeen_process_info(syscall_args_t* args) {
    pid_t pid = (pid_t)ARG1(args);
    void* info_buffer = (void*)ARG2(args);
    
    if (!info_buffer || !syscall_validate_user_pointer(info_buffer, sizeof(process_t), true)) {
        return -EFAULT;
    }
    
    process_t* target_proc;
    
    if (pid == 0) {
        target_proc = get_current_process();
    } else {
        target_proc = process_find_by_pid(pid);
    }
    
    if (!target_proc) {
        return -ESRCH;
    }
    
    // Copy process information (excluding sensitive data)
    process_t safe_copy = *target_proc;
    safe_copy.security_token = NULL;  // Don't expose security token
    safe_copy.address_space = NULL;   // Don't expose address space
    
    memcpy(info_buffer, &safe_copy, sizeof(process_t));
    return sizeof(process_t);
}

// Stub implementations for file system calls (to be implemented when filesystem is ready)
long sys_read(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_write(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_open(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_close(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_stat(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_lseek(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_access(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_mkdir(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_rmdir(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_unlink(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_chmod(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_chown(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_getcwd(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_chdir(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_execve(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_wait4(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_kill(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_mmap(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_munmap(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_mprotect(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_brk(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_uname(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_sysinfo(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_getrusage(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

// RaeenOS-specific stubs
long sys_raeen_device_info(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_raeen_security_set(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_raeen_ai_optimize(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_raeen_theme_set(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

long sys_raeen_compatibility(syscall_args_t* args) {
    (void)args;
    return -ENOSYS;  // Not implemented yet
}

// Statistics
syscall_stats_t* syscall_get_stats(void) {
    return &syscall_statistics;
}

void syscall_dump_stats(void) {
    KINFO("=== System Call Statistics ===");
    KINFO("Total syscalls:     %llu", syscall_statistics.total_syscalls);
    KINFO("Successful:         %llu", syscall_statistics.successful_syscalls);
    KINFO("Failed:             %llu", syscall_statistics.failed_syscalls);
    KINFO("Average exec time:  %llu ns", syscall_statistics.avg_execution_time_ns);
    KINFO("Max exec time:      %llu ns", syscall_statistics.max_execution_time_ns);
    
    KINFO("\nTop 10 most used system calls:");
    
    // Find top 10 most used syscalls
    for (int i = 0; i < 10; i++) {
        u64 max_count = 0;
        u32 max_syscall = 0;
        
        for (u32 j = 0; j < MAX_SYSCALLS; j++) {
            if (syscall_statistics.syscall_counts[j] > max_count) {
                max_count = syscall_statistics.syscall_counts[j];
                max_syscall = j;
            }
        }
        
        if (max_count > 0) {
            KINFO("  %s (%u): %llu calls", 
                  syscall_get_name(max_syscall), max_syscall, max_count);
            syscall_statistics.syscall_counts[max_syscall] = 0; // Clear for next iteration
        }
    }
}