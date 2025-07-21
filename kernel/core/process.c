#include "process.h"
#include "kernel.h"
#include "memory/memory.h"
#include "memory/pmm.h"
#include "hal/hal.h"
#include <string.h>

#define KERNEL_STACK_SIZE (PAGE_SIZE * 4) // 16KB kernel stack per process

static pcb_t* ready_queue_head = NULL;
static pcb_t* ready_queue_tail = NULL;
static pcb_t* current_process = NULL;
static pid_t next_pid = 1; // Start with PID 1
static bool scheduling_started = false;

extern void context_switch(process_context_t* old, process_context_t* new);

error_t process_init(void) {
    KINFO("Initializing Process Management...");
    // Create an idle process (PID 0)
    pcb_t* idle_process = memory_alloc(sizeof(pcb_t));
    memset(idle_process, 0, sizeof(pcb_t));
    idle_process->pid = 0;
    idle_process->state = PROCESS_STATE_RUNNING;
    idle_process->pml4 = hal_get_page_directory();
    current_process = idle_process;

    // Timer interrupt is now handled in the HAL

    return SUCCESS;
}

pcb_t* process_create_kthread(void (*entry)(void)) {
    hal_disable_interrupts();

    pcb_t* pcb = memory_alloc(sizeof(pcb_t));
    if (!pcb) {
        KERROR("Failed to allocate PCB");
        hal_enable_interrupts();
        return NULL;
    }
    memset(pcb, 0, sizeof(pcb_t));

    pcb->kernel_stack = (virt_addr_t)memory_alloc(KERNEL_STACK_SIZE);
    if (!pcb->kernel_stack) {
        KERROR("Failed to allocate kernel stack");
        memory_free(pcb);
        hal_enable_interrupts();
        return NULL;
    }

    pcb->pid = next_pid++;
    pcb->state = PROCESS_STATE_CREATED;
    pcb->pml4 = hal_get_page_directory(); // Kernel threads share the kernel address space

    // Set up the initial context
    pcb->context.rsp = pcb->kernel_stack + KERNEL_STACK_SIZE; // Stack grows down
    pcb->context.rip = (u64)entry;
    pcb->context.rflags = 0x202; // Interrupts enabled
    pcb->context.cs = 0x08; // Kernel code segment
    pcb->context.ss = 0x10; // Kernel data segment

    // Add to ready queue
    pcb->state = PROCESS_STATE_READY;
    if (!ready_queue_head) {
        ready_queue_head = pcb;
        ready_queue_tail = pcb;
    } else {
        ready_queue_tail->next = pcb;
        ready_queue_tail = pcb;
    }

    KDEBUG("Created process with PID: %u", pcb->pid);
    hal_enable_interrupts();
    return pcb;
}

// AI-Optimized Hybrid Scheduler (Combines macOS' QoS with Windows' Fair Share)
void schedule() {
    process_t *best = NULL;
    float max_score = -1;
    
    // Calculate AI-predicted priority (40% historical usage, 30% process type, 30% user focus)
    for_each_process(p) {
        float ai_score = p->ai_ctx.usage_history * 0.4f
                       + p->type_priority * 0.3f
                       + p->window_focus * 0.3f;
        
        // Real-time adjustment using neural network (3-layer MLP)
        if(ai_nn_enabled) {
            float nn_input[3] = {p->cpu_usage, p->mem_usage, p->io_activity};
            ai_score += run_neural_net(p->ai_ctx.nn_model, nn_input);
        }
        
        if(ai_score > max_score) {
            max_score = ai_score;
            best = p;
        }
    }
    switch_to_process(best);

    pcb_t* prev_process = current_process;
    current_process = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (!ready_queue_head) {
        ready_queue_tail = NULL;
    }
    current_process->next = NULL;

    if (prev_process->state == PROCESS_STATE_RUNNING) {
        prev_process->state = PROCESS_STATE_READY;
        if (!ready_queue_head) {
            ready_queue_head = prev_process;
            ready_queue_tail = prev_process;
        } else {
            ready_queue_tail->next = prev_process;
            ready_queue_tail = prev_process;
        }
    }

    current_process->state = PROCESS_STATE_RUNNING;
    context_switch(&prev_process->context, &current_process->context);
}

void process_start_scheduling() {
    scheduling_started = true;
    KINFO("Starting scheduler. Timer interrupts will now drive scheduling.");

    // We no longer call schedule() here. It will be called by the timer interrupt.
    // In a more complete system, we'd set up the first user process and then
    // transition to it, which would then enable interrupts.
}

pcb_t* get_current_process(void) {
    return current_process;
}