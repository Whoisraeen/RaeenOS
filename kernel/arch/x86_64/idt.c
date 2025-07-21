#include "arch/x86_64/idt.h"
#include "arch/x86_64/tss.h"
#include "kernel.h"

idt_descriptor_t idt[IDT_ENTRIES];
idt_pointer_t idt_ptr;

void idt_set_descriptor(u8 vector, void* isr, u16 selector, u8 flags) {
    if (vector >= IDT_ENTRIES) {
        KERROR("IDT vector out of bounds: %u", vector);
        return;
    }
    
    idt_descriptor_t* descriptor = &idt[vector];
    descriptor->offset_low = (u16)((u64)isr & 0xFFFF);
    descriptor->selector = selector;
    descriptor->ist = 0; // Default to not using the IST
    descriptor->type_attr = flags;
    descriptor->offset_mid = (u16)(((u64)isr >> 16) & 0xFFFF);
    descriptor->offset_high = (u32)(((u64)isr >> 32) & 0xFFFFFFFF);
    descriptor->zero = 0;
}

void idt_set_descriptor_with_ist(u8 vector, void* isr, u16 selector, u8 flags, u8 ist_index) {
    if (vector >= IDT_ENTRIES) {
        KERROR("IDT vector out of bounds: %u", vector);
        return;
    }
    
    idt_descriptor_t* descriptor = &idt[vector];
    descriptor->offset_low = (u16)((u64)isr & 0xFFFF);
    descriptor->selector = selector;
    descriptor->ist = ist_index; // Use specified IST stack
    descriptor->type_attr = flags;
    descriptor->offset_mid = (u16)(((u64)isr >> 16) & 0xFFFF);
    descriptor->offset_high = (u32)(((u64)isr >> 32) & 0xFFFFFFFF);
    descriptor->zero = 0;
}

void idt_init(void) {
    KINFO("Initializing IDT...");

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (u64)&idt;

    // Zero out the IDT
    memset(&idt, 0, sizeof(idt));

    // Set interrupt descriptors for exceptions (vectors 0-31)
    // Use IST for critical exceptions that could have stack corruption issues
    
    idt_set_descriptor(0, isr0, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // #DE: Divide Error
    idt_set_descriptor(1, isr1, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // #DB: Debug
    idt_set_descriptor_with_ist(2, isr2, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_NMI); // NMI - use IST2
    idt_set_descriptor(3, isr3, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Breakpoint
    idt_set_descriptor_with_ist(4, isr4, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_OVERFLOW); // Overflow - use IST5
    idt_set_descriptor_with_ist(5, isr5, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_BOUND_RANGE); // Bound Range - use IST6
    idt_set_descriptor_with_ist(6, isr6, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_INVALID_OPCODE); // Invalid Opcode - use IST7
    idt_set_descriptor(7, isr7, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Device Not Available
    idt_set_descriptor_with_ist(8, isr8, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_DOUBLE_FAULT); // Double Fault - use IST1
    idt_set_descriptor(9, isr9, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Coprocessor Segment Overrun
    idt_set_descriptor(10, isr10, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Invalid TSS
    idt_set_descriptor(11, isr11, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Segment Not Present
    idt_set_descriptor(12, isr12, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // Stack-Segment Fault
    idt_set_descriptor(13, isr13, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE); // General Protection Fault
    idt_set_descriptor_with_ist(14, isr14, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE, IST_DOUBLE_FAULT); // Page Fault - use IST1
    idt_set_descriptor(15, isr15, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(16, isr16, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(17, isr17, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(18, isr18, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(19, isr19, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(20, isr20, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(21, isr21, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(22, isr22, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(23, isr23, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(24, isr24, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(25, isr25, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(26, isr26, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(27, isr27, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(28, isr28, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(29, isr29, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(30, isr30, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);
    idt_set_descriptor(31, isr31, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);

    // Set ISR for the timer interrupt (vector 32 = 0x20)
    idt_set_descriptor(32, timer_interrupt_handler_wrapper, 0x08, IDT_PRESENT | IDT_INTERRUPT_GATE);

    // Load the IDT
    x64_load_idt(&idt_ptr);

    KINFO("IDT initialized. IDT base: 0x%llx", idt_ptr.base);
    KINFO("IST stacks configured for critical exceptions:");
    KINFO("  Double Fault (vector 8): IST1");
    KINFO("  Page Fault (vector 14): IST1");
    KINFO("  NMI (vector 2): IST2");
    KINFO("  Overflow (vector 4): IST5");
    KINFO("  Bound Range (vector 5): IST6");
    KINFO("  Invalid Opcode (vector 6): IST7");
}