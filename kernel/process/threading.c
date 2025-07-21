#include "threading.h"
#include "process/process.h"
#include "memory/memory.h"
#include "syscalls/syscalls.h"
#include <string.h>
#include <stdio.h>

// Thread management state
static thread_t* thread_list = NULL;
static tid_t next_tid = 1;
static bool threading_initialized = false;

// Thread synchronization primitives
static mutex_t* mutex_list = NULL;
static semaphore_t* semaphore_list = NULL;
static condition_t* condition_list = NULL;

// Thread-local storage
#define TLS_SIZE 4096
static void* tls_pool = NULL;
static size_t tls_pool_size = 0;
static size_t tls_pool_used = 0;

// Initialize threading system
error_t threading_init(void) {
    if (threading_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing threading system");
    
    // Initialize thread-local storage pool
    tls_pool_size = TLS_SIZE * 1024; // 4MB for TLS
    tls_pool = memory_alloc(tls_pool_size);
    if (!tls_pool) {
        KERROR("Failed to allocate TLS pool");
        return E_NOMEM;
    }
    
    memset(tls_pool, 0, tls_pool_size);
    
    // Initialize synchronization primitives
    mutex_list = NULL;
    semaphore_list = NULL;
    condition_list = NULL;
    
    threading_initialized = true;
    
    KINFO("Threading system initialized");
    return SUCCESS;
}

// Shutdown threading system
void threading_shutdown(void) {
    if (!threading_initialized) {
        return;
    }
    
    KINFO("Shutting down threading system");
    
    // Free all threads
    thread_t* thread = thread_list;
    while (thread) {
        thread_t* next = thread->next;
        thread_destroy(thread->tid);
        thread = next;
    }
    
    // Free synchronization primitives
    mutex_t* mutex = mutex_list;
    while (mutex) {
        mutex_t* next = mutex->next;
        mutex_destroy(mutex->id);
        mutex = next;
    }
    
    semaphore_t* sem = semaphore_list;
    while (sem) {
        semaphore_t* next = sem->next;
        semaphore_destroy(sem->id);
        sem = next;
    }
    
    condition_t* cond = condition_list;
    while (cond) {
        condition_t* next = cond->next;
        condition_destroy(cond->id);
        cond = next;
    }
    
    // Free TLS pool
    if (tls_pool) {
        memory_free(tls_pool);
        tls_pool = NULL;
    }
    
    threading_initialized = false;
    
    KINFO("Threading system shutdown complete");
}

// Create a new thread
error_t thread_create(process_t* process, void* entry_point, void* arg, 
                     thread_priority_t priority, thread_t** thread) {
    if (!process || !entry_point || !thread) {
        return E_INVAL;
    }
    
    // Allocate thread structure
    thread_t* new_thread = (thread_t*)memory_alloc(sizeof(thread_t));
    if (!new_thread) {
        return E_NOMEM;
    }
    
    // Initialize thread
    memset(new_thread, 0, sizeof(thread_t));
    new_thread->tid = get_next_thread_id();
    new_thread->process = process;
    new_thread->entry_point = (uintptr_t)entry_point;
    new_thread->arg = arg;
    new_thread->priority = priority;
    new_thread->state = THREAD_STATE_NEW;
    new_thread->stack_size = 8192; // 8KB default stack
    
    // Allocate thread stack
    new_thread->stack = memory_alloc(new_thread->stack_size);
    if (!new_thread->stack) {
        memory_free(new_thread);
        return E_NOMEM;
    }
    
    // Allocate thread-local storage
    new_thread->tls = thread_alloc_tls();
    if (!new_thread->tls) {
        memory_free(new_thread->stack);
        memory_free(new_thread);
        return E_NOMEM;
    }
    
    // Set up thread context
    error_t result = thread_setup_context(new_thread);
    if (result != SUCCESS) {
        thread_free_tls(new_thread->tls);
        memory_free(new_thread->stack);
        memory_free(new_thread);
        return result;
    }
    
    // Add to thread list
    new_thread->next = thread_list;
    if (thread_list) {
        thread_list->prev = new_thread;
    }
    thread_list = new_thread;
    
    // Add to process thread list
    new_thread->next_in_process = process->threads;
    if (process->threads) {
        process->threads->prev_in_process = new_thread;
    }
    process->threads = new_thread;
    process->thread_count++;
    
    // Set thread state
    new_thread->state = THREAD_STATE_READY;
    
    *thread = new_thread;
    
    KDEBUG("Created thread TID=%u in process PID=%u", new_thread->tid, process->pid);
    return SUCCESS;
}

// Destroy a thread
error_t thread_destroy(tid_t tid) {
    thread_t* thread = thread_get_by_tid(tid);
    if (!thread) {
        return E_NOENT;
    }
    
    KDEBUG("Destroying thread TID=%u", tid);
    
    // Remove from thread list
    if (thread->prev) {
        thread->prev->next = thread->next;
    } else {
        thread_list = thread->next;
    }
    
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    
    // Remove from process thread list
    if (thread->prev_in_process) {
        thread->prev_in_process->next_in_process = thread->next_in_process;
    } else {
        thread->process->threads = thread->next_in_process;
    }
    
    if (thread->next_in_process) {
        thread->next_in_process->prev_in_process = thread->prev_in_process;
    }
    
    thread->process->thread_count--;
    
    // Free thread resources
    if (thread->stack) {
        memory_free(thread->stack);
    }
    
    if (thread->tls) {
        thread_free_tls(thread->tls);
    }
    
    memory_free(thread);
    
    return SUCCESS;
}

// Get thread by TID
thread_t* thread_get_by_tid(tid_t tid) {
    thread_t* thread = thread_list;
    while (thread) {
        if (thread->tid == tid) {
            return thread;
        }
        thread = thread->next;
    }
    return NULL;
}

// Get current thread
thread_t* thread_get_current(void) {
    process_t* current_process = process_get_current();
    if (!current_process) {
        return NULL;
    }
    
    // For now, return the main thread of the current process
    return current_process->threads;
}

// Set up thread context
error_t thread_setup_context(thread_t* thread) {
    if (!thread) {
        return E_INVAL;
    }
    
    // Initialize thread context structure
    memset(&thread->context, 0, sizeof(thread_context_t));
    
    // Set up stack pointer (grow downward)
    thread->context.rsp = (uintptr_t)thread->stack + thread->stack_size - 16;
    
    // Align stack to 16-byte boundary
    thread->context.rsp &= ~0xF;
    
    // Set up entry point
    thread->context.rip = thread->entry_point;
    
    // Set up argument in RDI (first argument)
    thread->context.rdi = (uintptr_t)thread->arg;
    
    // Set up flags
    thread->context.rflags = 0x202; // Interrupts enabled, reserved bit set
    
    // Set up segment registers
    thread->context.cs = 0x23; // User code segment
    thread->context.ds = 0x2B; // User data segment
    thread->context.es = 0x2B; // User data segment
    thread->context.fs = 0x2B; // User data segment
    thread->context.gs = 0x2B; // User data segment
    thread->context.ss = 0x2B; // User stack segment
    
    return SUCCESS;
}

// Switch to thread
void thread_switch(thread_t* from, thread_t* to) {
    if (!to) {
        return;
    }
    
    KDEBUG("Switching from thread TID=%u to TID=%u", 
           from ? from->tid : 0, to->tid);
    
    // Save current thread context
    if (from) {
        thread_save_context(from);
        from->state = THREAD_STATE_READY;
    }
    
    // Restore target thread context
    thread_restore_context(to);
    to->state = THREAD_STATE_RUNNING;
    
    // Update current thread
    if (to->process) {
        process_set_current(to->process);
    }
}

// Save thread context
void thread_save_context(thread_t* thread) {
    if (!thread) {
        return;
    }
    
    // Save general purpose registers
    // This would be implemented in assembly
    // For now, we'll use a simplified approach
    
    KDEBUG("Saving context for thread TID=%u", thread->tid);
}

// Restore thread context
void thread_restore_context(thread_t* thread) {
    if (!thread) {
        return;
    }
    
    // Restore general purpose registers
    // This would be implemented in assembly
    // For now, we'll use a simplified approach
    
    KDEBUG("Restoring context for thread TID=%u", thread->tid);
}

// Thread yield
void thread_yield(void) {
    thread_t* current = thread_get_current();
    if (!current) {
        return;
    }
    
    // Find next ready thread
    thread_t* next = thread_get_next_ready();
    if (next && next != current) {
        thread_switch(current, next);
    }
}

// Get next ready thread
thread_t* thread_get_next_ready(void) {
    thread_t* thread = thread_list;
    while (thread) {
        if (thread->state == THREAD_STATE_READY) {
            return thread;
        }
        thread = thread->next;
    }
    return NULL;
}

// Allocate thread-local storage
void* thread_alloc_tls(void) {
    if (!tls_pool || tls_pool_used + TLS_SIZE > tls_pool_size) {
        return NULL;
    }
    
    void* tls = (char*)tls_pool + tls_pool_used;
    tls_pool_used += TLS_SIZE;
    
    // Clear TLS
    memset(tls, 0, TLS_SIZE);
    
    return tls;
}

// Free thread-local storage
void thread_free_tls(void* tls) {
    // For simplicity, we don't actually free TLS
    // In a real implementation, you'd track allocations
    (void)tls;
}

// Mutex operations
mutex_id_t mutex_create(void) {
    mutex_t* mutex = (mutex_t*)memory_alloc(sizeof(mutex_t));
    if (!mutex) {
        return 0;
    }
    
    mutex->id = (mutex_id_t)(uintptr_t)mutex;
    mutex->locked = false;
    mutex->owner = 0;
    mutex->wait_queue = NULL;
    
    // Add to mutex list
    mutex->next = mutex_list;
    mutex_list = mutex;
    
    return mutex->id;
}

error_t mutex_destroy(mutex_id_t id) {
    mutex_t* mutex = mutex_get_by_id(id);
    if (!mutex) {
        return E_NOENT;
    }
    
    if (mutex->locked) {
        return E_BUSY;
    }
    
    // Remove from mutex list
    if (mutex == mutex_list) {
        mutex_list = mutex->next;
    } else {
        mutex_t* prev = mutex_list;
        while (prev && prev->next != mutex) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = mutex->next;
        }
    }
    
    memory_free(mutex);
    return SUCCESS;
}

error_t mutex_lock(mutex_id_t id) {
    mutex_t* mutex = mutex_get_by_id(id);
    if (!mutex) {
        return E_NOENT;
    }
    
    thread_t* current = thread_get_current();
    if (!current) {
        return E_INVAL;
    }
    
    while (mutex->locked) {
        // Add to wait queue
        mutex->wait_queue = current;
        current->state = THREAD_STATE_BLOCKED;
        
        // Yield to another thread
        thread_yield();
        
        // Check if we were woken up
        if (current->state == THREAD_STATE_READY) {
            break;
        }
    }
    
    mutex->locked = true;
    mutex->owner = current->tid;
    
    return SUCCESS;
}

error_t mutex_unlock(mutex_id_t id) {
    mutex_t* mutex = mutex_get_by_id(id);
    if (!mutex) {
        return E_NOENT;
    }
    
    thread_t* current = thread_get_current();
    if (!current || current->tid != mutex->owner) {
        return E_PERM;
    }
    
    mutex->locked = false;
    mutex->owner = 0;
    
    // Wake up waiting thread
    if (mutex->wait_queue) {
        thread_t* waiting = mutex->wait_queue;
        mutex->wait_queue = NULL;
        waiting->state = THREAD_STATE_READY;
    }
    
    return SUCCESS;
}

// Get mutex by ID
mutex_t* mutex_get_by_id(mutex_id_t id) {
    mutex_t* mutex = mutex_list;
    while (mutex) {
        if (mutex->id == id) {
            return mutex;
        }
        mutex = mutex->next;
    }
    return NULL;
}

// Semaphore operations
semaphore_id_t semaphore_create(int initial_value) {
    semaphore_t* sem = (semaphore_t*)memory_alloc(sizeof(semaphore_t));
    if (!sem) {
        return 0;
    }
    
    sem->id = (semaphore_id_t)(uintptr_t)sem;
    sem->value = initial_value;
    sem->wait_queue = NULL;
    
    // Add to semaphore list
    sem->next = semaphore_list;
    semaphore_list = sem;
    
    return sem->id;
}

error_t semaphore_destroy(semaphore_id_t id) {
    semaphore_t* sem = semaphore_get_by_id(id);
    if (!sem) {
        return E_NOENT;
    }
    
    if (sem->wait_queue) {
        return E_BUSY;
    }
    
    // Remove from semaphore list
    if (sem == semaphore_list) {
        semaphore_list = sem->next;
    } else {
        semaphore_t* prev = semaphore_list;
        while (prev && prev->next != sem) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = sem->next;
        }
    }
    
    memory_free(sem);
    return SUCCESS;
}

error_t semaphore_wait(semaphore_id_t id) {
    semaphore_t* sem = semaphore_get_by_id(id);
    if (!sem) {
        return E_NOENT;
    }
    
    thread_t* current = thread_get_current();
    if (!current) {
        return E_INVAL;
    }
    
    while (sem->value <= 0) {
        // Add to wait queue
        sem->wait_queue = current;
        current->state = THREAD_STATE_BLOCKED;
        
        // Yield to another thread
        thread_yield();
        
        // Check if we were woken up
        if (current->state == THREAD_STATE_READY) {
            break;
        }
    }
    
    sem->value--;
    return SUCCESS;
}

error_t semaphore_signal(semaphore_id_t id) {
    semaphore_t* sem = semaphore_get_by_id(id);
    if (!sem) {
        return E_NOENT;
    }
    
    sem->value++;
    
    // Wake up waiting thread
    if (sem->wait_queue) {
        thread_t* waiting = sem->wait_queue;
        sem->wait_queue = NULL;
        waiting->state = THREAD_STATE_READY;
    }
    
    return SUCCESS;
}

// Get semaphore by ID
semaphore_t* semaphore_get_by_id(semaphore_id_t id) {
    semaphore_t* sem = semaphore_list;
    while (sem) {
        if (sem->id == id) {
            return sem;
        }
        sem = sem->next;
    }
    return NULL;
}

// Condition variable operations
condition_id_t condition_create(void) {
    condition_t* cond = (condition_t*)memory_alloc(sizeof(condition_t));
    if (!cond) {
        return 0;
    }
    
    cond->id = (condition_id_t)(uintptr_t)cond;
    cond->wait_queue = NULL;
    
    // Add to condition list
    cond->next = condition_list;
    condition_list = cond;
    
    return cond->id;
}

error_t condition_destroy(condition_id_t id) {
    condition_t* cond = condition_get_by_id(id);
    if (!cond) {
        return E_NOENT;
    }
    
    if (cond->wait_queue) {
        return E_BUSY;
    }
    
    // Remove from condition list
    if (cond == condition_list) {
        condition_list = cond->next;
    } else {
        condition_t* prev = condition_list;
        while (prev && prev->next != cond) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = cond->next;
        }
    }
    
    memory_free(cond);
    return SUCCESS;
}

error_t condition_wait(condition_id_t id, mutex_id_t mutex_id) {
    condition_t* cond = condition_get_by_id(id);
    if (!cond) {
        return E_NOENT;
    }
    
    thread_t* current = thread_get_current();
    if (!current) {
        return E_INVAL;
    }
    
    // Release mutex
    mutex_unlock(mutex_id);
    
    // Add to wait queue
    cond->wait_queue = current;
    current->state = THREAD_STATE_BLOCKED;
    
    // Yield to another thread
    thread_yield();
    
    // Re-acquire mutex
    return mutex_lock(mutex_id);
}

error_t condition_signal(condition_id_t id) {
    condition_t* cond = condition_get_by_id(id);
    if (!cond) {
        return E_NOENT;
    }
    
    // Wake up waiting thread
    if (cond->wait_queue) {
        thread_t* waiting = cond->wait_queue;
        cond->wait_queue = NULL;
        waiting->state = THREAD_STATE_READY;
    }
    
    return SUCCESS;
}

error_t condition_broadcast(condition_id_t id) {
    condition_t* cond = condition_get_by_id(id);
    if (!cond) {
        return E_NOENT;
    }
    
    // Wake up all waiting threads
    while (cond->wait_queue) {
        thread_t* waiting = cond->wait_queue;
        cond->wait_queue = waiting->next;
        waiting->state = THREAD_STATE_READY;
    }
    
    return SUCCESS;
}

// Get condition by ID
condition_t* condition_get_by_id(condition_id_t id) {
    condition_t* cond = condition_list;
    while (cond) {
        if (cond->id == id) {
            return cond;
        }
        cond = cond->next;
    }
    return NULL;
}

// Get next available thread ID
tid_t get_next_thread_id(void) {
    return next_tid++;
}

// Check if threading is initialized
bool threading_is_initialized(void) {
    return threading_initialized;
} 