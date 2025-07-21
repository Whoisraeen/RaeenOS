.section .text
.global syscall_entry

syscall_entry:
    // The 'syscall' instruction transfers control here from user space.
    // It saves user RIP in RCX and RFLAGS in R11.
    // Other registers (RAX, RDI, RSI, etc.) hold syscall number and arguments.

    // Swap to the kernel stack. We assume a per-CPU structure holds this.
    // For now, we'll just use the current stack pointer, assuming we are on
    // a valid kernel stack after the interrupt gate transition.
    // A real implementation would use 'swapgs' and load RSP from a TSS or MSR.

    // Push all registers to create the syscall_frame_t on the stack.
    // The order must match the struct in syscall.h
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx // User RIP
    push rbx
    push rax // Syscall number

    // The C dispatcher function expects a pointer to the frame in RDI
    mov rdi, rsp
    
    // Call the C-level dispatcher
    call syscall_dispatcher

    // The return value from the dispatcher is in RAX.
    // It needs to be placed into the saved RAX on the stack.
    mov [rsp + 0x30], rax // Offset of rax in the frame

    // Restore all registers
    pop rax
    pop rbx
    pop rcx // User RIP will be handled by sysretq
    pop rdx
    pop rsi
    pop rdi
    pop rbp

    // Return to user space.
    // 'sysretq' restores RIP from RCX and RFLAGS from R11.
    sysretq