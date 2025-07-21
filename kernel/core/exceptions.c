#include "interrupts.h"
#include "kernel.h"
#include "memory/include/memory.h"

// Exception handler implementations
void exception_divide_by_zero(interrupt_context_t* context) {
    KERROR("EXCEPTION: Divide by zero at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    KERNEL_PANIC("Divide by zero exception");
}

void exception_debug(interrupt_context_t* context) {
    KDEBUG("DEBUG EXCEPTION: at RIP=0x%llx", context->rip);
    // Debug exceptions can be handled more gracefully
}

void exception_nmi(interrupt_context_t* context) {
    KERROR("NMI (Non-Maskable Interrupt) at RIP=0x%llx", context->rip);
    // NMI usually indicates serious hardware problems
    KERNEL_PANIC("Non-maskable interrupt");
}

void exception_breakpoint(interrupt_context_t* context) {
    KDEBUG("BREAKPOINT: at RIP=0x%llx", context->rip);
    // Breakpoints are used by debuggers
    dump_interrupt_context(context);
}

void exception_overflow(interrupt_context_t* context) {
    KERROR("EXCEPTION: Overflow at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    KERNEL_PANIC("Overflow exception");
}

void exception_bound_range_exceeded(interrupt_context_t* context) {
    KERROR("EXCEPTION: Bound range exceeded at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    KERNEL_PANIC("Bound range exceeded exception");
}

void exception_invalid_opcode(interrupt_context_t* context) {
    KERROR("EXCEPTION: Invalid opcode at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    KERNEL_PANIC("Invalid opcode exception");
}

void exception_device_not_available(interrupt_context_t* context) {
    KERROR("EXCEPTION: Device not available at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    
    // This could be FPU/SSE not available - try to handle gracefully
    // For now, just panic
    KERNEL_PANIC("Device not available exception");
}

void exception_double_fault(interrupt_context_t* context) {
    KERROR("FATAL: Double fault at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    
    // Double fault is unrecoverable
    KERNEL_PANIC("Double fault - system halted");
}

void exception_invalid_tss(interrupt_context_t* context) {
    KERROR("EXCEPTION: Invalid TSS at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    KERNEL_PANIC("Invalid TSS exception");
}

void exception_segment_not_present(interrupt_context_t* context) {
    KERROR("EXCEPTION: Segment not present at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    KERNEL_PANIC("Segment not present exception");
}

void exception_stack_segment_fault(interrupt_context_t* context) {
    KERROR("EXCEPTION: Stack segment fault at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    KERNEL_PANIC("Stack segment fault exception");
}

void exception_general_protection(interrupt_context_t* context) {
    KERROR("EXCEPTION: General protection fault at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    
    // Analyze error code
    if (context->error_code != 0) {
        u16 selector = context->error_code & 0xFFFF;
        bool external = (context->error_code & 0x01) != 0;
        u8 table = (context->error_code >> 1) & 0x03;
        
        KERROR("  Selector: 0x%x, External: %s, Table: %s", 
               selector, external ? "Yes" : "No",
               table == 0 ? "GDT" : (table == 1 ? "IDT" : (table == 2 ? "LDT" : "IDT")));
    }
    
    KERNEL_PANIC("General protection fault");
}

void exception_page_fault(interrupt_context_t* context) {
    // Get the faulting address from CR2
    u64 fault_addr = 0; // This should read CR2 register
    __asm__ volatile ("mov %%cr2, %0" : "=r" (fault_addr));
    
    // Decode error code
    bool present = (context->error_code & 0x01) != 0;
    bool write = (context->error_code & 0x02) != 0;
    bool user = (context->error_code & 0x04) != 0;
    bool reserved = (context->error_code & 0x08) != 0;
    bool instruction = (context->error_code & 0x10) != 0;
    
    KDEBUG("PAGE FAULT: addr=0x%llx, RIP=0x%llx, error=0x%llx", 
           fault_addr, context->rip, context->error_code);
    KDEBUG("  Present: %s, Write: %s, User: %s, Reserved: %s, Instruction: %s",
           present ? "Yes" : "No", write ? "Yes" : "No", user ? "Yes" : "No",
           reserved ? "Yes" : "No", instruction ? "Yes" : "No");
    
    // Check if this is a kernel page fault
    if (!user && fault_addr >= KERNEL_SPACE_START) {
        KERROR("Kernel page fault at 0x%llx from RIP=0x%llx", fault_addr, context->rip);
        dump_interrupt_context(context);
        KERNEL_PANIC("Kernel page fault");
    }
    
    // For user page faults, try to handle through memory management
    if (user) {
        // This would normally send a signal to the process
        // For now, just call the page fault handler
        page_fault_handler(fault_addr, (u32)context->error_code);
        return;
    }
    
    // Unhandled page fault
    KERROR("Unhandled page fault at 0x%llx from RIP=0x%llx", fault_addr, context->rip);
    dump_interrupt_context(context);
    KERNEL_PANIC("Unhandled page fault");
}

void exception_x87_fpu_error(interrupt_context_t* context) {
    KERROR("EXCEPTION: x87 FPU error at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    
    // Could try to handle FPU errors more gracefully
    KERNEL_PANIC("x87 FPU error exception");
}

void exception_alignment_check(interrupt_context_t* context) {
    KERROR("EXCEPTION: Alignment check at RIP=0x%llx, error_code=0x%llx", 
           context->rip, context->error_code);
    dump_interrupt_context(context);
    KERNEL_PANIC("Alignment check exception");
}

void exception_machine_check(interrupt_context_t* context) {
    KERROR("FATAL: Machine check exception at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    
    // Machine check exceptions indicate serious hardware problems
    KERNEL_PANIC("Machine check exception - hardware failure detected");
}

void exception_simd_fpu_error(interrupt_context_t* context) {
    KERROR("EXCEPTION: SIMD FPU error at RIP=0x%llx", context->rip);
    dump_interrupt_context(context);
    
    // Could try to handle SIMD errors more gracefully
    KERNEL_PANIC("SIMD FPU error exception");
}

// Default hardware interrupt handlers
void irq_timer(interrupt_context_t* context) {
    // Timer interrupt - this will be used for scheduling
    static u64 timer_ticks = 0;
    timer_ticks++;
    
    // Every 1000 ticks (roughly 1 second at 1000 Hz), print a message
    if (timer_ticks % 1000 == 0) {
        KDEBUG("Timer tick: %llu seconds", timer_ticks / 1000);
    }
    
    // This is where we would call the scheduler
    // scheduler_tick();
}

void irq_keyboard(interrupt_context_t* context) {
    // Read the scancode from the keyboard controller
    u8 scancode = hal_inb(0x60);
    
    KDEBUG("Keyboard interrupt: scancode=0x%02x", scancode);
    
    // This is where we would process the keyboard input
    // keyboard_handle_scancode(scancode);
}

void irq_serial_port(interrupt_context_t* context) {
    KDEBUG("Serial port interrupt");
    // Handle serial port data
}

void irq_rtc(interrupt_context_t* context) {
    KDEBUG("RTC interrupt");
    // Handle real-time clock interrupt
}

void irq_mouse(interrupt_context_t* context) {
    KDEBUG("Mouse interrupt");
    // Handle mouse data
}

void irq_spurious(interrupt_context_t* context) {
    KWARN("Spurious interrupt received");
    // Don't send EOI for spurious interrupts
}

// System call handler
void syscall_handler(interrupt_context_t* context) {
    // System call number is in RAX
    u64 syscall_num = context->rax;
    
    KDEBUG("System call: %llu", syscall_num);
    
    // This is where we would dispatch to the actual system call handlers
    // For now, just return an error
    context->rax = (u64)-1; // Return error
}