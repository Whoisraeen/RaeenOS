#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include "types.h"

// Interrupt frame structure (matches what's pushed by ISR stubs)
typedef struct {
    // Pushed by ISR stubs
    u64 interrupt_number;
    u64 error_code;
    
    // Pushed by CPU
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
    
    // Pushed by ISR common stub
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp;
    u64 r8, r9, r10, r11;
    u64 r12, r13, r14, r15;
} __attribute__((packed)) interrupt_frame_t;

// Exception handler function
void interrupt_handler(interrupt_frame_t* frame);

// Hardware interrupt handlers
void handle_timer_interrupt(void);
void handle_hardware_interrupt(u64 vector);

// Memory fault handling
bool memory_handle_page_fault(u64 fault_address, bool write, bool user);

// Process management
void process_terminate(void* process, int exit_code);

// Logging functions
void KLOG(const char* format, ...);
void KWARN(const char* format, ...);

#endif // EXCEPTION_HANDLER_H 