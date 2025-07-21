#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include <stddef.h>

// Memory management constants
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PAGE_MASK 0xFFF

// Virtual memory layout
#define KERNEL_BASE 0xFFFFFFFF80000000
#define KERNEL_STACK_BASE 0xFFFFFFFF90000000
#define USER_BASE 0x0000000000000000
#define USER_STACK_BASE 0x00007FFFFFFFFFFF

// Memory protection flags
#define MEMORY_READ 0x01
#define MEMORY_WRITE 0x02
#define MEMORY_EXECUTE 0x04
#define MEMORY_USER 0x08
#define MEMORY_GLOBAL 0x10
#define MEMORY_NOCACHE 0x20

// Page table entry flags
#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_WRITETHROUGH (1ULL << 3)
#define PAGE_NOCACHE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NOEXECUTE (1ULL << 63)

// Memory allocation types
typedef enum {
    ALLOC_KERNEL,
    ALLOC_USER,
    ALLOC_DEVICE,
    ALLOC_DMA
} allocation_type_t;

// Memory region structure
typedef struct memory_region {
    phys_addr_t start;
    phys_addr_t end;
    u32 type;
    u32 flags;
} memory_region_t;

// Page frame structure
typedef struct page_frame {
    u32 ref_count;
    u32 flags;
    struct page_frame* next;
    struct page_frame* prev;
} page_frame_t;

// Virtual memory area structure
typedef struct vm_area {
    virt_addr_t start;
    virt_addr_t end;
    u32 flags;
    u32 prot;
    struct vm_area* next;
    struct vm_area* prev;
} vm_area_t;

// Memory mapping structure
typedef struct memory_mapping {
    virt_addr_t virtual_addr;
    phys_addr_t physical_addr;
    size_t size;
    u32 flags;
    u32 prot;
} memory_mapping_t;

// Memory statistics
typedef struct memory_stats {
    u64 total_physical;
    u64 available_physical;
    u64 used_physical;
    u64 total_virtual;
    u64 used_virtual;
    u32 page_faults;
    u32 page_swaps;
} memory_stats_t;

// Memory management functions
error_t memory_init(void);
void memory_shutdown(void);

// Physical memory management
error_t memory_alloc_physical(size_t size, phys_addr_t* addr);
error_t memory_free_physical(phys_addr_t addr, size_t size);
error_t memory_reserve_physical(phys_addr_t start, phys_addr_t end);
error_t memory_get_physical_stats(memory_stats_t* stats);

// Virtual memory management
error_t memory_map_virtual(virt_addr_t virt_addr, phys_addr_t phys_addr, 
                          size_t size, u32 flags, u32 prot);
error_t memory_unmap_virtual(virt_addr_t virt_addr, size_t size);
error_t memory_protect_virtual(virt_addr_t virt_addr, size_t size, u32 prot);
error_t memory_alloc_virtual(size_t size, virt_addr_t* addr, u32 flags);
error_t memory_free_virtual(virt_addr_t addr, size_t size);

// Page table management
error_t memory_create_page_tables(void);
error_t memory_switch_page_tables(phys_addr_t page_table);
error_t memory_invalidate_page(virt_addr_t addr);
error_t memory_invalidate_all_pages(void);

// Memory allocation (malloc/free equivalents)
void* memory_alloc(size_t size);
void* memory_alloc_aligned(size_t size, size_t alignment);
void memory_free(void* ptr);
void* memory_realloc(void* ptr, size_t size);

// Memory copying and setting
void memory_copy(void* dest, const void* src, size_t size);
void memory_set(void* dest, u8 value, size_t size);
void memory_zero(void* dest, size_t size);

// Memory mapping for devices
error_t memory_map_device(phys_addr_t device_addr, size_t size, 
                         virt_addr_t* virt_addr);
error_t memory_unmap_device(virt_addr_t virt_addr, size_t size);

// Memory region management
error_t memory_add_region(phys_addr_t start, phys_addr_t end, u32 type, u32 flags);
error_t memory_get_regions(memory_region_t* regions, u32* count);
error_t memory_find_region(phys_addr_t addr, memory_region_t* region);

// Memory debugging
void memory_dump_page_tables(void);
void memory_dump_mappings(void);
void memory_validate_integrity(void);

// Architecture-specific functions
#ifdef __x86_64__
error_t memory_init_x64(void);
void memory_invalidate_tlb(void);
void memory_invalidate_tlb_entry(virt_addr_t addr);
#else
error_t memory_init_x86(void);
void memory_invalidate_tlb(void);
void memory_invalidate_tlb_entry(virt_addr_t addr);
#endif

// Memory allocation macros
#define MEMORY_ALLOC(size) memory_alloc(size)
#define MEMORY_FREE(ptr) memory_free(ptr)
#define MEMORY_ALLOC_ALIGNED(size, align) memory_alloc_aligned(size, align)

// Page alignment macros
#define PAGE_ALIGN_UP(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#define IS_PAGE_ALIGNED(addr) (((addr) & (PAGE_SIZE - 1)) == 0)

// Memory protection macros
#define PROT_NONE 0
#define PROT_READ MEMORY_READ
#define PROT_WRITE MEMORY_WRITE
#define PROT_EXEC MEMORY_EXECUTE
#define PROT_USER MEMORY_USER

// Memory mapping flags
#define MAP_PRIVATE 0x01
#define MAP_SHARED 0x02
#define MAP_FIXED 0x04
#define MAP_ANONYMOUS 0x08

#endif // MEMORY_H 