#include "arch/x86_64/exception_handler.h"
#include "arch/x86_64/tss.h"
#include "kernel.h"
#include "process/process.h"
#include "memory/memory.h"
#include <string.h>

// Exception names for better error reporting
static const char* exception_names[] = {
    "Divide Error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

// Exception severity levels
typedef enum {
    EXCEPTION_SEVERITY_INFO,
    EXCEPTION_SEVERITY_WARNING,
    EXCEPTION_SEVERITY_ERROR,
    EXCEPTION_SEVERITY_FATAL
} exception_severity_t;

// Exception information structure
typedef struct {
    u64 vector;
    u64 error_code;
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
    exception_severity_t severity;
    bool handled;
    bool fatal;
} exception_info_t;

// Forward declarations
static void handle_critical_exception(exception_info_t* info);
static void handle_page_fault(exception_info_t* info);
static void handle_double_fault(exception_info_t* info);
static void handle_general_protection_fault(exception_info_t* info);
static void handle_invalid_opcode(exception_info_t* info);
static void handle_divide_error(exception_info_t* info);
static void handle_debug_exception(exception_info_t* info);
static void handle_overflow(exception_info_t* info);
static void handle_bound_range(exception_info_t* info);
static void handle_nmi(exception_info_t* info);
static void log_exception(exception_info_t* info);
static void dump_registers(interrupt_frame_t* frame);
static void dump_stack_trace(u64 rbp);
static bool is_kernel_mode(u64 cs);
static process_t* get_current_process(void);

void interrupt_handler(interrupt_frame_t* frame) {
    if (!frame) {
        KERROR("Interrupt handler called with NULL frame");
        hal_halt();
    }
    
    u64 vector = frame->interrupt_number;
    u64 error_code = frame->error_code;
    
    // Create exception info structure
    exception_info_t info = {
        .vector = vector,
        .error_code = error_code,
        .rip = frame->rip,
        .cs = frame->cs,
        .rflags = frame->rflags,
        .rsp = frame->rsp,
        .ss = frame->ss,
        .severity = EXCEPTION_SEVERITY_INFO,
        .handled = false,
        .fatal = false
    };
    
    // Determine exception severity and handling
    if (vector < 32) {
        // System exception
        switch (vector) {
            case 0:  // Divide Error
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_divide_error(&info);
                break;
                
            case 1:  // Debug
                info.severity = EXCEPTION_SEVERITY_INFO;
                handle_debug_exception(&info);
                break;
                
            case 2:  // NMI
                info.severity = EXCEPTION_SEVERITY_WARNING;
                handle_nmi(&info);
                break;
                
            case 3:  // Breakpoint
                info.severity = EXCEPTION_SEVERITY_INFO;
                info.handled = true;
                break;
                
            case 4:  // Overflow
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_overflow(&info);
                break;
                
            case 5:  // Bound Range Exceeded
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_bound_range(&info);
                break;
                
            case 6:  // Invalid Opcode
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_invalid_opcode(&info);
                break;
                
            case 7:  // Device Not Available
                info.severity = EXCEPTION_SEVERITY_WARNING;
                info.handled = true;
                break;
                
            case 8:  // Double Fault
                info.severity = EXCEPTION_SEVERITY_FATAL;
                info.fatal = true;
                handle_double_fault(&info);
                break;
                
            case 9:  // Coprocessor Segment Overrun
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 10: // Invalid TSS
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 11: // Segment Not Present
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 12: // Stack-Segment Fault
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 13: // General Protection Fault
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_general_protection_fault(&info);
                break;
                
            case 14: // Page Fault
                info.severity = EXCEPTION_SEVERITY_ERROR;
                handle_page_fault(&info);
                break;
                
            case 16: // x87 FPU Error
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 17: // Alignment Check
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            case 18: // Machine Check
                info.severity = EXCEPTION_SEVERITY_FATAL;
                info.fatal = true;
                break;
                
            case 19: // SIMD Floating-Point Exception
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
                
            default:
                info.severity = EXCEPTION_SEVERITY_ERROR;
                info.handled = true;
                break;
        }
    } else if (vector == 32) {
        // Timer interrupt - handle normally
        handle_timer_interrupt();
        return;
    } else {
        // Other hardware interrupts
        handle_hardware_interrupt(vector);
        return;
    }
    
    // Log the exception
    log_exception(&info);
    
    // Handle fatal exceptions
    if (info.fatal) {
        KERROR("Fatal exception occurred - system panic");
        dump_registers(frame);
        dump_stack_trace(frame->rbp);
        hal_halt();
    }
    
    // Handle unhandled exceptions
    if (!info.handled) {
        KERROR("Unhandled exception %u: %s", (u32)vector, 
               vector < 32 ? exception_names[vector] : "Unknown");
        dump_registers(frame);
        
        // If in kernel mode, panic
        if (is_kernel_mode(frame->cs)) {
            KERROR("Kernel exception - system panic");
            hal_halt();
        } else {
            // If in user mode, kill the process
            process_t* process = get_current_process();
            if (process) {
                KERROR("Killing process %d due to unhandled exception", process->pid);
                process_terminate(process, -1);
            }
        }
    }
}

static void handle_critical_exception(exception_info_t* info) {
    // Switch to a known-good IST stack for critical exception handling
    tss_switch_to_interrupt_stack(IST_DOUBLE_FAULT);
    
    KERROR("Critical exception %u handled with IST stack", (u32)info->vector);
}

static void handle_page_fault(exception_info_t* info) {
    u64 fault_address = hal_get_cr2();
    bool present = (info->error_code & 1) != 0;
    bool write = (info->error_code & 2) != 0;
    bool user = (info->error_code & 4) != 0;
    bool reserved = (info->error_code & 8) != 0;
    bool instruction = (info->error_code & 16) != 0;
    
    KERROR("Page Fault at 0x%llx", fault_address);
    KERROR("  Error Code: 0x%llx", info->error_code);
    KERROR("  Present: %s", present ? "Yes" : "No");
    KERROR("  Write: %s", write ? "Yes" : "No");
    KERROR("  User: %s", user ? "Yes" : "No");
    KERROR("  Reserved: %s", reserved ? "Yes" : "No");
    KERROR("  Instruction: %s", instruction ? "Yes" : "No");
    KERROR("  RIP: 0x%llx", info->rip);
    
    // Try to handle the page fault
    if (!present) {
        // Page not present - try to page in
        if (memory_handle_page_fault(fault_address, write, user)) {
            info->handled = true;
            return;
        }
    }
    
    // If we can't handle it, it's a real error
    info->handled = false;
}

static void handle_double_fault(exception_info_t* info) {
    KERROR("Double Fault detected!");
    KERROR("  Error Code: 0x%llx", info->error_code);
    KERROR("  RIP: 0x%llx", info->rip);
    KERROR("  RSP: 0x%llx", info->rsp);
    
    // Double fault is always fatal
    info->fatal = true;
    info->handled = false;
}

static void handle_general_protection_fault(exception_info_t* info) {
    KERROR("General Protection Fault!");
    KERROR("  Error Code: 0x%llx", info->error_code);
    KERROR("  RIP: 0x%llx", info->rip);
    KERROR("  CS: 0x%llx", info->cs);
    
    // Check if it's a user-mode fault
    if (is_kernel_mode(info->cs)) {
        KERROR("Kernel GPF - system panic");
        info->fatal = true;
    } else {
        KERROR("User-mode GPF - terminating process");
        info->handled = true;
    }
}

static void handle_invalid_opcode(exception_info_t* info) {
    KERROR("Invalid Opcode at 0x%llx", info->rip);
    
    // Check if it's a user-mode fault
    if (is_kernel_mode(info->cs)) {
        KERROR("Kernel invalid opcode - system panic");
        info->fatal = true;
    } else {
        KERROR("User-mode invalid opcode - terminating process");
        info->handled = true;
    }
}

static void handle_divide_error(exception_info_t* info) {
    KERROR("Divide Error at 0x%llx", info->rip);
    
    // Check if it's a user-mode fault
    if (is_kernel_mode(info->cs)) {
        KERROR("Kernel divide error - system panic");
        info->fatal = true;
    } else {
        KERROR("User-mode divide error - terminating process");
        info->handled = true;
    }
}

static void handle_debug_exception(exception_info_t* info) {
    KINFO("Debug exception at 0x%llx", info->rip);
    info->handled = true;
}

static void handle_overflow(exception_info_t* info) {
    KERROR("Overflow exception at 0x%llx", info->rip);
    
    // Check if it's a user-mode fault
    if (is_kernel_mode(info->cs)) {
        KERROR("Kernel overflow - system panic");
        info->fatal = true;
    } else {
        KERROR("User-mode overflow - terminating process");
        info->handled = true;
    }
}

static void handle_bound_range(exception_info_t* info) {
    KERROR("Bound Range Exceeded at 0x%llx", info->rip);
    
    // Check if it's a user-mode fault
    if (is_kernel_mode(info->cs)) {
        KERROR("Kernel bound range - system panic");
        info->fatal = true;
    } else {
        KERROR("User-mode bound range - terminating process");
        info->handled = true;
    }
}

static void handle_nmi(exception_info_t* info) {
    KWARN("Non-Maskable Interrupt received");
    info->handled = true;
}

static void log_exception(exception_info_t* info) {
    const char* severity_str = "INFO";
    switch (info->severity) {
        case EXCEPTION_SEVERITY_WARNING: severity_str = "WARN"; break;
        case EXCEPTION_SEVERITY_ERROR: severity_str = "ERROR"; break;
        case EXCEPTION_SEVERITY_FATAL: severity_str = "FATAL"; break;
        default: break;
    }
    
    if (info->vector < 32) {
        KLOG("%s: Exception %u (%s) at 0x%llx", severity_str, 
             (u32)info->vector, exception_names[info->vector], info->rip);
    } else {
        KLOG("%s: Interrupt %u at 0x%llx", severity_str, (u32)info->vector, info->rip);
    }
}

static void dump_registers(interrupt_frame_t* frame) {
    KERROR("Register dump:");
    KERROR("  RAX: 0x%llx  RBX: 0x%llx  RCX: 0x%llx  RDX: 0x%llx", 
           frame->rax, frame->rbx, frame->rcx, frame->rdx);
    KERROR("  RSI: 0x%llx  RDI: 0x%llx  RBP: 0x%llx  RSP: 0x%llx", 
           frame->rsi, frame->rdi, frame->rbp, frame->rsp);
    KERROR("  R8:  0x%llx  R9:  0x%llx  R10: 0x%llx  R11: 0x%llx", 
           frame->r8, frame->r9, frame->r10, frame->r11);
    KERROR("  R12: 0x%llx  R13: 0x%llx  R14: 0x%llx  R15: 0x%llx", 
           frame->r12, frame->r13, frame->r14, frame->r15);
    KERROR("  RIP: 0x%llx  RFLAGS: 0x%llx", frame->rip, frame->rflags);
    KERROR("  CS: 0x%llx  SS: 0x%llx", frame->cs, frame->ss);
}

static void dump_stack_trace(u64 rbp) {
    KERROR("Stack trace:");
    u64 frame_ptr = rbp;
    int depth = 0;
    
    while (frame_ptr && depth < 20) {
        u64* frame = (u64*)frame_ptr;
        u64 return_addr = frame[1];
        
        if (return_addr >= 0x1000 && return_addr < 0x7FFFFFFFFFFF) {
            KERROR("  [%d] 0x%llx", depth, return_addr);
        }
        
        frame_ptr = frame[0];
        depth++;
    }
}

static bool is_kernel_mode(u64 cs) {
    return (cs & 3) == 0;
}

static process_t* get_current_process(void) {
    // TODO: Implement current process retrieval
    return NULL;
}

void handle_timer_interrupt(void) {
    // Handle timer interrupt
    // This would update system time, scheduler, etc.
}

void handle_hardware_interrupt(u64 vector) {
    // Handle hardware interrupts
    KDEBUG("Hardware interrupt %u", (u32)vector);
} 