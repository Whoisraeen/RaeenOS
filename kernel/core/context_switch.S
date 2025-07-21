.global context_switch
.global context_save
.global context_restore
.section .text

# Complete context switch implementation
# rdi = old_context, rsi = new_context
context_switch:
    # Save current context
    call context_save
    
    # Switch to new context
    mov rdi, rsi
    call context_restore
    
    ret

# Save CPU context to memory
# rdi = context structure pointer
context_save:
    # Save general purpose registers
    mov [rdi + 0x00], rax
    mov [rdi + 0x08], rbx
    mov [rdi + 0x10], rcx
    mov [rdi + 0x18], rdx
    mov [rdi + 0x20], rsi
    mov [rdi + 0x28], rdi
    mov [rdi + 0x30], rbp
    mov [rdi + 0x38], rsp
    mov [rdi + 0x40], r8
    mov [rdi + 0x48], r9
    mov [rdi + 0x50], r10
    mov [rdi + 0x58], r11
    mov [rdi + 0x60], r12
    mov [rdi + 0x68], r13
    mov [rdi + 0x70], r14
    mov [rdi + 0x78], r15
    
    # Save RIP (return address)
    mov rax, [rsp]
    mov [rdi + 0x80], rax
    
    # Save RFLAGS
    pushfq
    pop rax
    mov [rdi + 0x88], rax
    
    # Save segment registers
    mov ax, cs
    mov [rdi + 0x90], ax
    mov ax, ds
    mov [rdi + 0x92], ax
    mov ax, es
    mov [rdi + 0x94], ax
    mov ax, fs
    mov [rdi + 0x96], ax
    mov ax, gs
    mov [rdi + 0x98], ax
    mov ax, ss
    mov [rdi + 0x9A], ax
    
    # Save FPU/SSE state
    fxsave [rdi + 0x100]
    
    ret

# Restore CPU context from memory
# rdi = context structure pointer
context_restore:
    # Restore FPU/SSE state
    fxrstor [rdi + 0x100]
    
    # Restore segment registers
    mov ax, [rdi + 0x92]
    mov ds, ax
    mov ax, [rdi + 0x94]
    mov es, ax
    mov ax, [rdi + 0x96]
    mov fs, ax
    mov ax, [rdi + 0x98]
    mov gs, ax
    
    # Restore RFLAGS
    mov rax, [rdi + 0x88]
    push rax
    popfq
    
    # Restore stack pointer
    mov rsp, [rdi + 0x38]
    
    # Restore general purpose registers (except RSP which is already restored)
    mov rax, [rdi + 0x00]
    mov rbx, [rdi + 0x08]
    mov rcx, [rdi + 0x10]
    mov rdx, [rdi + 0x18]
    mov rsi, [rdi + 0x20]
    mov rbp, [rdi + 0x30]
    mov r8,  [rdi + 0x40]
    mov r9,  [rdi + 0x48]
    mov r10, [rdi + 0x50]
    mov r11, [rdi + 0x58]
    mov r12, [rdi + 0x60]
    mov r13, [rdi + 0x68]
    mov r14, [rdi + 0x70]
    mov r15, [rdi + 0x78]
    
    # Restore RDI last and jump to saved RIP
    push qword ptr [rdi + 0x80]  # Push return address
    mov rdi, [rdi + 0x28]        # Restore RDI
    ret                          # Jump to saved RIP