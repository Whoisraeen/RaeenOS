#include "include/interrupts.h"
#include "include/kernel.h"
#include "../memory/include/memory.h"
#include "../hal/include/hal.h"
#include <string.h>

// Global interrupt state
static idt_entry_t idt[IDT_ENTRIES];
static idt_descriptor_t idt_descriptor;
static interrupt_handler_func_t interrupt_handlers[IDT_ENTRIES];
static interrupt_stats_t interrupt_statistics = {0};
static bool interrupts_initialized = false;

// Set up an IDT entry
void idt_set_entry(u8 vector, void* handler, u8 type, u8 dpl, u8 ist) {
    u64 handler_addr = (u64)handler;
    
    idt[vector].offset_low = handler_addr & 0xFFFF;
    idt[vector].selector = 0x08; // Kernel code segment
    idt[vector].ist = ist;
    idt[vector].type_attr = type | (dpl << 5);
    idt[vector].offset_mid = (handler_addr >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

// Load the IDT
void idt_load(void) {
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (u64)&idt;
    
    __asm__ volatile ("lidt %0" :: "m" (idt_descriptor));
}

// Initialize IDT with default handlers
error_t idt_init(void) {
    KDEBUG("Initializing Interrupt Descriptor Table");
    
    // Clear IDT and handlers
    memset(idt, 0, sizeof(idt));
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));
    
    // Set up exception handlers
    idt_set_entry(EXCEPTION_DIVIDE_BY_ZERO, interrupt_stub_0, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_DEBUG, interrupt_stub_1, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_NMI, interrupt_stub_2, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_BREAKPOINT, interrupt_stub_3, IDT_TYPE_TRAP_GATE, 3, 0);
    idt_set_entry(EXCEPTION_OVERFLOW, interrupt_stub_4, IDT_TYPE_TRAP_GATE, 3, 0);
    idt_set_entry(EXCEPTION_BOUND_RANGE_EXCEEDED, interrupt_stub_5, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_INVALID_OPCODE, interrupt_stub_6, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_DEVICE_NOT_AVAILABLE, interrupt_stub_7, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_DOUBLE_FAULT, interrupt_stub_8, IDT_TYPE_INTERRUPT_GATE, 0, 1);
    idt_set_entry(EXCEPTION_INVALID_TSS, interrupt_stub_10, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_SEGMENT_NOT_PRESENT, interrupt_stub_11, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_STACK_SEGMENT_FAULT, interrupt_stub_12, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_GENERAL_PROTECTION, interrupt_stub_13, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_PAGE_FAULT, interrupt_stub_14, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_X87_FPU_ERROR, interrupt_stub_16, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_ALIGNMENT_CHECK, interrupt_stub_17, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_MACHINE_CHECK, interrupt_stub_18, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(EXCEPTION_SIMD_FPU_ERROR, interrupt_stub_19, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    
    // Set up IRQ handlers
    idt_set_entry(IRQ_BASE + IRQ_TIMER, interrupt_stub_32, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_BASE + IRQ_KEYBOARD, interrupt_stub_33, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_BASE + IRQ_CASCADE, interrupt_stub_34, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_BASE + IRQ_SERIAL_PORT2, interrupt_stub_35, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_BASE + IRQ_SERIAL_PORT1, interrupt_stub_36, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_PARALLEL_PORT2, interrupt_stub_37, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_FLOPPY, interrupt_stub_38, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_PARALLEL_PORT1, interrupt_stub_39, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_RTC, interrupt_stub_40, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_ACPI, interrupt_stub_41, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_AVAILABLE1, interrupt_stub_42, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_AVAILABLE2, interrupt_stub_43, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_MOUSE, interrupt_stub_44, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_FPU, interrupt_stub_45, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_PRIMARY_ATA, interrupt_stub_46, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    idt_set_entry(IRQ_SECONDARY_ATA, interrupt_stub_47, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    
    // Set up system call handler
    idt_set_entry(SYSCALL_INTERRUPT, interrupt_stub_128, IDT_TYPE_INTERRUPT_GATE, 3, 0);
    
    // Set up spurious interrupt handler
    idt_set_entry(SPURIOUS_INTERRUPT, interrupt_stub_255, IDT_TYPE_INTERRUPT_GATE, 0, 0);
    
    // Register default exception handlers
    register_interrupt_handler(EXCEPTION_DIVIDE_BY_ZERO, exception_divide_by_zero);
    register_interrupt_handler(EXCEPTION_DEBUG, exception_debug);
    register_interrupt_handler(EXCEPTION_NMI, exception_nmi);
    register_interrupt_handler(EXCEPTION_BREAKPOINT, exception_breakpoint);
    register_interrupt_handler(EXCEPTION_OVERFLOW, exception_overflow);
    register_interrupt_handler(EXCEPTION_BOUND_RANGE_EXCEEDED, exception_bound_range_exceeded);
    register_interrupt_handler(EXCEPTION_INVALID_OPCODE, exception_invalid_opcode);
    register_interrupt_handler(EXCEPTION_DEVICE_NOT_AVAILABLE, exception_device_not_available);
    register_interrupt_handler(EXCEPTION_DOUBLE_FAULT, exception_double_fault);
    register_interrupt_handler(EXCEPTION_INVALID_TSS, exception_invalid_tss);
    register_interrupt_handler(EXCEPTION_SEGMENT_NOT_PRESENT, exception_segment_not_present);
    register_interrupt_handler(EXCEPTION_STACK_SEGMENT_FAULT, exception_stack_segment_fault);
    register_interrupt_handler(EXCEPTION_GENERAL_PROTECTION, exception_general_protection);
    register_interrupt_handler(EXCEPTION_PAGE_FAULT, exception_page_fault);
    register_interrupt_handler(EXCEPTION_X87_FPU_ERROR, exception_x87_fpu_error);
    register_interrupt_handler(EXCEPTION_ALIGNMENT_CHECK, exception_alignment_check);
    register_interrupt_handler(EXCEPTION_MACHINE_CHECK, exception_machine_check);
    register_interrupt_handler(EXCEPTION_SIMD_FPU_ERROR, exception_simd_fpu_error);
    
    // Register default IRQ handlers
    register_interrupt_handler(IRQ_BASE + IRQ_TIMER, irq_timer);
    register_interrupt_handler(IRQ_BASE + IRQ_KEYBOARD, irq_keyboard);
    register_interrupt_handler(IRQ_BASE + IRQ_RTC, irq_rtc);
    register_interrupt_handler(IRQ_BASE + IRQ_MOUSE, irq_mouse);
    register_interrupt_handler(SPURIOUS_INTERRUPT, irq_spurious);
    
    // Register system call handler
    register_interrupt_handler(SYSCALL_INTERRUPT, syscall_handler);
    
    // Load the IDT
    idt_load();
    
    KDEBUG("IDT initialized with %d entries", IDT_ENTRIES);
    return SUCCESS;
}

// Initialize the 8259 PIC
error_t pic_init(void) {
    KDEBUG("Initializing 8259 PIC");
    
    // Save masks
    u8 mask1 = hal_inb(PIC1_DATA);
    u8 mask2 = hal_inb(PIC2_DATA);
    
    // Start initialization sequence
    hal_outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    hal_outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // Set vector offsets
    hal_outb(PIC1_DATA, IRQ_BASE);         // Master PIC starts at IRQ_BASE
    hal_outb(PIC2_DATA, IRQ_BASE + 8);     // Slave PIC starts at IRQ_BASE + 8
    
    // Configure cascade
    hal_outb(PIC1_DATA, 4);                // Tell master PIC there's a slave at IRQ2
    hal_outb(PIC2_DATA, 2);                // Tell slave PIC its cascade identity
    
    // Set modes
    hal_outb(PIC1_DATA, ICW4_8086);
    hal_outb(PIC2_DATA, ICW4_8086);
    
    // Restore masks
    hal_outb(PIC1_DATA, mask1);
    hal_outb(PIC2_DATA, mask2);
    
    KDEBUG("PIC initialized");
    return SUCCESS;
}

// Send End of Interrupt to PIC
void pic_send_eoi(u8 irq) {
    if (irq >= 8) {
        hal_outb(PIC2_COMMAND, PIC_EOI);
    }
    hal_outb(PIC1_COMMAND, PIC_EOI);
}

// Mask/unmask IRQs
void pic_mask_irq(u8 irq) {
    u16 port;
    u8 value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = hal_inb(port) | (1 << irq);
    hal_outb(port, value);
}

void pic_unmask_irq(u8 irq) {
    u16 port;
    u8 value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = hal_inb(port) & ~(1 << irq);
    hal_outb(port, value);
}

// Initialize interrupt system
error_t interrupts_init(void) {
    if (interrupts_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing interrupt handling system");
    
    // Initialize statistics
    memset(&interrupt_statistics, 0, sizeof(interrupt_statistics));
    
    // Initialize IDT
    error_t result = idt_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize IDT: %d", result);
        return result;
    }
    
    // Initialize PIC
    result = pic_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize PIC: %d", result);
        return result;
    }
    
    // Mask all IRQs initially except timer and keyboard
    for (u8 i = 0; i < 16; i++) {
        if (i != 0 && i != 1) { // Keep timer and keyboard enabled
            pic_mask_irq(i);
        }
    }
    
    interrupts_initialized = true;
    
    KINFO("Interrupt handling system initialized");
    return SUCCESS;
}

void interrupts_shutdown(void) {
    if (!interrupts_initialized) {
        return;
    }
    
    KINFO("Shutting down interrupt handling system");
    
    // Disable all interrupts
    interrupts_disable();
    
    // Mask all IRQs
    for (u8 i = 0; i < 16; i++) {
        pic_mask_irq(i);
    }
    
    interrupts_initialized = false;
}

// Interrupt handler registration
error_t register_interrupt_handler(u8 vector, interrupt_handler_func_t handler) {
    if (!handler) {
        return E_INVAL;
    }
    
    interrupt_handlers[vector] = handler;
    KDEBUG("Registered interrupt handler for vector %u", vector);
    return SUCCESS;
}

error_t unregister_interrupt_handler(u8 vector) {
    interrupt_handlers[vector] = NULL;
    KDEBUG("Unregistered interrupt handler for vector %u", vector);
    return SUCCESS;
}

interrupt_handler_func_t get_interrupt_handler(u8 vector) {
    return interrupt_handlers[vector];
}

// Common interrupt handler (called from assembly stubs)
void interrupt_dispatch(interrupt_context_t* context) {
    u8 vector = (u8)context->interrupt_number;
    
    // Update statistics
    interrupt_statistics.total_interrupts++;
    interrupt_statistics.interrupt_count[vector]++;
    
    if (vector < 32) {
        interrupt_statistics.exceptions++;
    } else if (vector >= IRQ_BASE && vector < IRQ_BASE + 16) {
        interrupt_statistics.hardware_interrupts++;
    } else {
        interrupt_statistics.software_interrupts++;
    }
    
    // Call registered handler
    interrupt_handler_func_t handler = interrupt_handlers[vector];
    if (handler) {
        handler(context);
    } else {
        KWARN("Unhandled interrupt: vector=%u, error_code=0x%llx", 
              vector, context->error_code);
    }
    
    // Send EOI for hardware interrupts
    if (vector >= IRQ_BASE && vector < IRQ_BASE + 16) {
        pic_send_eoi(vector - IRQ_BASE);
    }
}

// Critical section management
critical_section_t enter_critical_section(void) {
    critical_section_t state;
    state.interrupts_were_enabled = interrupts_are_enabled();
    interrupts_disable();
    return state;
}

void exit_critical_section(critical_section_t state) {
    if (state.interrupts_were_enabled) {
        interrupts_enable();
    }
}

// Interrupt control
bool interrupts_are_enabled(void) {
    return hal_are_interrupts_enabled();
}

void interrupts_enable(void) {
    hal_enable_interrupts();
}

void interrupts_disable(void) {
    hal_disable_interrupts();
}

// Statistics and debugging
interrupt_stats_t* get_interrupt_stats(void) {
    return &interrupt_statistics;
}

void dump_interrupt_stats(void) {
    hal_console_print("Interrupt Statistics:\n");
    hal_console_print("  Total interrupts:    %llu\n", interrupt_statistics.total_interrupts);
    hal_console_print("  Hardware interrupts: %llu\n", interrupt_statistics.hardware_interrupts);
    hal_console_print("  Software interrupts: %llu\n", interrupt_statistics.software_interrupts);
    hal_console_print("  Exceptions:          %llu\n", interrupt_statistics.exceptions);
    hal_console_print("  Spurious interrupts: %llu\n", interrupt_statistics.spurious_interrupts);
    
    hal_console_print("\nTop interrupt vectors:\n");
    for (int i = 0; i < 16; i++) {
        u64 max_count = 0;
        u8 max_vector = 0;
        
        for (u16 j = 0; j < IDT_ENTRIES; j++) {
            if (interrupt_statistics.interrupt_count[j] > max_count) {
                max_count = interrupt_statistics.interrupt_count[j];
                max_vector = j;
            }
        }
        
        if (max_count > 0) {
            hal_console_print("  Vector %3u: %llu interrupts\n", max_vector, max_count);
            interrupt_statistics.interrupt_count[max_vector] = 0; // Clear for next iteration
        }
    }
}

void dump_interrupt_context(interrupt_context_t* context) {
    hal_console_print("Interrupt Context Dump:\n");
    hal_console_print("  Vector:      %llu\n", context->interrupt_number);
    hal_console_print("  Error Code:  0x%llx\n", context->error_code);
    hal_console_print("  RIP:         0x%llx\n", context->rip);
    hal_console_print("  CS:          0x%llx\n", context->cs);
    hal_console_print("  RFLAGS:      0x%llx\n", context->rflags);
    hal_console_print("  RSP:         0x%llx\n", context->rsp);
    hal_console_print("  SS:          0x%llx\n", context->ss);
    hal_console_print("  RAX:         0x%llx\n", context->rax);
    hal_console_print("  RBX:         0x%llx\n", context->rbx);
    hal_console_print("  RCX:         0x%llx\n", context->rcx);
    hal_console_print("  RDX:         0x%llx\n", context->rdx);
}

// Exception handler implementations
void exception_divide_by_zero(interrupt_context_t* context) {
    KERROR("#DE: Divide by Zero Exception");
    dump_interrupt_context(context);
    KERNEL_PANIC("Divide by zero");
}
void exception_debug(interrupt_context_t* context) {
    KWARN("#DB: Debug Exception");
    dump_interrupt_context(context);
}
void exception_nmi(interrupt_context_t* context) {
    KERROR("#NMI: Non-Maskable Interrupt");
    dump_interrupt_context(context);
    KERNEL_PANIC("Non-maskable interrupt");
}
void exception_breakpoint(interrupt_context_t* context) {
    KWARN("#BP: Breakpoint Exception");
    dump_interrupt_context(context);
}
void exception_overflow(interrupt_context_t* context) {
    KWARN("#OF: Overflow Exception");
    dump_interrupt_context(context);
}
void exception_bound_range_exceeded(interrupt_context_t* context) {
    KERROR("#BR: Bound Range Exceeded");
    dump_interrupt_context(context);
    KERNEL_PANIC("Bound range exceeded");
}
void exception_invalid_opcode(interrupt_context_t* context) {
    KERROR("#UD: Invalid Opcode");
    dump_interrupt_context(context);
    KERNEL_PANIC("Invalid opcode");
}
void exception_device_not_available(interrupt_context_t* context) {
    KERROR("#NM: Device Not Available");
    dump_interrupt_context(context);
    KERNEL_PANIC("Device not available");
}
void exception_double_fault(interrupt_context_t* context) {
    KERROR("#DF: Double Fault");
    dump_interrupt_context(context);
    KERNEL_PANIC("Double fault");
}
void exception_invalid_tss(interrupt_context_t* context) {
    KERROR("#TS: Invalid TSS");
    dump_interrupt_context(context);
    KERNEL_PANIC("Invalid TSS");
}
void exception_segment_not_present(interrupt_context_t* context) {
    KERROR("#NP: Segment Not Present");
    dump_interrupt_context(context);
    KERNEL_PANIC("Segment not present");
}
void exception_stack_segment_fault(interrupt_context_t* context) {
    KERROR("#SS: Stack Segment Fault");
    dump_interrupt_context(context);
    KERNEL_PANIC("Stack segment fault");
}
void exception_general_protection(interrupt_context_t* context) {
    KERROR("#GP: General Protection Fault");
    dump_interrupt_context(context);
    KERNEL_PANIC("General protection fault");
}
void exception_page_fault(interrupt_context_t* context) {
    KERROR("#PF: Page Fault");
    dump_interrupt_context(context);
    KERNEL_PANIC("Page fault");
}
void exception_x87_fpu_error(interrupt_context_t* context) {
    KERROR("#MF: x87 FPU Floating-Point Error");
    dump_interrupt_context(context);
    KERNEL_PANIC("x87 FPU error");
}
void exception_alignment_check(interrupt_context_t* context) {
    KERROR("#AC: Alignment Check");
    dump_interrupt_context(context);
    KERNEL_PANIC("Alignment check");
}
void exception_machine_check(interrupt_context_t* context) {
    KERROR("#MC: Machine Check");
    dump_interrupt_context(context);
    KERNEL_PANIC("Machine check");
}
void exception_simd_fpu_error(interrupt_context_t* context) {
    KERROR("#XM: SIMD Floating-Point Exception");
    dump_interrupt_context(context);
    KERNEL_PANIC("SIMD FPU error");
}