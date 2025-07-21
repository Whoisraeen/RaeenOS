#include "memory/include/memory.h"
#include "kernel.h"
#include "hal/hal.h"
#include "core/include/multiboot.h"

// Global memory statistics
static memory_stats_t memory_statistics = {0};
static bool memory_initialized = false;

// Memory region parsing from multiboot
static memory_region_t* parse_multiboot_memory_map(struct multiboot_info* mbi) {
    if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
        KERROR("No memory map provided by bootloader");
        return NULL;
    }
    
    memory_region_t* regions = NULL;
    memory_region_t* last_region = NULL;
    
    struct multiboot_mmap_entry* mmap = (struct multiboot_mmap_entry*)mbi->mmap_addr;
    struct multiboot_mmap_entry* mmap_end = 
        (struct multiboot_mmap_entry*)(mbi->mmap_addr + mbi->mmap_length);
    
    while (mmap < mmap_end) {
        // Create new region
        memory_region_t* region = (memory_region_t*)kernel_heap_alloc(sizeof(memory_region_t), 0);
        if (!region) {
            KERROR("Failed to allocate memory region structure");
            break;
        }
        
        region->start = mmap->addr;
        region->end = mmap->addr + mmap->len;
        region->next = NULL;
        
        // Convert multiboot type to our type
        switch (mmap->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                region->type = MEMORY_TYPE_AVAILABLE;
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                region->type = MEMORY_TYPE_RESERVED;
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                region->type = MEMORY_TYPE_ACPI_RECLAIMABLE;
                break;
            case MULTIBOOT_MEMORY_NVS:
                region->type = MEMORY_TYPE_ACPI_NVS;
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                region->type = MEMORY_TYPE_BAD;
                break;
            default:
                region->type = MEMORY_TYPE_RESERVED;
                break;
        }
        
        // Add to list
        if (last_region) {
            last_region->next = region;
        } else {
            regions = region;
        }
        last_region = region;
        
        KDEBUG("Memory region: 0x%llx-0x%llx (%lluMB) type=%d",
               region->start, region->end,
               (region->end - region->start) / (1024 * 1024),
               region->type);
        
        // Move to next entry
        mmap = (struct multiboot_mmap_entry*)((char*)mmap + mmap->size + sizeof(mmap->size));
    }
    
    return regions;
}

// Add kernel and reserved regions
static void mark_kernel_regions(void) {
    // Mark kernel code/data as used
    extern char _kernel_start[], _kernel_end[];
    phys_addr_t kernel_start = (phys_addr_t)_kernel_start;
    phys_addr_t kernel_end = (phys_addr_t)_kernel_end;
    
    memory_mark_region_used(kernel_start, kernel_end, MEMORY_TYPE_KERNEL);
    
    // Mark low memory as reserved (BIOS, IVT, etc.)
    memory_mark_region_used(0x0, 0x100000, MEMORY_TYPE_RESERVED);
    
    // Mark page tables as used (assuming they're at 0x1000-0x4000 from bootloader)
    memory_mark_region_used(0x1000, 0x4000, MEMORY_TYPE_RESERVED);
    
    KDEBUG("Marked kernel regions as used");
}

error_t memory_init(void) {
    if (memory_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing memory management subsystem");
    
    // First, we need to get memory map from bootloader
    // For now, create a simple default memory map
    // In a real implementation, this would come from multiboot info
    
    memory_region_t* regions = NULL;
    
    // Create a default memory layout for testing
    // This should be replaced with actual multiboot parsing
    memory_region_t* low_mem = (memory_region_t*)kernel_heap_alloc(sizeof(memory_region_t), 0);
    memory_region_t* high_mem = (memory_region_t*)kernel_heap_alloc(sizeof(memory_region_t), 0);
    
    if (!low_mem || !high_mem) {
        KERROR("Failed to allocate memory region structures");
        return E_NOMEM;
    }
    
    // Low memory (0-1MB, mostly reserved)
    low_mem->start = 0x0;
    low_mem->end = 0x100000;  // 1MB
    low_mem->type = MEMORY_TYPE_RESERVED;
    low_mem->next = high_mem;
    
    // High memory (1MB-128MB for testing)
    high_mem->start = 0x100000;   // 1MB
    high_mem->end = 0x8000000;    // 128MB
    high_mem->type = MEMORY_TYPE_AVAILABLE;
    high_mem->next = NULL;
    
    regions = low_mem;
    
    // Initialize physical memory allocator
    error_t result = physical_memory_init(regions);
    if (result != SUCCESS) {
        KERROR("Failed to initialize physical memory allocator: %d", result);
        return result;
    }
    
    // Mark kernel and system regions as used
    mark_kernel_regions();
    
    // Initialize kernel heap
    result = kernel_heap_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize kernel heap: %d", result);
        return result;
    }
    
    // Initialize virtual memory management
    result = virtual_memory_init();
    if (result != SUCCESS) {
        KERROR("Failed to initialize virtual memory: %d", result);
        return result;
    }
    
    // Update statistics
    memory_statistics.total_physical = physical_get_total_memory();
    memory_statistics.available_physical = physical_get_free_memory();
    memory_statistics.used_physical = physical_get_used_memory();
    memory_statistics.total_virtual = USER_SPACE_END - USER_SPACE_START;
    memory_statistics.used_virtual = 0;
    memory_statistics.kernel_heap_used = 0;
    memory_statistics.user_heap_used = 0;
    memory_statistics.page_faults = 0;
    memory_statistics.swapped_pages = 0;
    
    memory_initialized = true;
    
    KINFO("Memory management initialized successfully");
    KINFO("Physical memory: %llu MB total, %llu MB available",
          memory_statistics.total_physical / (1024 * 1024),
          memory_statistics.available_physical / (1024 * 1024));
    
    return SUCCESS;
}

void memory_shutdown(void) {
    if (!memory_initialized) {
        return;
    }
    
    KINFO("Shutting down memory management");
    
    // Cleanup would go here
    // For now, just mark as uninitialized
    memory_initialized = false;
}

memory_stats_t* memory_get_stats(void) {
    if (!memory_initialized) {
        return NULL;
    }
    
    // Update dynamic statistics
    memory_statistics.available_physical = physical_get_free_memory();
    memory_statistics.used_physical = physical_get_used_memory();
    
    return &memory_statistics;
}

// User memory management functions
void* user_alloc(address_space_t* as, size_t size, u32 prot) {
    if (!as || size == 0) {
        return NULL;
    }
    
    size = PAGE_ALIGN_UP(size);
    
    // Find suitable virtual address range
    virt_addr_t start_addr = as->heap_end;
    virt_addr_t end_addr = start_addr + size;
    
    // Check if we have space
    if (end_addr >= as->stack_start) {
        KDEBUG("User heap collision with stack");
        return NULL;
    }
    
    // Create VMA for this allocation
    vma_t* vma = vma_create(start_addr, end_addr, 0, prot);
    if (!vma) {
        return NULL;
    }
    
    error_t result = vma_insert(as, vma);
    if (result != SUCCESS) {
        vma_destroy(vma);
        return NULL;
    }
    
    // Update heap end
    as->heap_end = end_addr;
    
    // Pages will be allocated on-demand through page faults
    
    KDEBUG("Allocated user memory: 0x%llx-%llx (%zu bytes)",
           start_addr, end_addr, size);
    
    return (void*)start_addr;
}

void* user_alloc_at(address_space_t* as, virt_addr_t addr, size_t size, u32 prot) {
    if (!as || size == 0 || addr % PAGE_SIZE != 0) {
        return NULL;
    }
    
    size = PAGE_ALIGN_UP(size);
    virt_addr_t end_addr = addr + size;
    
    // Check if range is valid
    if (addr < USER_SPACE_START || end_addr >= USER_SPACE_END) {
        return NULL;
    }
    
    // Check for overlap with existing VMAs
    if (vma_find(as, addr) || vma_find(as, end_addr - 1)) {
        return NULL;
    }
    
    // Create VMA
    vma_t* vma = vma_create(addr, end_addr, 0, prot);
    if (!vma) {
        return NULL;
    }
    
    error_t result = vma_insert(as, vma);
    if (result != SUCCESS) {
        vma_destroy(vma);
        return NULL;
    }
    
    KDEBUG("Allocated user memory at fixed address: 0x%llx-%llx",
           addr, end_addr);
    
    return (void*)addr;
}

void user_free(address_space_t* as, void* ptr) {
    if (!as || !ptr) {
        return;
    }
    
    virt_addr_t addr = (virt_addr_t)ptr;
    vma_t* vma = vma_find(as, addr);
    
    if (!vma || vma->start != addr) {
        KERROR("Attempt to free invalid user memory: 0x%llx", addr);
        return;
    }
    
    // Unmap all pages in the VMA
    for (virt_addr_t page = vma->start; page < vma->end; page += PAGE_SIZE) {
        phys_addr_t phys = page_table_get_physical(as, page);
        if (phys != 0) {
            page_table_unmap(as, page);
            physical_free_page(phys);
        }
    }
    
    // Remove and destroy VMA
    vma_remove(as, vma);
    vma_destroy(vma);
    
    KDEBUG("Freed user memory: 0x%llx-%llx", vma->start, vma->end);
}

error_t user_protect(address_space_t* as, void* ptr, size_t size, u32 prot) {
    if (!as || !ptr || size == 0) {
        return E_INVAL;
    }
    
    virt_addr_t start_addr = PAGE_ALIGN_DOWN((virt_addr_t)ptr);
    virt_addr_t end_addr = PAGE_ALIGN_UP((virt_addr_t)ptr + size);
    
    // Update page table flags for all pages in range
    for (virt_addr_t page = start_addr; page < end_addr; page += PAGE_SIZE) {
        u32 flags = PAGE_FLAG_PRESENT;
        if (prot & PROT_WRITE) flags |= PAGE_FLAG_WRITABLE;
        if (prot & PROT_USER) flags |= PAGE_FLAG_USER;
        if (!(prot & PROT_EXEC)) flags |= PAGE_FLAG_NO_EXECUTE;
        
        error_t result = page_table_set_flags(as, page, flags);
        if (result != SUCCESS && result != E_MEMORY_NOT_MAPPED) {
            return result;
        }
    }
    
    // Update VMA protection flags
    vma_t* vma = vma_find(as, start_addr);
    if (vma) {
        vma->prot = prot;
    }
    
    return SUCCESS;
}

// Memory copy operations with safety checks
error_t memory_copy_to_user(address_space_t* as, void* user_ptr, const void* kernel_ptr, size_t size) {
    if (!as || !user_ptr || !kernel_ptr || size == 0) {
        return E_INVAL;
    }
    
    virt_addr_t user_addr = (virt_addr_t)user_ptr;
    
    // Verify user address is valid and writable
    vma_t* vma = vma_find(as, user_addr);
    if (!vma || !(vma->prot & PROT_WRITE)) {
        return E_MEMORY_ACCESS_VIOLATION;
    }
    
    // Check bounds
    if (user_addr + size > vma->end) {
        return E_MEMORY_ACCESS_VIOLATION;
    }
    
    // TODO: Add page fault handling during copy
    memcpy(user_ptr, kernel_ptr, size);
    
    return SUCCESS;
}

error_t memory_copy_from_user(address_space_t* as, void* kernel_ptr, const void* user_ptr, size_t size) {
    if (!as || !kernel_ptr || !user_ptr || size == 0) {
        return E_INVAL;
    }
    
    virt_addr_t user_addr = (virt_addr_t)user_ptr;
    
    // Verify user address is valid and readable
    vma_t* vma = vma_find(as, user_addr);
    if (!vma || !(vma->prot & PROT_READ)) {
        return E_MEMORY_ACCESS_VIOLATION;
    }
    
    // Check bounds
    if (user_addr + size > vma->end) {
        return E_MEMORY_ACCESS_VIOLATION;
    }
    
    // TODO: Add page fault handling during copy
    memcpy(kernel_ptr, user_ptr, size);
    
    return SUCCESS;
}

error_t memory_copy_string_from_user(address_space_t* as, char* kernel_str, const char* user_str, size_t max_len) {
    if (!as || !kernel_str || !user_str || max_len == 0) {
        return E_INVAL;
    }
    
    virt_addr_t user_addr = (virt_addr_t)user_str;
    vma_t* vma = vma_find(as, user_addr);
    if (!vma || !(vma->prot & PROT_READ)) {
        return E_MEMORY_ACCESS_VIOLATION;
    }
    
    // Copy string safely
    size_t i;
    for (i = 0; i < max_len - 1; i++) {
        if (user_addr + i >= vma->end) {
            return E_MEMORY_ACCESS_VIOLATION;
        }
        
        kernel_str[i] = user_str[i];
        if (user_str[i] == '\0') {
            break;
        }
    }
    
    kernel_str[i] = '\0';
    return SUCCESS;
}

// Debug and diagnostic functions
void memory_dump_page_tables(address_space_t* as) {
    if (!as || !as->page_directory) {
        hal_console_print("Invalid address space\n");
        return;
    }
    
    hal_console_print("Page table dump for address space %p:\n", as);
    
    page_table_t* pml4 = as->page_directory;
    
    for (u32 pml4_idx = 0; pml4_idx < 512; pml4_idx++) {
        if (!(pml4->entries[pml4_idx] & PAGE_FLAG_PRESENT)) {
            continue;
        }
        
        hal_console_print("PML4[%u]: 0x%llx\n", pml4_idx, pml4->entries[pml4_idx]);
        
        // Could continue dumping PDPT, PD, PT levels but that's a lot of output
        // This is just a basic implementation
    }
}

void memory_dump_vmas(address_space_t* as) {
    if (!as) {
        hal_console_print("Invalid address space\n");
        return;
    }
    
    hal_console_print("VMAs for address space %p:\n", as);
    
    vma_t* vma = as->vma_list;
    u32 count = 0;
    
    while (vma) {
        hal_console_print("  VMA %u: 0x%llx-0x%llx flags=0x%x prot=0x%x\n",
                         count++, vma->start, vma->end, vma->flags, vma->prot);
        vma = vma->next;
    }
    
    if (count == 0) {
        hal_console_print("  No VMAs\n");
    }
}