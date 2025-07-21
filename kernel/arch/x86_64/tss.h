#ifndef TSS_H
#define TSS_H

#include "types.h"

// TSS structure for x86_64 (simplified version)
typedef struct {
    u32 reserved0;
    u64 rsp0;      // Ring 0 stack pointer
    u64 rsp1;      // Ring 1 stack pointer  
    u64 rsp2;      // Ring 2 stack pointer
    u64 reserved1;
    u64 ist1;      // Interrupt Stack Table entry 1
    u64 ist2;      // Interrupt Stack Table entry 2
    u64 ist3;      // Interrupt Stack Table entry 3
    u64 ist4;      // Interrupt Stack Table entry 4
    u64 ist5;      // Interrupt Stack Table entry 5
    u64 ist6;      // Interrupt Stack Table entry 6
    u64 ist7;      // Interrupt Stack Table entry 7
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;
} __attribute__((packed)) tss_t;

// IST stack sizes
#define IST_STACK_SIZE 4096  // 4KB per IST stack
#define NUM_IST_STACKS 7     // IST1-IST7

// IST stack purposes
#define IST_DOUBLE_FAULT     1  // IST1: Double fault handler
#define IST_NMI              2  // IST2: NMI handler
#define IST_MACHINE_CHECK    3  // IST3: Machine check handler
#define IST_DEBUG            4  // IST4: Debug exception
#define IST_OVERFLOW         5  // IST5: Overflow exception
#define IST_BOUND_RANGE      6  // IST6: Bound range exceeded
#define IST_INVALID_OPCODE   7  // IST7: Invalid opcode

// TSS functions
error_t tss_init(void);
void tss_set_rsp0(u64 rsp);
void tss_set_ist(u8 ist_index, u64 rsp);
u64 tss_get_ist(u8 ist_index);
void tss_load(void);

// IST management
error_t ist_init_stacks(void);
void ist_set_stack(u8 ist_index, u64 stack_ptr);
u64 ist_get_stack(u8 ist_index);
void ist_allocate_stack(u8 ist_index);
void ist_free_stack(u8 ist_index);

// Stack management utilities
u64 tss_allocate_stack(size_t size);
void tss_free_stack(u64 stack_ptr);
bool tss_validate_stack(u64 stack_ptr);

// TSS switching
void tss_switch_to_kernel_stack(void);
void tss_switch_to_user_stack(u64 user_rsp);
void tss_switch_to_interrupt_stack(u8 ist_index);

// Debug and monitoring
void tss_dump_info(void);
void tss_validate_integrity(void);
bool tss_is_initialized(void);

// Error handling
const char* tss_get_error_string(error_t error);
void tss_handle_stack_corruption(u64 corrupted_stack);

#endif // TSS_H 