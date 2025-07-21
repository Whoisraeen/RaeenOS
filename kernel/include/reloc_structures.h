#ifndef RELOC_STRUCTURES_H
#define RELOC_STRUCTURES_H

#include "types.h"

// Relocation block structure
typedef struct {
    u32 virtual_address; // RVA of the block
    u32 block_size;      // Total size of the block, including header
} relocation_block_t;

#endif