#include "include/process.h"
#include "../core/include/kernel.h"
#include "../core/include/interrupts.h"
#include "../hal/include/hal.h"
#include "../memory/include/memory.h"
#include <string.h>

// Scheduler state
static bool scheduler_initialized = false;
static bool scheduler_running = false;
static u64 scheduler_tick_count = 0;
static u64 last_schedule_time = 0;

// Ready queues (one per priority level)
static wait_queue_t* ready_queues[5] = {NULL}; // 5 priority levels: CRITICAL, HIGH, NORMAL, LOW, IDLE

// Current scheduling information
static thread_t* current_thread = NULL;
static u64 current_time_slice_start = 0;

// Round-robin scheduling state
static u32 current_priority_level = 0;

// Real-time scheduling state
static u64 rt_quantum = 1000; // 1ms for real-time tasks
static u64 normal_quantum = 10000; // 10ms for normal tasks
static u64 background_quantum = 50000; // 50ms for background tasks

// Load average calculation
static u64 load_average_1min = 0;
static u64 load_average_5min = 0;
static u64 load_average_15min = 0;

// Forward declarations
static void scheduler_add_to_ready_queue(thread_t* thread);
static thread_t* scheduler_remove_from_ready_queue(process_priority_t priority);
static void scheduler_handle_time_slice_expiry(void);
static void scheduler_update_load_average(void);
static void scheduler_aging(void);
static thread_t* scheduler_select_next_thread(void);
static void scheduler_preempt_if_needed(void);

error_t scheduler_init(void) {
    if (scheduler_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing advanced scheduler");
    
    // Create ready queues for each priority level
    for (int i = 0; i < 5; i++) {
        ready_queues[i] = wait_queue_create("ready_queue");
        if (!ready_queues[i]) {
            KERROR("Failed to create ready queue for priority %d", i);
            return E_NOMEM;
        }
    }
    
    scheduler_tick_count = 0;
    last_schedule_time = hal_get_timestamp();
    current_priority_level = 0;
    
    scheduler_initialized = true;
    
    KINFO("Advanced scheduler initialized with 5 priority levels");
    return SUCCESS;
}

void scheduler_start(void) {
    if (!scheduler_initialized) {
        KERROR("Scheduler not initialized");
        return;
    }
    
    KINFO("Starting advanced scheduler");
    
    scheduler_running = true;
    
    // Add idle thread to lowest priority queue
    if (idle_thread) {
        scheduler_add_thread(idle_thread);
    }
    
    // Start with idle thread if no other threads are ready
    current_thread = scheduler_next_thread();
    if (!current_thread) {
        current_thread = idle_thread;
    }
    
    current_time_slice_start = hal_get_timestamp();
    
    KINFO("Scheduler started with thread TID=%u", 
          current_thread ? current_thread->tid : 0);
}

void scheduler_tick(void) {
    if (!scheduler_running) {
        return;
    }
    
    scheduler_tick_count++;
    u64 current_time = hal_get_timestamp();
    
    // Update time statistics
    if (current_thread) {
        u64 time_used = current_time - current_time_slice_start;
        current_thread->time_used += time_used;
        current_thread->total_runtime += time_used;
        
        if (current_thread->parent_process) {
            current_thread->parent_process->total_cpu_time += time_used;
        }
    }
    
    // Check if current thread's time slice has expired
    if (current_thread && 
        (current_time - current_time_slice_start) >= current_thread->time_slice) {
        scheduler_handle_time_slice_expiry();
    }
    
    // Perform aging every 100 ticks (prevent starvation)
    if (scheduler_tick_count % 100 == 0) {
        scheduler_aging();
    }
    
    // Update load average every second (assuming 1000 Hz timer)
    if (scheduler_tick_count % 1000 == 0) {
        scheduler_update_load_average();
    }
    
    // Check for preemption
    scheduler_preempt_if_needed();
    
    last_schedule_time = current_time;
}

thread_t* scheduler_next_thread(void) {
    if (!scheduler_initialized) {
        return NULL;
    }
    
    return scheduler_select_next_thread();
}

static thread_t* scheduler_select_next_thread(void) {
    // Try each priority level in order (higher priority first)
    for (int priority = PRIORITY_CRITICAL; priority <= PRIORITY_IDLE; priority++) {
        thread_t* thread = scheduler_remove_from_ready_queue((process_priority_t)priority);
        if (thread) {
            thread->state = THREAD_STATE_RUNNING;
            thread->last_scheduled = hal_get_timestamp();
            
            // Set time slice based on priority
            switch (priority) {
                case PRIORITY_CRITICAL:
                    thread->time_slice = rt_quantum;
                    break;
                case PRIORITY_HIGH:
                    thread->time_slice = normal_quantum / 2;
                    break;
                case PRIORITY_NORMAL:
                    thread->time_slice = normal_quantum;
                    break;
                case PRIORITY_LOW:
                    thread->time_slice = normal_quantum * 2;
                    break;
                case PRIORITY_IDLE:
                    thread->time_slice = background_quantum;
                    break;
                default:
                    thread->time_slice = normal_quantum;
                    break;
            }
            
            return thread;
        }
    }
    
    // No threads available, return idle thread
    if (idle_thread && idle_thread->state == THREAD_STATE_READY) {
        idle_thread->state = THREAD_STATE_RUNNING;
        idle_thread->time_slice = background_quantum;
        return idle_thread;
    }
    
    return NULL;
}

void scheduler_yield(void) {
    if (!scheduler_running || !current_thread) {
        return;
    }
    
    KDEBUG("Thread TID=%u yielding", current_thread->tid);
    
    // Save current thread state
    if (current_thread->state == THREAD_STATE_RUNNING) {
        current_thread->state = THREAD_STATE_READY;
        scheduler_add_to_ready_queue(current_thread);
    }
    
    // Find next thread to run
    thread_t* next_thread = scheduler_next_thread();
    if (!next_thread) {
        next_thread = idle_thread;
    }
    
    // Context switch if we have a different thread
    if (next_thread && next_thread != current_thread) {
        context_switch(current_thread, next_thread);
    }
}

void scheduler_preempt(void) {
    if (!scheduler_running) {
        return;
    }
    
    KDEBUG("Preempting current thread");
    scheduler_yield();
}

static void scheduler_preempt_if_needed(void) {
    if (!current_thread) {
        return;
    }
    
    // Check if a higher priority thread has become ready
    for (int priority = PRIORITY_CRITICAL; priority < current_thread->priority; priority++) {
        if (ready_queues[priority] && ready_queues[priority]->head) {
            KDEBUG("Preempting thread TID=%u for higher priority thread", current_thread->tid);
            scheduler_preempt();
            return;
        }
    }
}

error_t scheduler_add_thread(thread_t* thread) {
    if (!thread) {
        return E_INVAL;
    }
    
    thread->state = THREAD_STATE_READY;
    scheduler_add_to_ready_queue(thread);
    
    KDEBUG("Added thread TID=%u to ready queue (priority=%d)", 
           thread->tid, thread->priority);
    
    return SUCCESS;
}

error_t scheduler_remove_thread(thread_t* thread) {
    if (!thread) {
        return E_INVAL;
    }
    
    // Remove from ready queue if present
    wait_queue_t* queue = ready_queues[thread->priority];
    if (queue) {
        // Linear search to remove thread
        thread_t* current = queue->head;
        thread_t* prev = NULL;
        
        while (current) {
            if (current == thread) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    queue->head = current->next;
                }
                
                if (current->next) {
                    current->next->prev = prev;
                }
                
                queue->count--;
                KDEBUG("Removed thread TID=%u from ready queue", thread->tid);
                return SUCCESS;
            }
            
            prev = current;
            current = current->next;
        }
    }
    
    return E_NOENT;
}

static void scheduler_handle_time_slice_expiry(void) {
    if (!current_thread) {
        return;
    }
    
    KDEBUG("Time slice expired for thread TID=%u", current_thread->tid);
    
    // Move thread to end of its priority queue
    if (current_thread->state == THREAD_STATE_RUNNING) {
        current_thread->state = THREAD_STATE_READY;
        scheduler_add_to_ready_queue(current_thread);
    }
    
    // Select next thread
    thread_t* next_thread = scheduler_next_thread();
    if (next_thread && next_thread != current_thread) {
        context_switch(current_thread, next_thread);
    }
}

static void scheduler_aging(void) {
    // Implement priority aging to prevent starvation
    for (int priority = PRIORITY_IDLE; priority > PRIORITY_CRITICAL; priority--) {
        wait_queue_t* queue = ready_queues[priority];
        if (!queue || !queue->head) {
            continue;
        }
        
        thread_t* thread = queue->head;
        while (thread) {
            // Increase priority of threads that have been waiting too long
            u64 current_time = hal_get_timestamp();
            u64 wait_time = current_time - thread->last_scheduled;
            if (wait_time > 1000000) { // 1 second in microseconds
                thread->priority = (thread->priority > 0) ? thread->priority - 1 : 0;
                thread->last_scheduled = current_time;
                KDEBUG("Aged thread TID=%u to priority %d", thread->tid, thread->priority);
            }
            thread = thread->next;
        }
    }
}

static void scheduler_update_load_average(void) {
    // Calculate load average based on number of runnable threads
    u32 runnable_threads = 0;
    for (int i = 0; i < 5; i++) {
        if (ready_queues[i]) {
            runnable_threads += ready_queues[i]->count;
        }
    }
    
    // Simple exponential moving average
    load_average_1min = (load_average_1min * 59 + runnable_threads * 100) / 100;
    load_average_5min = (load_average_5min * 299 + runnable_threads * 100) / 100;
    load_average_15min = (load_average_15min * 899 + runnable_threads * 100) / 100;
    
    KDEBUG("Load average: 1min=%llu.%02llu, 5min=%llu.%02llu, 15min=%llu.%02llu",
           load_average_1min / 100, load_average_1min % 100,
           load_average_5min / 100, load_average_5min % 100,
           load_average_15min / 100, load_average_15min % 100);
}

static void scheduler_add_to_ready_queue(thread_t* thread) {
    if (!thread || thread->priority > PRIORITY_IDLE) {
        return;
    }
    
    wait_queue_t* queue = ready_queues[thread->priority];
    if (!queue) {
        return;
    }
    
    // Add to end of queue (FIFO within same priority)
    thread->next = NULL;
    thread->prev = queue->tail;
    
    if (queue->tail) {
        queue->tail->next = thread;
    } else {
        queue->head = thread;
    }
    
    queue->tail = thread;
    queue->count++;
}

static thread_t* scheduler_remove_from_ready_queue(process_priority_t priority) {
    if (priority > PRIORITY_IDLE) {
        return NULL;
    }
    
    wait_queue_t* queue = ready_queues[priority];
    if (!queue || !queue->head) {
        return NULL;
    }
    
    // Remove from head of queue (FIFO)
    thread_t* thread = queue->head;
    queue->head = thread->next;
    
    if (queue->head) {
        queue->head->prev = NULL;
    } else {
        queue->tail = NULL;
    }
    
    queue->count--;
    thread->next = NULL;
    thread->prev = NULL;
    
    return thread;
}

void context_switch(thread_t* old_thread, thread_t* new_thread) {
    if (!old_thread || !new_thread) {
        return;
    }
    
    KDEBUG("Context switch: TID=%u -> TID=%u", old_thread->tid, new_thread->tid);
    
    // Save old thread context
    if (old_thread) {
        context_save(&old_thread->context);
    }
    
    // Restore new thread context
    if (new_thread) {
        context_restore(&new_thread->context);
    }
    
    // Update current thread pointer
    current_thread = new_thread;
    current_time_slice_start = hal_get_timestamp();
    
    // Switch address space if needed
    if (old_thread->parent_process != new_thread->parent_process) {
        // This would switch page tables in a real implementation
        KDEBUG("Switching address space: PID=%u -> PID=%u",
               old_thread->parent_process ? old_thread->parent_process->pid : 0,
               new_thread->parent_process ? new_thread->parent_process->pid : 0);
    }
}

void scheduler_dump_stats(void) {
    KINFO("=== Scheduler Statistics ===");
    KINFO("Initialized: %s", scheduler_initialized ? "Yes" : "No");
    KINFO("Running: %s", scheduler_running ? "Yes" : "No");
    KINFO("Tick count: %llu", scheduler_tick_count);
    KINFO("Current thread: TID=%u", current_thread ? current_thread->tid : 0);
    KINFO("Load average: 1min=%llu.%02llu, 5min=%llu.%02llu, 15min=%llu.%02llu",
           load_average_1min / 100, load_average_1min % 100,
           load_average_5min / 100, load_average_5min % 100,
           load_average_15min / 100, load_average_15min % 100);
}

void scheduler_dump_queues(void) {
    KINFO("=== Ready Queues ===");
    for (int i = 0; i < 5; i++) {
        wait_queue_t* queue = ready_queues[i];
        if (queue && queue->count > 0) {
            KINFO("Priority %d: %u threads", i, queue->count);
            thread_t* thread = queue->head;
            while (thread) {
                KINFO("  TID=%u, PID=%u", 
                      thread->tid, 
                      thread->parent_process ? thread->parent_process->pid : 0);
                thread = thread->next;
            }
        }
    }
}

// Wait queue management
wait_queue_t* wait_queue_create(const char* name) {
    wait_queue_t* queue = (wait_queue_t*)memory_alloc(sizeof(wait_queue_t));
    if (!queue) {
        return NULL;
    }
    
    memset(queue, 0, sizeof(wait_queue_t));
    if (name) {
        strncpy(queue->name, name, sizeof(queue->name) - 1);
    }
    
    return queue;
}

void wait_queue_destroy(wait_queue_t* queue) {
    if (!queue) {
        return;
    }
    
    // Wake up all waiting threads
    thread_t* thread = queue->head;
    while (thread) {
        thread_t* next = thread->next;
        thread->state = THREAD_STATE_READY;
        scheduler_add_thread(thread);
        thread = next;
    }
    
    memory_free(queue);
}

error_t wait_queue_add(wait_queue_t* queue, thread_t* thread) {
    if (!queue || !thread) {
        return E_INVAL;
    }
    
    thread->state = THREAD_STATE_WAITING;
    thread->wait_queue = queue;
    thread->next = NULL;
    thread->prev = queue->tail;
    
    if (queue->tail) {
        queue->tail->next = thread;
    } else {
        queue->head = thread;
    }
    
    queue->tail = thread;
    queue->count++;
    
    return SUCCESS;
}

error_t thread_sleep(u64 milliseconds) {
    if (!current_thread) {
        return E_EPERM;
    }
    
    current_thread->sleep_until = hal_get_timestamp() + milliseconds * 1000;
    current_thread->state = THREAD_STATE_SLEEPING;
    
    scheduler_yield();
    
    return SUCCESS;
}

error_t thread_block(wait_queue_t* wait_queue) {
    if (!current_thread || !wait_queue) {
        return E_INVAL;
    }
    
    return wait_queue_add(wait_queue, current_thread);
}

error_t thread_wake(thread_t* thread) {
    if (!thread) {
        return E_INVAL;
    }
    
    if (thread->state == THREAD_STATE_WAITING && thread->wait_queue) {
        // Remove from wait queue
        wait_queue_t* queue = thread->wait_queue;
        if (thread->prev) {
            thread->prev->next = thread->next;
        } else {
            queue->head = thread->next;
        }
        
        if (thread->next) {
            thread->next->prev = thread->prev;
        } else {
            queue->tail = thread->prev;
        }
        
        queue->count--;
        thread->wait_queue = NULL;
        thread->next = NULL;
        thread->prev = NULL;
        
        // Add to ready queue
        thread->state = THREAD_STATE_READY;
        scheduler_add_thread(thread);
        
        return SUCCESS;
    }
    
    return E_INVAL;
}

error_t thread_wake_all(wait_queue_t* wait_queue) {
    if (!wait_queue) {
        return E_INVAL;
    }
    
    thread_t* thread = wait_queue->head;
    while (thread) {
        thread_t* next = thread->next;
        thread_wake(thread);
        thread = next;
    }
    
    return SUCCESS;
}