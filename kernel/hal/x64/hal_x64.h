#ifndef HAL_X64_H
#define HAL_X64_H

#include "types.h"

// x64-specific register structures
typedef struct {
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11;
    u64 r12, r13, r14, r15;
    u64 rip, rflags;
    u16 cs, ds, es, fs, gs, ss;
} x64_registers_t;

// Control register structures
typedef struct {
    u64 cr0, cr2, cr3, cr4;
    u64 cr8;  // Task Priority Register (TPR) in x64
} x64_control_regs_t;

// MSR (Model Specific Register) definitions
#define MSR_EFER           0xC0000080
#define MSR_STAR           0xC0000081
#define MSR_LSTAR          0xC0000082
#define MSR_CSTAR          0xC0000083
#define MSR_SYSCALL_MASK   0xC0000084
#define MSR_FS_BASE        0xC0000100
#define MSR_GS_BASE        0xC0000101
#define MSR_KERNEL_GS_BASE 0xC0000102

// EFER register bits
#define EFER_SCE   (1 << 0)   // System Call Extensions
#define EFER_LME   (1 << 8)   // Long Mode Enable
#define EFER_LMA   (1 << 10)  // Long Mode Active
#define EFER_NXE   (1 << 11)  // No-Execute Enable

// CR0 register bits
#define CR0_PE     (1 << 0)   // Protection Enable
#define CR0_MP     (1 << 1)   // Monitor Coprocessor
#define CR0_EM     (1 << 2)   // Emulation
#define CR0_TS     (1 << 3)   // Task Switched
#define CR0_ET     (1 << 4)   // Extension Type
#define CR0_NE     (1 << 5)   // Numeric Error
#define CR0_WP     (1 << 16)  // Write Protect
#define CR0_AM     (1 << 18)  // Alignment Mask
#define CR0_NW     (1 << 29)  // Not Write-through
#define CR0_CD     (1 << 30)  // Cache Disable
#define CR0_PG     (1 << 31)  // Paging

// CR4 register bits
#define CR4_VME    (1 << 0)   // Virtual-8086 Mode Extensions
#define CR4_PVI    (1 << 1)   // Protected-Mode Virtual Interrupts
#define CR4_TSD    (1 << 2)   // Time Stamp Disable
#define CR4_DE     (1 << 3)   // Debugging Extensions
#define CR4_PSE    (1 << 4)   // Page Size Extensions
#define CR4_PAE    (1 << 5)   // Physical Address Extension
#define CR4_MCE    (1 << 6)   // Machine-Check Enable
#define CR4_PGE    (1 << 7)   // Page Global Enable
#define CR4_PCE    (1 << 8)   // Performance Counter Enable
#define CR4_OSFXSR (1 << 9)   // OS Support for FXSAVE/FXRSTOR
#define CR4_OSXMMEXCPT (1 << 10) // OS Support for Unmasked SIMD Exceptions
#define CR4_VMXE   (1 << 13)  // VMX Enable
#define CR4_SMXE   (1 << 14)  // SMX Enable
#define CR4_FSGSBASE (1 << 16) // FSGSBASE Enable
#define CR4_PCIDE  (1 << 17)  // PCID Enable
#define CR4_OSXSAVE (1 << 18) // XSAVE and Processor Extended States Enable
#define CR4_SMEP   (1 << 20)  // Supervisor Mode Execution Prevention
#define CR4_SMAP   (1 << 21)  // Supervisor Mode Access Prevention

// RFLAGS register bits
#define RFLAGS_CF  (1 << 0)   // Carry Flag
#define RFLAGS_PF  (1 << 2)   // Parity Flag
#define RFLAGS_AF  (1 << 4)   // Auxiliary Carry Flag
#define RFLAGS_ZF  (1 << 6)   // Zero Flag
#define RFLAGS_SF  (1 << 7)   // Sign Flag
#define RFLAGS_TF  (1 << 8)   // Trap Flag
#define RFLAGS_IF  (1 << 9)   // Interrupt Enable Flag
#define RFLAGS_DF  (1 << 10)  // Direction Flag
#define RFLAGS_OF  (1 << 11)  // Overflow Flag
#define RFLAGS_IOPL (3 << 12) // I/O Privilege Level
#define RFLAGS_NT  (1 << 14)  // Nested Task
#define RFLAGS_RF  (1 << 16)  // Resume Flag
#define RFLAGS_VM  (1 << 17)  // Virtual-8086 Mode
#define RFLAGS_AC  (1 << 18)  // Alignment Check
#define RFLAGS_VIF (1 << 19)  // Virtual Interrupt Flag
#define RFLAGS_VIP (1 << 20)  // Virtual Interrupt Pending
#define RFLAGS_ID  (1 << 21)  // ID Flag

// Page table entry structure for x64
typedef struct {
    u64 present     : 1;
    u64 writable    : 1;
    u64 user        : 1;
    u64 writethrough: 1;
    u64 cache_disable: 1;
    u64 accessed    : 1;
    u64 dirty       : 1;
    u64 pat         : 1;
    u64 global      : 1;
    u64 available   : 3;
    u64 address     : 40;
    u64 available2  : 11;
    u64 no_execute  : 1;
} __attribute__((packed)) x64_pte_t;

// GDT entry structure
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

// GDT pointer structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) gdt_ptr_t;

// IDT entry structure
typedef struct {
    u16 offset_low;
    u16 selector;
    u8  ist;
    u8  type_attr;
    u16 offset_middle;
    u32 offset_high;
    u32 zero;
} __attribute__((packed)) idt_entry_t;

// IDT pointer structure
typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) idt_ptr_t;

// Function declarations
void x64_enable_interrupts(void);
void x64_disable_interrupts(void);
bool x64_are_interrupts_enabled(void);
void x64_halt_cpu(void);
void x64_pause_cpu(void);

u64 x64_read_cr0(void);
u64 x64_read_cr2(void);
u64 x64_read_cr3(void);
u64 x64_read_cr4(void);
u64 x64_read_cr8(void);

void x64_write_cr0(u64 value);
void x64_write_cr3(u64 value);
void x64_write_cr4(u64 value);
void x64_write_cr8(u64 value);

u64 x64_read_msr(u32 msr);
void x64_write_msr(u32 msr, u64 value);

u64 x64_read_rflags(void);
void x64_write_rflags(u64 value);

void x64_invlpg(virt_addr_t addr);
void x64_flush_tlb(void);

void x64_cpuid(u32 leaf, u32* eax, u32* ebx, u32* ecx, u32* edx);

// Memory barriers
void x64_memory_barrier(void);
void x64_read_barrier(void);
void x64_write_barrier(void);

// Cache operations
void x64_wbinvd(void);
void x64_invd(void);
void x64_clflush(void* addr);

// Time operations
u64 x64_rdtsc(void);
u64 x64_rdtscp(u32* aux);

// Segment operations
void x64_load_gdt(gdt_ptr_t* gdt_ptr);
void x64_load_idt(idt_ptr_t* idt_ptr);

// I/O operations are inherited from the generic HAL

#endif // HAL_X64_H