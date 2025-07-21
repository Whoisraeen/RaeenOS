#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

// Process states
typedef enum {
    PROCESS_STATE_CREATED,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_READY,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

// Stores the register state for context switching
typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
    u64 rip, cs, rflags, rsp, ss;
} __attribute__((packed)) process_context_t;

// Process control block (PCB) structure
struct pcb {
    pid_t pid;  // Process ID
    process_state_t state;

    process_context_t context; // Saved register state

    phys_addr_t pml4;       // Physical address of the process's page map
    virt_addr_t kernel_stack; // Base of the kernel stack for this process

    struct pcb* next; // For linked lists (e.g., ready queue)
};
typedef struct pcb pcb_t;

error_t process_init(void);

// Creates a new kernel-mode process
pcb_t* process_create_kthread(void (*entry)(void));

// Starts the scheduler
void process_start_scheduling(void);

// Returns the currently running process
pcb_t* get_current_process(void);

#endif // PROCESS_H