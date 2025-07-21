#ifndef KHEAP_H
#define KHEAP_H

#include "types.h"

// Initialize the kernel heap.
void kheap_init(virt_addr_t start, size_t size);

// Allocate memory from the kernel heap.
void* kmalloc(size_t size);

// Free memory on the kernel heap.
void kfree(void* ptr);

#endif // KHEAP_H