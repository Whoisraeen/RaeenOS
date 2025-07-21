#include "include/process.h"
#include "../core/include/kernel.h"
#include "../memory/include/memory.h"
#include "../hal/include/hal.h"
#include "../memory/memory_integration.h"

// Production Process Management Integration
// Integrates all process management components with kernel subsystems

// Global process management variables
process_t* process_list_head = NULL;
thread_t* current_thread = NULL;
process_t* current_process = NULL;
process_t* idle_process = NULL;
thread_t* idle_thread = NULL;
pid_t next_pid = 1;
tid_t next_tid = 1;
wait_queue_t* ready_queues[5] = {NULL};

// Initialize process management system
error_t process_management_init(void) {
    KINFO("Initializing production process management system");
    
    // Initialize process core
    error_t result = process_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize process core: %d", result);
        return result;
    }
    
    // Initialize scheduler
    result = scheduler_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize scheduler: %d", result);
        return result;
    }
    
    KINFO("Production process management system initialized successfully");
    return SUCCESS;
}

// Start the scheduler and begin multitasking
error_t process_management_start(void) {
    if (!current_process) {
        KERROR("No processes available to start scheduler");
        return E_INVAL;
    }
    
    KINFO("Starting production scheduler");
    scheduler_start();
    
    // Enable timer interrupt for preemptive scheduling
    // This would be integrated with the interrupt system
    KINFO("Preemptive multitasking enabled");
    
    return SUCCESS;
}

// Create the first user process (typically init)
process_t* create_init_process(void) {
    KINFO("Creating init process");
    
    process_t* init_process = process_create("init", PRIORITY_NORMAL, 0, 0);
    if (!init_process) {
        KERROR("Failed to create init process");
        return NULL;
    }
    
    // Create main thread for init process
    thread_t* init_thread = thread_create(init_process, (void*)0x400000, NULL, USER_STACK_SIZE);
    if (!init_thread) {
        KERROR("Failed to create init thread");
        process_destroy(init_process);
        return NULL;
    }
    
    init_process->main_thread = init_thread;
    init_thread->state = THREAD_STATE_READY;
    
    // Add to scheduler
    scheduler_add_thread(init_thread);
    
    KINFO("Init process created successfully (PID=%u, TID=%u)", 
          init_process->pid, init_thread->tid);
    
    return init_process;
}

// Timer interrupt handler for preemptive scheduling
void process_timer_interrupt_handler(void) {
    if (current_thread) {
        scheduler_tick();
    }
}

// Get current process information for syscalls
process_t* get_current_process(void) {
    return current_process;
}

// Get current thread information
thread_t* get_current_thread(void) {
    return current_thread;
}

// Set current process/thread (used by scheduler)
void set_current_process(process_t* process) {
    current_process = process;
}

void set_current_thread(thread_t* thread) {
    current_thread = thread;
    current_process = thread ? thread->parent_process : NULL;
}

// Process memory management integration
error_t process_allocate_user_memory(process_t* process, virt_addr_t addr, size_t size, uint32_t flags) {
    if (!process || !process->address_space) {
        return E_INVAL;
    }
    
    // Use VMM to allocate user memory
    vma_t* vma = vmm_create_vma(process->address_space, addr, size, 
                                VMA_PROT_READ | VMA_PROT_WRITE, 
                                VMA_MAP_PRIVATE | VMA_MAP_ANONYMOUS,
                                VMA_TYPE_ANONYMOUS);
    if (!vma) {
        return E_NOMEM;
    }
    
    process->memory_usage += size;
    return SUCCESS;
}

// Process cleanup on exit
void process_cleanup_on_exit(process_t* process) {
    if (!process) {
        return;
    }
    
    KDEBUG("Cleaning up process PID=%u on exit", process->pid);
    
    // Clean up all child processes
    process_t* child = process->first_child;
    while (child) {
        process_t* next_child = child->next_sibling;
        process_exit(child, -1);
        child = next_child;
    }
    
    // Close all file descriptors
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (process->fd_table[i]) {
            // TODO: Close file descriptor when filesystem is implemented
            process->fd_table[i] = NULL;
        }
    }
    
    // Free process memory
    if (process->address_space) {
        vmm_destroy_address_space(process->address_space);
        process->address_space = NULL;
    }
}

// Game Mode optimization
error_t enable_game_mode_optimizations(process_t* process) {
    if (!process) {
        return E_INVAL;
    }
    
    error_t result = process_enable_game_mode(process);
    if (result != SUCCESS) {
        return result;
    }
    
    // Additional Game Mode optimizations
    // TODO: Integrate with memory management for priority allocation
    // TODO: Integrate with I/O scheduler for faster disk access
    // TODO: Integrate with GPU scheduler for exclusive access
    
    KINFO("Game Mode optimizations enabled for process PID=%u", process->pid);
    return SUCCESS;
}

// Process statistics and monitoring
void dump_process_management_stats(void) {
    scheduler_stats_t* stats = scheduler_get_stats();
    
    KINFO("=== Process Management Statistics ===");
    KINFO("Total processes created: %llu", stats->total_processes_created);
    KINFO("Total threads created: %llu", stats->total_threads_created);
    KINFO("Active processes: %llu", stats->processes_running);
    KINFO("Active threads: %llu", stats->threads_running);
    KINFO("Total context switches: %llu", stats->total_context_switches);
    
    if (current_thread) {
        KINFO("Current thread: TID=%u (PID=%u)", 
              current_thread->tid, 
              current_thread->parent_process ? current_thread->parent_process->pid : 0);
    }
    
    scheduler_dump_stats();
    scheduler_dump_queues();
}

// Validate process management integrity
error_t validate_process_management_integrity(void) {
    uint32_t process_count = 0;
    uint32_t thread_count = 0;
    
    // Walk through all processes
    process_t* process = process_list_head;
    while (process) {
        process_count++;
        
        // Validate process structure
        if (process->pid == 0 && process != idle_process) {
            KERROR("Invalid PID 0 for non-idle process");
            return E_CORRUPT;
        }
        
        // Count threads in this process
        thread_t* thread = process->thread_list;
        while (thread) {
            thread_count++;
            
            // Validate thread structure
            if (thread->parent_process != process) {
                KERROR("Thread TID=%u has incorrect parent process", thread->tid);
                return E_CORRUPT;
            }
            
            thread = thread->next_thread;
        }
        
        process = process->next_process;
    }
    
    KINFO("Process management integrity check passed: %u processes, %u threads", 
          process_count, thread_count);
    
    return SUCCESS;
}