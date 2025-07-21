#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"
#include <stddef.h>

// Interrupt descriptor table constants
#define IDT_ENTRIES 256
#define IDT_TYPE_INTERRUPT_GATE 0x8E
#define IDT_TYPE_TRAP_GATE 0x8F

// Exception numbers
#define EXCEPTION_DIVIDE_BY_ZERO          0
#define EXCEPTION_DEBUG                   1
#define EXCEPTION_NMI                     2
#define EXCEPTION_BREAKPOINT              3
#define EXCEPTION_OVERFLOW                4
#define EXCEPTION_BOUND_RANGE_EXCEEDED    5
#define EXCEPTION_INVALID_OPCODE          6
#define EXCEPTION_DEVICE_NOT_AVAILABLE    7
#define EXCEPTION_DOUBLE_FAULT            8
#define EXCEPTION_INVALID_TSS             10
#define EXCEPTION_SEGMENT_NOT_PRESENT     11
#define EXCEPTION_STACK_SEGMENT_FAULT     12
#define EXCEPTION_GENERAL_PROTECTION      13
#define EXCEPTION_PAGE_FAULT              14
#define EXCEPTION_X87_FPU_ERROR           16
#define EXCEPTION_ALIGNMENT_CHECK         17
#define EXCEPTION_MACHINE_CHECK           18
#define EXCEPTION_SIMD_FPU_ERROR          19

// IRQ numbers
#define IRQ_TIMER          0
#define IRQ_KEYBOARD       1
#define IRQ_CASCADE        2
#define IRQ_SERIAL_PORT2   3
#define IRQ_SERIAL_PORT1   4
#define IRQ_PARALLEL_PORT2 5
#define IRQ_FLOPPY         6
#define IRQ_PARALLEL_PORT1 7
#define IRQ_RTC            8
#define IRQ_ACPI           9
#define IRQ_AVAILABLE1     10
#define IRQ_AVAILABLE2     11
#define IRQ_MOUSE          12
#define IRQ_FPU            13
#define IRQ_PRIMARY_ATA    14
#define IRQ_SECONDARY_ATA  15

// Special interrupt numbers
#define SYSCALL_INTERRUPT  128
#define SPURIOUS_INTERRUPT 255

// IRQ base offset
#define IRQ_BASE 32

// PIC constants
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

// Interrupt context structure (architecture-specific)
#ifdef __x86_64__
typedef struct {
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11, r12, r13, r14, r15;
    u64 rip, rflags;
    u64 cs, ds, es, fs, gs, ss;
    u64 error_code;
    u64 interrupt_number;
} interrupt_context_t;
#else
typedef struct {
    u32 eax, ebx, ecx, edx;
    u32 esi, edi, ebp, esp;
    u32 eip, eflags;
    u32 cs, ds, es, fs, gs, ss;
    u32 error_code;
    u32 interrupt_number;
} interrupt_context_t;
#endif

// IDT entry structure
typedef struct {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 type_attr;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
} __attribute__((packed)) idt_entry_t;

// IDT descriptor structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) idt_descriptor_t;

// Interrupt handler function type
typedef void (*interrupt_handler_func_t)(interrupt_context_t* context);

// Interrupt statistics
typedef struct {
    u64 exception_counts[32];
    u64 irq_counts[16];
    u64 syscall_count;
    u64 spurious_count;
    u64 total_interrupts;
    u64 hardware_interrupts;
    u64 software_interrupts;
    u64 exceptions;
    u64 spurious_interrupts;
    u64 interrupt_count[IDT_ENTRIES];
} interrupt_stats_t;

// Critical section type
typedef struct {
    bool interrupts_were_enabled;
} critical_section_t;

// Function declarations
error_t interrupts_init(void);
void interrupts_shutdown(void);
void interrupts_enable(void);
void interrupts_disable(void);
bool interrupts_are_enabled(void);

// IDT management
error_t idt_init(void);
void idt_load(void);
void idt_set_entry(u8 vector, void* handler, u8 type, u8 dpl, u8 ist);

// PIC management
error_t pic_init(void);
void pic_send_eoi(u8 irq);
void pic_mask_irq(u8 irq);
void pic_unmask_irq(u8 irq);

// Interrupt handler management
error_t register_interrupt_handler(u8 vector, interrupt_handler_func_t handler);
error_t unregister_interrupt_handler(u8 vector);
interrupt_handler_func_t get_interrupt_handler(u8 vector);

// Interrupt dispatch
void interrupt_dispatch(interrupt_context_t* context);

// Critical sections
critical_section_t enter_critical_section(void);
void exit_critical_section(critical_section_t state);

// Statistics and debugging
void dump_interrupt_stats(void);
void dump_interrupt_context(interrupt_context_t* context);

// Exception handlers
void exception_divide_by_zero(interrupt_context_t* context);
void exception_debug(interrupt_context_t* context);
void exception_nmi(interrupt_context_t* context);
void exception_breakpoint(interrupt_context_t* context);
void exception_overflow(interrupt_context_t* context);
void exception_bound_range_exceeded(interrupt_context_t* context);
void exception_invalid_opcode(interrupt_context_t* context);
void exception_device_not_available(interrupt_context_t* context);
void exception_double_fault(interrupt_context_t* context);
void exception_invalid_tss(interrupt_context_t* context);
void exception_segment_not_present(interrupt_context_t* context);
void exception_stack_segment_fault(interrupt_context_t* context);
void exception_general_protection(interrupt_context_t* context);
void exception_page_fault(interrupt_context_t* context);
void exception_x87_fpu_error(interrupt_context_t* context);
void exception_alignment_check(interrupt_context_t* context);
void exception_machine_check(interrupt_context_t* context);
void exception_simd_fpu_error(interrupt_context_t* context);

// IRQ handlers
void irq_timer(interrupt_context_t* context);
void irq_keyboard(interrupt_context_t* context);
void irq_rtc(interrupt_context_t* context);
void irq_mouse(interrupt_context_t* context);
void irq_spurious(interrupt_context_t* context);

// System call handler
void syscall_handler(interrupt_context_t* context);

// Assembly interrupt stubs (defined in interrupt_stubs.asm)
extern void interrupt_stub_0(void);
extern void interrupt_stub_1(void);
extern void interrupt_stub_2(void);
extern void interrupt_stub_3(void);
extern void interrupt_stub_4(void);
extern void interrupt_stub_5(void);
extern void interrupt_stub_6(void);
extern void interrupt_stub_7(void);
extern void interrupt_stub_8(void);
extern void interrupt_stub_10(void);
extern void interrupt_stub_11(void);
extern void interrupt_stub_12(void);
extern void interrupt_stub_13(void);
extern void interrupt_stub_14(void);
extern void interrupt_stub_16(void);
extern void interrupt_stub_17(void);
extern void interrupt_stub_18(void);
extern void interrupt_stub_19(void);
extern void interrupt_stub_32(void);
extern void interrupt_stub_33(void);
extern void interrupt_stub_34(void);
extern void interrupt_stub_35(void);
extern void interrupt_stub_36(void);
extern void interrupt_stub_37(void);
extern void interrupt_stub_38(void);
extern void interrupt_stub_39(void);
extern void interrupt_stub_40(void);
extern void interrupt_stub_41(void);
extern void interrupt_stub_42(void);
extern void interrupt_stub_43(void);
extern void interrupt_stub_44(void);
extern void interrupt_stub_45(void);
extern void interrupt_stub_46(void);
extern void interrupt_stub_47(void);
extern void interrupt_stub_128(void);
extern void interrupt_stub_255(void);

#endif // INTERRUPTS_H