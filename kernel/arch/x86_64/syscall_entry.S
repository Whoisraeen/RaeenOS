.section .text

.global syscall_entry
.extern syscall_dispatcher

// Syscall entry point
// This is called when user code executes SYSCALL instruction
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
    
    // Switch to kernel stack if needed
    movq %rsp, %rax
    andq $0xFFFF000000000000, %rax
    jz 1f
    // We're on user stack, switch to kernel stack
    movq %gs:0x0, %rsp  // Load kernel stack from TSS
1:
    
    // Set up C calling convention
    // rdi = syscall number
    // rsi = arg1
    // rdx = arg2
    // rcx = arg3
    // r8 = arg4
    // r9 = arg5
    // [rsp+8] = arg6 (pushed by SYSCALL)
    
    // Move syscall number to rdi
    movq %rax, %rdi
    
    // Move arguments to correct registers
    movq %rcx, %rdx  // arg2
    movq %r10, %rcx  // arg3
    movq %r8, %r8    // arg4
    movq %r9, %r9    // arg5
    
    // Get arg6 from stack
    movq 8(%rsp), %rax
    pushq %rax       // Push arg6 for C function
    
    // Call C dispatcher
    call syscall_dispatcher
    
    // Clean up stack
    addq $8, %rsp
    
    // Restore registers
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

// MSR setup functions
.global syscall_setup_msr
syscall_setup_msr:
    // Set up STAR MSR (syscall target address)
    movq $syscall_entry, %rax
    movq $0, %rdx
    movq $0xC0000081, %rcx  // STAR MSR
    wrmsr
    
    // Set up LSTAR MSR (long mode syscall target)
    movq $syscall_entry, %rax
    movq $0, %rdx
    movq $0xC0000082, %rcx  // LSTAR MSR
    wrmsr
    
    // Set up SFMASK MSR (syscall flag mask)
    movq $0x00000300, %rax  // Clear TF and IF flags
    movq $0, %rdx
    movq $0xC0000084, %rcx  // SFMASK MSR
    wrmsr
    
    ret

.global syscall_enable
syscall_enable:
    // Enable syscall instruction
    movq %cr4, %rax
    orq $0x1000, %rax  // Set OSFXSR bit
    movq %rax, %cr4
    
    // Enable syscall in EFER
    movq $0xC0000080, %rcx  // EFER MSR
    rdmsr
    orq $0x1, %rax  // Set SCE bit
    wrmsr
    
    ret 