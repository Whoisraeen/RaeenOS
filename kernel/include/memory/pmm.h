#ifndef PMM_H
#define PMM_H

#include "types.h"
#include "multiboot.h"

#define PAGE_SIZE 4096

// Initializes the physical memory manager.
// Must be called once at startup, passing the multiboot info.
error_t pmm_init(struct multiboot_info* mbi);

// Allocates a single physical page.
phys_addr_t pmm_alloc_page(void);

// Frees a single physical page.
void pmm_free_page(phys_addr_t page);

#endif // PMM_H