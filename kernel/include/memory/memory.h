#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// Page flags
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_LARGE      0x080

// Function to initialize memory management
error_t memory_init(void);

// Memory allocation/deallocation functions
void* memory_alloc(size_t size);
void memory_free(void* ptr);

// Functions for managing process address spaces
phys_addr_t memory_create_address_space(void);
void memory_destroy_address_space(phys_addr_t pml4_phys);
error_t memory_map(phys_addr_t pml4_phys, virt_addr_t virt, phys_addr_t phys, size_t size, u64 flags);

#endif // MEMORY_H