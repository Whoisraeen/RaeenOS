.global context_switch
.section .text

context_switch:
    // rdi = old_context, rsi = new_context

    // Save old context
    pushfq
    pop rax
    mov [rdi + 0x40], rax  // rflags
    mov [rdi + 0x48], rsp  // rsp 
    mov [rdi + 0x50], ss   // ss
    // RIP is not saved here; it's saved on the stack by the 'call' to schedule
    
    // Save all general-purpose registers as defined in process_context_t
    mov [rdi], r15
    mov [rdi + 0x08], r14
    mov [rdi + 0x10], r13
    mov [rdi + 0x18], r12
    mov [rdi + 0x20], r11
    mov [rdi + 0x28], r10
    mov [rdi + 0x30], r9
    mov [rdi + 0x38], r8
    mov [rdi + 0x58], rbp // Original RDI value is now in the struct
    mov [rdi + 0x60], rsi
    mov [rdi + 0x68], rdx
    mov [rdi + 0x70], rcx
    mov [rdi + 0x78], rbx
    mov [rdi + 0x80], rax // RAX MUST be saved as part of the old context

    // Load new context
    mov rsp, [rsi + 0x48] // rsp
    mov rax, [rsi + 0x40] // rflags
    push rax
    popfq

    // Restore all general-purpose registers
    mov r15, [rsi]
    mov r14, [rsi + 0x08]
    mov r13, [rsi + 0x10]
    mov r12, [rsi + 0x18]
    mov r11, [rsi + 0x20]
    mov r10, [rsi + 0x28]
    mov r9,  [rsi + 0x30]
    mov r8,  [rsi + 0x38]
    mov rbp, [rsi + 0x58] // ... and so on for rsi, rdx, rcx, rbx
    mov rax, [rsi + 0x80]

    // iretq will load cs, rip, rsp, ss, rflags
    // The 'ret' instruction will pop the new RIP from the stack
    ret