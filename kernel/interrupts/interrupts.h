#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"
#include <stddef.h>

// Interrupt vector numbers
#define IRQ_TIMER 0
#define IRQ_KEYBOARD 1
#define IRQ_CASCADE 2
#define IRQ_COM2 3
#define IRQ_COM1 4
#define IRQ_LPT2 5
#define IRQ_FLOPPY 6
#define IRQ_LPT1 7
#define IRQ_RTC 8
#define IRQ_ACPI 9
#define IRQ_RESERVED1 10
#define IRQ_RESERVED2 11
#define IRQ_PS2 12
#define IRQ_FPU 13
#define IRQ_PRIMARY_ATA 14
#define IRQ_SECONDARY_ATA 15

// Exception numbers
#define EXCEPTION_DIVIDE_ERROR 0
#define EXCEPTION_DEBUG 1
#define EXCEPTION_NMI 2
#define EXCEPTION_BREAKPOINT 3
#define EXCEPTION_OVERFLOW 4
#define EXCEPTION_BOUND_RANGE 5
#define EXCEPTION_INVALID_OPCODE 6
#define EXCEPTION_DEVICE_NOT_AVAILABLE 7
#define EXCEPTION_DOUBLE_FAULT 8
#define EXCEPTION_COPROCESSOR_SEGMENT 9
#define EXCEPTION_INVALID_TSS 10
#define EXCEPTION_SEGMENT_NOT_PRESENT 11
#define EXCEPTION_STACK_SEGMENT_FAULT 12
#define EXCEPTION_GENERAL_PROTECTION 13
#define EXCEPTION_PAGE_FAULT 14
#define EXCEPTION_RESERVED1 15
#define EXCEPTION_FPU_ERROR 16
#define EXCEPTION_ALIGNMENT_CHECK 17
#define EXCEPTION_MACHINE_CHECK 18
#define EXCEPTION_SIMD_FPU_ERROR 19
#define EXCEPTION_VIRTUALIZATION 20
#define EXCEPTION_RESERVED2 21
#define EXCEPTION_RESERVED3 22
#define EXCEPTION_RESERVED4 23
#define EXCEPTION_RESERVED5 24
#define EXCEPTION_RESERVED6 25
#define EXCEPTION_RESERVED7 26
#define EXCEPTION_RESERVED8 27
#define EXCEPTION_HYPERVISOR_INJECTION 28
#define EXCEPTION_VMM_COMMUNICATION 29
#define EXCEPTION_SECURITY 30
#define EXCEPTION_RESERVED9 31

// Interrupt flags
#define INTERRUPT_FLAG_ENABLED (1 << 0)
#define INTERRUPT_FLAG_DISABLED (1 << 1)
#define INTERRUPT_FLAG_MASKED (1 << 2)
#define INTERRUPT_FLAG_PENDING (1 << 3)

// Interrupt handler function type
typedef void (*interrupt_handler_t)(u32 interrupt_num, void* context);

// Interrupt context structure
typedef struct interrupt_context {
    // General purpose registers
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11, r12, r13, r14, r15;
    
    // Instruction pointer and flags
    u64 rip, rflags;
    
    // Segment registers
    u64 cs, ds, es, fs, gs, ss;
    
    // Error code (for exceptions)
    u64 error_code;
    
    // Interrupt number
    u32 interrupt_num;
    
    // Additional context
    void* user_context;
    
} interrupt_context_t;

// Interrupt descriptor structure
typedef struct interrupt_descriptor {
    u16 offset_low;
    u16 segment_selector;
    u8 ist;
    u8 type_attributes;
    u16 offset_middle;
    u32 offset_high;
    u32 reserved;
} __attribute__((packed)) interrupt_descriptor_t;

// Interrupt table structure
typedef struct interrupt_table {
    interrupt_descriptor_t descriptors[256];
} __attribute__((packed)) interrupt_table_t;

// Interrupt controller structure
typedef struct interrupt_controller {
    // PIC/APIC registers
    void* pic_master_command;
    void* pic_master_data;
    void* pic_slave_command;
    void* pic_slave_data;
    
    // APIC registers
    void* apic_base;
    u32 apic_id;
    u32 apic_version;
    
    // Interrupt handlers
    interrupt_handler_t handlers[256];
    void* handler_contexts[256];
    
    // Interrupt statistics
    u64 interrupt_counts[256];
    u64 last_interrupt_time;
    
    // Configuration
    bool apic_enabled;
    bool pic_enabled;
    u32 spurious_count;
    
} interrupt_controller_t;

// Exception information structure
typedef struct exception_info {
    u32 exception_number;
    u64 fault_address;
    u64 error_code;
    char description[64];
} exception_info_t;

// Interrupt management functions
error_t interrupts_init(void);
void interrupts_shutdown(void);

// Interrupt handler registration
error_t interrupt_register_handler(u32 interrupt_num, interrupt_handler_t handler, void* context);
error_t interrupt_unregister_handler(u32 interrupt_num);
error_t interrupt_set_handler(u32 interrupt_num, interrupt_handler_t handler);

// Interrupt control
void interrupt_enable(void);
void interrupt_disable(void);
void interrupt_enable_irq(u32 irq);
void interrupt_disable_irq(u32 irq);
void interrupt_send_eoi(u32 irq);

// PIC/APIC management
error_t pic_init(void);
error_t apic_init(void);
void pic_send_eoi(u32 irq);
void apic_send_eoi(void);

// Interrupt table management
error_t interrupt_setup_idt(void);
error_t interrupt_load_idt(interrupt_table_t* idt);
void interrupt_set_gate(u32 num, void* handler, u16 selector, u8 type, u8 ist);

// Exception handling
error_t exception_init(void);
void exception_handler(interrupt_context_t* context);
void exception_dump_context(interrupt_context_t* context);
const char* exception_get_description(u32 exception_num);

// Page fault handling
void page_fault_handler(interrupt_context_t* context);
error_t page_fault_resolve(virt_addr_t fault_address, u64 error_code);

// Interrupt debugging
void interrupt_dump_handlers(void);
void interrupt_dump_statistics(void);
void interrupt_validate_integrity(void);

// Architecture-specific functions
#ifdef __x86_64__
error_t interrupts_init_x64(void);
void interrupt_handler_x64(interrupt_context_t* context);
void interrupt_setup_idt_x64(void);
#else
error_t interrupts_init_x86(void);
void interrupt_handler_x86(interrupt_context_t* context);
void interrupt_setup_idt_x86(void);
#endif

// Interrupt macros
#define INTERRUPT_REGISTER(num, handler, context) \
    interrupt_register_handler(num, handler, context)

#define INTERRUPT_UNREGISTER(num) \
    interrupt_unregister_handler(num)

#define INTERRUPT_ENABLE() interrupt_enable()
#define INTERRUPT_DISABLE() interrupt_disable()

#define INTERRUPT_ENABLE_IRQ(irq) interrupt_enable_irq(irq)
#define INTERRUPT_DISABLE_IRQ(irq) interrupt_disable_irq(irq)

// Exception macros
#define EXCEPTION_HANDLER(exception_num, handler) \
    interrupt_register_handler(exception_num, handler, NULL)

// Interrupt context macros
#define INTERRUPT_CONTEXT_GET_REG(context, reg) ((context)->reg)
#define INTERRUPT_CONTEXT_SET_REG(context, reg, value) ((context)->reg = (value))

// Error code macros for page faults
#define PAGE_FAULT_PRESENT(error) ((error) & 0x01)
#define PAGE_FAULT_WRITE(error) ((error) & 0x02)
#define PAGE_FAULT_USER(error) ((error) & 0x04)
#define PAGE_FAULT_RESERVED(error) ((error) & 0x08)
#define PAGE_FAULT_INSTRUCTION(error) ((error) & 0x10)

#endif // INTERRUPTS_H 