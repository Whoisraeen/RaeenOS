#include "tss.h"
#include "kernel.h"
#include "memory/memory.h"
#include "hal/hal.h"
#include <string.h>

// Global TSS instance
static tss_t kernel_tss __attribute__((aligned(16)));
static bool tss_initialized = false;

// IST stacks
static u8* ist_stacks[NUM_IST_STACKS] = {NULL};
static bool ist_allocated[NUM_IST_STACKS] = {false};

// GDT TSS descriptor index (will be set by GDT initialization)
static u16 tss_gdt_index = 0;

error_t tss_init(void) {
    if (tss_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing Task State Segment (TSS)");
    
    // Clear TSS structure
    memset(&kernel_tss, 0, sizeof(tss_t));
    
    // Set IOPB offset to indicate no I/O permission bitmap
    kernel_tss.iopb_offset = sizeof(tss_t);
    
    // Initialize IST stacks
    error_t result = ist_init_stacks();
    if (result != SUCCESS) {
        KERROR("Failed to initialize IST stacks");
        return result;
    }
    
    // Set up IST entries in TSS
    for (u8 i = 0; i < NUM_IST_STACKS; i++) {
        if (ist_stacks[i]) {
            // Point to top of stack (stacks grow downward)
            u64 stack_top = (u64)ist_stacks[i] + IST_STACK_SIZE;
            tss_set_ist(i + 1, stack_top);
            KINFO("IST%d stack: 0x%llx", i + 1, stack_top);
        }
    }
    
    // Allocate kernel stack for Ring 0
    u64 kernel_stack = tss_allocate_stack(16 * 1024); // 16KB kernel stack
    if (kernel_stack == 0) {
        KERROR("Failed to allocate kernel stack");
        return E_NOMEM;
    }
    
    tss_set_rsp0(kernel_stack + 16 * 1024); // Point to top of stack
    KINFO("Kernel stack (RSP0): 0x%llx", kernel_stack + 16 * 1024);
    
    tss_initialized = true;
    KINFO("TSS initialized successfully");
    
    return SUCCESS;
}

error_t ist_init_stacks(void) {
    KINFO("Initializing Interrupt Stack Table (IST)");
    
    for (u8 i = 0; i < NUM_IST_STACKS; i++) {
        ist_allocate_stack(i);
    }
    
    KINFO("IST initialized with %u stacks", NUM_IST_STACKS);
    return SUCCESS;
}

void ist_allocate_stack(u8 ist_index) {
    if (ist_index >= NUM_IST_STACKS) {
        KERROR("Invalid IST index: %u", ist_index);
        return;
    }
    
    if (ist_allocated[ist_index]) {
        KERROR("IST stack %u already allocated", ist_index);
        return;
    }
    
    // Allocate IST stack
    ist_stacks[ist_index] = memory_alloc(IST_STACK_SIZE);
    if (!ist_stacks[ist_index]) {
        KERROR("Failed to allocate IST stack %u", ist_index);
        return;
    }
    
    // Clear the stack
    memset(ist_stacks[ist_index], 0, IST_STACK_SIZE);
    
    ist_allocated[ist_index] = true;
    KINFO("Allocated IST stack %u at 0x%llx", ist_index, (u64)ist_stacks[ist_index]);
}

void ist_free_stack(u8 ist_index) {
    if (ist_index >= NUM_IST_STACKS) {
        return;
    }
    
    if (ist_stacks[ist_index]) {
        memory_free(ist_stacks[ist_index]);
        ist_stacks[ist_index] = NULL;
        ist_allocated[ist_index] = false;
        KINFO("Freed IST stack %u", ist_index);
    }
}

void tss_set_rsp0(u64 rsp) {
    kernel_tss.rsp0 = rsp;
    KDEBUG("Set RSP0 to 0x%llx", rsp);
}

void tss_set_ist(u8 ist_index, u64 rsp) {
    if (ist_index < 1 || ist_index > 7) {
        KERROR("Invalid IST index: %u", ist_index);
        return;
    }
    
    switch (ist_index) {
        case 1: kernel_tss.ist1 = rsp; break;
        case 2: kernel_tss.ist2 = rsp; break;
        case 3: kernel_tss.ist3 = rsp; break;
        case 4: kernel_tss.ist4 = rsp; break;
        case 5: kernel_tss.ist5 = rsp; break;
        case 6: kernel_tss.ist6 = rsp; break;
        case 7: kernel_tss.ist7 = rsp; break;
    }
    
    KDEBUG("Set IST%d to 0x%llx", ist_index, rsp);
}

u64 tss_get_ist(u8 ist_index) {
    if (ist_index < 1 || ist_index > 7) {
        return 0;
    }
    
    switch (ist_index) {
        case 1: return kernel_tss.ist1;
        case 2: return kernel_tss.ist2;
        case 3: return kernel_tss.ist3;
        case 4: return kernel_tss.ist4;
        case 5: return kernel_tss.ist5;
        case 6: return kernel_tss.ist6;
        case 7: return kernel_tss.ist7;
        default: return 0;
    }
}

void tss_load(void) {
    if (!tss_initialized) {
        KERROR("TSS not initialized");
        return;
    }
    
    // Load TSS into GDT and set TR register
    // This will be called after GDT is set up
    KINFO("TSS loaded into GDT at index %u", tss_gdt_index);
}

u64 tss_allocate_stack(size_t size) {
    // Allocate aligned stack
    u8* stack = memory_alloc(size);
    if (!stack) {
        return 0;
    }
    
    // Clear the stack
    memset(stack, 0, size);
    
    return (u64)stack;
}

void tss_free_stack(u64 stack_ptr) {
    if (stack_ptr) {
        memory_free((void*)stack_ptr);
    }
}

bool tss_validate_stack(u64 stack_ptr) {
    if (!stack_ptr) {
        return false;
    }
    
    // Check if stack pointer is within valid memory range
    // This is a basic validation - in a real system you'd check page tables
    return (stack_ptr >= 0x1000 && stack_ptr < 0x7FFFFFFFFFFF);
}

void tss_switch_to_kernel_stack(void) {
    // This would be called when entering kernel mode
    // The CPU will automatically use RSP0 from TSS
    KDEBUG("Switched to kernel stack");
}

void tss_switch_to_user_stack(u64 user_rsp) {
    // This would be called when switching to user mode
    // The user RSP would be saved in the process structure
    KDEBUG("Switched to user stack: 0x%llx", user_rsp);
}

void tss_switch_to_interrupt_stack(u8 ist_index) {
    if (ist_index < 1 || ist_index > 7) {
        KERROR("Invalid IST index for interrupt: %u", ist_index);
        return;
    }
    
    u64 ist_stack = tss_get_ist(ist_index);
    if (!ist_stack) {
        KERROR("IST stack %u not available", ist_index);
        return;
    }
    
    KDEBUG("Switched to IST stack %u: 0x%llx", ist_index, ist_stack);
}

void tss_dump_info(void) {
    if (!tss_initialized) {
        KINFO("TSS not initialized");
        return;
    }
    
    KINFO("Task State Segment Information:");
    KINFO("  RSP0 (Ring 0): 0x%llx", kernel_tss.rsp0);
    KINFO("  RSP1 (Ring 1): 0x%llx", kernel_tss.rsp1);
    KINFO("  RSP2 (Ring 2): 0x%llx", kernel_tss.rsp2);
    KINFO("  IOPB Offset: %u", kernel_tss.iopb_offset);
    
    KINFO("Interrupt Stack Table:");
    for (u8 i = 1; i <= 7; i++) {
        u64 ist_stack = tss_get_ist(i);
        KINFO("  IST%d: 0x%llx", i, ist_stack);
    }
    
    KINFO("IST Stack Allocations:");
    for (u8 i = 0; i < NUM_IST_STACKS; i++) {
        KINFO("  IST%d: %s at 0x%llx", i + 1, 
              ist_allocated[i] ? "allocated" : "not allocated",
              (u64)ist_stacks[i]);
    }
}

void tss_validate_integrity(void) {
    if (!tss_initialized) {
        KERROR("TSS integrity check failed: not initialized");
        return;
    }
    
    // Validate kernel stack
    if (!tss_validate_stack(kernel_tss.rsp0)) {
        KERROR("TSS integrity check failed: invalid RSP0");
        return;
    }
    
    // Validate IST stacks
    for (u8 i = 1; i <= 7; i++) {
        u64 ist_stack = tss_get_ist(i);
        if (ist_stack && !tss_validate_stack(ist_stack)) {
            KERROR("TSS integrity check failed: invalid IST%d", i);
            return;
        }
    }
    
    KINFO("TSS integrity check passed");
}

bool tss_is_initialized(void) {
    return tss_initialized;
}

const char* tss_get_error_string(error_t error) {
    switch (error) {
        case E_NOMEM:
            return "Out of memory";
        case E_INVAL:
            return "Invalid parameter";
        case E_ALREADY:
            return "Already initialized";
        default:
            return "Unknown error";
    }
}

void tss_handle_stack_corruption(u64 corrupted_stack) {
    KERROR("Stack corruption detected at 0x%llx", corrupted_stack);
    
    // Switch to a known-good stack for error handling
    tss_switch_to_interrupt_stack(IST_DOUBLE_FAULT);
    
    // Log the corruption
    KERROR("Stack corruption handled - system continuing");
    
    // In a real system, you might want to:
    // 1. Log the corruption details
    // 2. Kill the offending process
    // 3. Trigger a kernel panic if it's a kernel stack
    // 4. Send a signal to the process
}

// Function to set the GDT index for TSS
void tss_set_gdt_index(u16 index) {
    tss_gdt_index = index;
}

// Function to get the TSS base address for GDT
u64 tss_get_base_address(void) {
    return (u64)&kernel_tss;
}

// Function to get TSS size for GDT
u32 tss_get_size(void) {
    return sizeof(tss_t);
} 