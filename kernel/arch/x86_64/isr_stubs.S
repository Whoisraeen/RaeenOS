.section .text

// Extern C handler
extern interrupt_handler

// Macro to create an ISR stub that does NOT push an error code
.macro ISR_NO_ERR_CODE vector
.global isr\vector
isr\vector:
    cli             // Disable interrupts
    pushq $0        // Push a dummy error code
    pushq $\vector  // Push the interrupt number
    jmp isr_common_stub
.endm

// Macro to create an ISR stub that DOES have an error code
.macro ISR_ERR_CODE vector
.global isr\vector
isr\vector:
    cli             // Disable interrupts
    // Error code is already on the stack
    pushq $\vector  // Push the interrupt number
    jmp isr_common_stub
.endm

// Create stubs for all 32 exceptions
ISR_NO_ERR_CODE 0
ISR_NO_ERR_CODE 1
ISR_NO_ERR_CODE 2
ISR_NO_ERR_CODE 3
ISR_NO_ERR_CODE 4
ISR_NO_ERR_CODE 5
ISR_NO_ERR_CODE 6
ISR_NO_ERR_CODE 7
ISR_ERR_CODE    8
ISR_NO_ERR_CODE 9
ISR_ERR_CODE    10
ISR_ERR_CODE    11
ISR_ERR_CODE    12
ISR_ERR_CODE    13
ISR_ERR_CODE    14
ISR_NO_ERR_CODE 15
ISR_NO_ERR_CODE 16
ISR_NO_ERR_CODE 17
ISR_NO_ERR_CODE 18
ISR_NO_ERR_CODE 19
ISR_NO_ERR_CODE 20
ISR_NO_ERR_CODE 21
ISR_NO_ERR_CODE 22
ISR_NO_ERR_CODE 23
ISR_NO_ERR_CODE 24
ISR_NO_ERR_CODE 25
ISR_NO_ERR_CODE 26
ISR_NO_ERR_CODE 27
ISR_NO_ERR_CODE 28
ISR_NO_ERR_CODE 29
ISR_NO_ERR_CODE 30
ISR_NO_ERR_CODE 31

// Timer interrupt (IRQ0, vector 32)
.global timer_interrupt_handler_wrapper
timer_interrupt_handler_wrapper:
    cli
    pushq $0        // Dummy error code
    pushq $32       // Interrupt number
    jmp isr_common_stub

isr_common_stub:
    // Save all general purpose registers
    pushq %r15; pushq %r14; pushq %r13; pushq %r12; pushq %r11; pushq %r10; pushq %r9; pushq %r8
    pushq %rbp; pushq %rdi; pushq %rsi; pushq %rdx; pushq %rcx; pushq %rbx; pushq %rax

    movq %rsp, %rdi // Pass stack pointer (interrupt_frame) to C handler
    call interrupt_handler

    // ... restore registers and return ...
    // The C handler should never return for a fatal exception. If it does:
    popq %rax; popq %rbx; popq %rcx; popq %rdx; popq %rsi; popq %rdi; popq %rbp
    popq %r8; popq %r9; popq %r10; popq %r11; popq %r12; popq %r13; popq %r14; popq %r15
    
    addq $16, %rsp  // Pop interrupt number and error code
    iretq