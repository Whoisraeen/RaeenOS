.section .text

// User mode entry point
// void user_mode_entry(user_program_t* program)
.global user_mode_entry
user_mode_entry:
    // Save kernel stack pointer
    movq %rsp, %rax
    movq %rax, kernel_stack_save
    
    // Get program context from rdi (first parameter)
    movq %rdi, %rax
    
    // Set up user stack pointer
    movq 8(%rax), %rsp  // program->stack_pointer
    
    // Set up segment registers for user mode
    movq $0x23, %rax    // User data segment (RPL=3, TI=0, Index=4)
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %gs
    movq %rax, %ss
    
    // Set up registers for user mode
    xorq %rax, %rax     // Clear rax
    xorq %rbx, %rbx     // Clear rbx
    xorq %rcx, %rcx     // Clear rcx
    xorq %rdx, %rdx     // Clear rdx
    xorq %rsi, %rsi     // Clear rsi
    xorq %rdi, %rdi     // Clear rdi
    xorq %rbp, %rbp     // Clear rbp
    xorq %r8, %r8       // Clear r8
    xorq %r9, %r9       // Clear r9
    xorq %r10, %r10     // Clear r10
    xorq %r11, %r11     // Clear r11
    xorq %r12, %r12     // Clear r12
    xorq %r13, %r13     // Clear r13
    xorq %r14, %r14     // Clear r14
    xorq %r15, %r15     // Clear r15
    
    // Get entry point from program context
    movq (%rdi), %rax   // program->entry_point
    
    // Set up stack frame for iretq
    pushq $0x23         // User SS (RPL=3, TI=0, Index=4)
    pushq %rsp          // User RSP
    pushq $0x202        // User RFLAGS (IF=1, IOPL=0, reserved=1)
    pushq $0x1B         // User CS (RPL=3, TI=0, Index=3)
    pushq %rax          // User RIP (entry point)
    
    // Switch to user mode
    iretq

// Return to kernel mode
// void user_mode_exit(void)
.global user_mode_exit
user_mode_exit:
    // Switch back to kernel segments
    movq $0x10, %rax    // Kernel data segment
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %gs
    movq %rax, %ss
    
    // Restore kernel stack
    movq kernel_stack_save, %rsp
    
    // Return to kernel
    ret

// System call entry point
// void syscall_entry(void)
.global syscall_entry
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
    
    // Switch to kernel segments
    movq $0x10, %rax    // Kernel data segment
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %gs
    
    // Call system call handler
    // rdi = syscall number
    // rsi = arg1
    // rdx = arg2
    // r10 = arg3
    // r8 = arg4
    // r9 = arg5
    call syscall_handler
    
    // Restore user registers
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

// Page fault handler for user mode
// void user_page_fault_handler(void)
.global user_page_fault_handler
user_page_fault_handler:
    // Check if fault occurred in user mode
    testq $0x4, 8(%rsp)  // Check CS in interrupt frame
    jz kernel_page_fault // If kernel mode, handle normally
    
    // User mode page fault - handle demand paging
    call handle_user_page_fault
    
    // Return to user mode
    iretq

kernel_page_fault:
    // Handle kernel page fault normally
    call exception_page_fault
    // Should not return

// Handle user page fault (demand paging)
// void handle_user_page_fault(void)
.global handle_user_page_fault
handle_user_page_fault:
    // Get faulting address from CR2
    movq %cr2, %rdi
    
    // Call demand paging handler
    call memory_handle_page_fault
    
    ret

// Data section
.section .data
kernel_stack_save:
    .quad 0 