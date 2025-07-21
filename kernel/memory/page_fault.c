/*
 * RaeenOS Page Fault Handler
 * Production-grade page fault handling with advanced features
 */

#include "include/vmm.h"
#include "../core/include/kernel.h"
#include "../hal/include/hal.h"
#include "../arch/x86_64/include/interrupts.h"

// Page fault error codes (x86-64 specific)
#define PF_PRESENT      (1 << 0)  // Page was present
#define PF_WRITE        (1 << 1)  // Was a write access
#define PF_USER         (1 << 2)  // Was user mode access
#define PF_RESERVED     (1 << 3)  // Reserved bit violation
#define PF_INSTRUCTION  (1 << 4)  // Was instruction fetch

// External function from VMM
extern error_t vmm_handle_page_fault(page_fault_info_t* fault_info);

/**
 * Main page fault interrupt handler
 * Called from interrupt vector 14 (0x0E)
 */
void page_fault_handler(interrupt_frame_t* frame) {
    // Get the faulting address from CR2
    virt_addr_t fault_addr;
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
    
    // Parse error code
    uint64_t error_code = frame->error_code;
    
    // Create fault info structure
    page_fault_info_t fault_info = {
        .fault_addr = fault_addr,
        .error_code = error_code,
        .write_fault = (error_code & PF_WRITE) != 0,
        .user_fault = (error_code & PF_USER) != 0,
        .execute_fault = (error_code & PF_INSTRUCTION) != 0,
        .protection_fault = (error_code & PF_PRESENT) != 0,
        .present_fault = (error_code & PF_PRESENT) != 0,
        .process = get_current_process(),
        .vma = NULL,
        .timestamp = hal_get_timestamp(),
        .resolved = false,
        .resolution_time_us = 0,
    };
    
    // Handle the page fault
    error_t result = vmm_handle_page_fault(&fault_info);
    
    if (result != SUCCESS) {
        // Page fault could not be resolved
        KERROR("Page fault at 0x%llx could not be resolved (error: %d)", 
               fault_addr, result);
        
        if (fault_info.user_fault) {
            // Send SIGSEGV to user process
            // TODO: Implement signal delivery
            KERROR("Sending SIGSEGV to process");
        } else {
            // Kernel page fault - this is bad
            KERNEL_PANIC("Unresolvable kernel page fault at 0x%llx", fault_addr);
        }
    }
}

/**
 * Initialize page fault handler
 */
error_t page_fault_init(void) {
    KINFO("Initializing page fault handler");
    
    // Register page fault interrupt handler
    // This will be called by the interrupt system
    register_interrupt_handler(14, page_fault_handler);
    
    KINFO("Page fault handler initialized");
    return SUCCESS;
}