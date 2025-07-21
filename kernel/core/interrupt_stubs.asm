; RaeenOS Interrupt Stubs
; x64 Assembly interrupt entry points

[BITS 64]

section .text

; External function
extern interrupt_dispatch

; Common interrupt handler macro
%macro INTERRUPT_STUB 1
global interrupt_stub_%1
interrupt_stub_%1:
    ; Push dummy error code if not provided by CPU
    %if %1 != 8 && %1 != 10 && %1 != 11 && %1 != 12 && %1 != 13 && %1 != 14 && %1 != 17 && %1 != 21
        push 0  ; Dummy error code
    %endif
    
    ; Push interrupt number
    push %1
    
    ; Jump to common handler
    jmp interrupt_common_handler
%endmacro

; Exception stubs (0-31)
INTERRUPT_STUB 0   ; Divide by zero
INTERRUPT_STUB 1   ; Debug
INTERRUPT_STUB 2   ; NMI
INTERRUPT_STUB 3   ; Breakpoint
INTERRUPT_STUB 4   ; Overflow
INTERRUPT_STUB 5   ; Bound range exceeded
INTERRUPT_STUB 6   ; Invalid opcode
INTERRUPT_STUB 7   ; Device not available
INTERRUPT_STUB 8   ; Double fault (has error code)
INTERRUPT_STUB 9   ; Coprocessor segment overrun (reserved)
INTERRUPT_STUB 10  ; Invalid TSS (has error code)
INTERRUPT_STUB 11  ; Segment not present (has error code)
INTERRUPT_STUB 12  ; Stack segment fault (has error code)
INTERRUPT_STUB 13  ; General protection (has error code)
INTERRUPT_STUB 14  ; Page fault (has error code)
INTERRUPT_STUB 15  ; Reserved
INTERRUPT_STUB 16  ; x87 FPU error
INTERRUPT_STUB 17  ; Alignment check (has error code)
INTERRUPT_STUB 18  ; Machine check
INTERRUPT_STUB 19  ; SIMD FPU error
INTERRUPT_STUB 20  ; Virtualization exception
INTERRUPT_STUB 21  ; Control protection exception (has error code)

; Reserved exceptions (22-31)
%assign i 22
%rep 10
    INTERRUPT_STUB i
    %assign i i+1
%endrep

; Hardware interrupts (32-47) - IRQ 0-15
%assign i 32
%rep 16
    INTERRUPT_STUB i
    %assign i i+1
%endrep

; Software interrupts (48-127)
%assign i 48
%rep 80
    INTERRUPT_STUB i
    %assign i i+1
%endrep

; System call interrupt (128)
INTERRUPT_STUB 128

; Additional interrupts (129-254)
%assign i 129
%rep 126
    INTERRUPT_STUB i
    %assign i i+1
%endrep

; Spurious interrupt (255)
INTERRUPT_STUB 255

; Common interrupt handler
global interrupt_common_handler
interrupt_common_handler:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Save segment registers
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax
    
    ; Load kernel data segment
    mov ax, 0x10    ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Create interrupt context structure on stack
    ; Stack layout (from top to bottom):
    ; - SS (pushed by CPU)
    ; - RSP (pushed by CPU)
    ; - RFLAGS (pushed by CPU)
    ; - CS (pushed by CPU)
    ; - RIP (pushed by CPU)
    ; - Error code (pushed by stub or CPU)
    ; - Interrupt number (pushed by stub)
    ; - Segment registers (gs, fs, es, ds)
    ; - General purpose registers (r15-rax)
    
    ; RSP now points to the interrupt context
    mov rdi, rsp    ; Pass pointer to context as first argument
    
    ; Align stack to 16 bytes (required by System V ABI)
    and rsp, ~0xF
    
    ; Call the C interrupt dispatcher
    call interrupt_dispatch
    
    ; Restore RSP
    mov rsp, rdi
    
    ; Restore segment registers
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax
    
    ; Restore general purpose registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; Remove interrupt number and error code
    add rsp, 16
    
    ; Return from interrupt
    iretq

; Helper functions for enabling/disabling interrupts
global asm_enable_interrupts
asm_enable_interrupts:
    sti
    ret

global asm_disable_interrupts
asm_disable_interrupts:
    cli
    ret

global asm_halt
asm_halt:
    hlt
    ret

; Load IDT
global asm_load_idt
asm_load_idt:
    lidt [rdi]
    ret

; Get/Set control registers
global asm_get_cr0
asm_get_cr0:
    mov rax, cr0
    ret

global asm_get_cr2
asm_get_cr2:
    mov rax, cr2
    ret

global asm_get_cr3
asm_get_cr3:
    mov rax, cr3
    ret

global asm_set_cr3
asm_set_cr3:
    mov cr3, rdi
    ret

; Read RFLAGS
global asm_get_rflags
asm_get_rflags:
    pushfq
    pop rax
    ret