.global syscall_entry
.global syscall_return

// Syscall entry point
// This is called when a user program executes the syscall instruction
syscall_entry:
    // Save user registers
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    
    // Switch to kernel stack
    movq %rsp, %gs:0x0  // Save user stack pointer
    movq %gs:0x8, %rsp  // Load kernel stack pointer
    
    // Set up kernel segments
    movq $0x10, %rax    // Kernel data segment
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %gs
    
    // Call the syscall handler
    // Arguments: syscall number in %rax, args in %rdi, %rsi, %rdx, %r10, %r8, %r9
    movq %rax, %rdi     // syscall number
    movq %r10, %rcx     // 4th argument
    call syscall_handler
    
    // Restore user registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rax
    
    // Return to user mode
    sysretq

// Syscall return (for when we want to return to user mode from kernel)
syscall_return:
    // Set up user segments
    movq $0x23, %rax    // User data segment
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %gs
    
    // Restore user stack
    movq %gs:0x0, %rsp  // Restore user stack pointer
    
    // Return to user code
    retq 