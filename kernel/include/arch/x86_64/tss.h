#ifndef TSS_H
#define TSS_H

#include "types.h"

#define IST_ENTRIES 7

// Task State Segment structure for x86-64
typedef struct {
    u32 reserved0;
    u64 rsp0;         // Stack pointer for ring 0
    u64 rsp1;         // Stack pointer for ring 1
    u64 rsp2;         // Stack pointer for ring 2
    u64 reserved1;
    u64 ist[IST_ENTRIES]; // Interrupt Stack Table pointers
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;  // I/O Map Base Address
} __attribute__((packed)) tss_t;

#endif // TSS_H