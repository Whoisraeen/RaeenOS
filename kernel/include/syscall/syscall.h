#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

// This structure defines the register state passed from the assembly handler
// to the C dispatcher. The order MUST match the push/pop order in syscall_entry.S
typedef struct {
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax; // Must match push order
    // The following are passed by the syscall instruction itself
    // u64 rip (in rcx), rflags (in r11)
    // u64 rsp (on stack switch)
} __attribute__((packed)) syscall_frame_t;

// Initializes the syscall subsystem.
void syscall_init(void);

// Architecture-specific syscall setup (called by syscall_init).
void x64_syscall_init(void);

// The main C entry point for all system calls.
u64 syscall_dispatcher(syscall_frame_t* frame);

#endif // SYSCALL_H