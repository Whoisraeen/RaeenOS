#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "hal/hal.h"
#include "core/bootloader_handoff.h"
#include <stddef.h>
#include <stdbool.h>

// Memory constants
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGES_PER_TABLE     512
#define KERNEL_HEAP_START   0xFFFF800000000000ULL
#define KERNEL_HEAP_SIZE    (64 * 1024 * 1024)  // 64MB
#define USER_SPACE_START    0x0000000000400000ULL  // 4MB
#define USER_SPACE_END      0x0000800000000000ULL
#define KERNEL_SPACE_START  0xFFFF800000000000ULL

// Page flags
#define PAGE_FLAG_PRESENT     (1 << 0)
#define PAGE_FLAG_WRITABLE    (1 << 1)
#define PAGE_FLAG_USER        (1 << 2)
#define PAGE_FLAG_WRITETHROUGH (1 << 3)
#define PAGE_FLAG_CACHE_DISABLE (1 << 4)
#define PAGE_FLAG_ACCESSED    (1 << 5)
#define PAGE_FLAG_DIRTY       (1 << 6)
#define PAGE_FLAG_HUGE        (1 << 7)
#define PAGE_FLAG_GLOBAL      (1 << 8)
#define PAGE_FLAG_NO_EXECUTE  (1ULL << 63)

// Memory allocation flags
#define ALLOC_FLAG_ZERO       (1 << 0)
#define ALLOC_FLAG_DMA        (1 << 1)
#define ALLOC_FLAG_KERNEL     (1 << 2)
#define ALLOC_FLAG_USER       (1 << 3)
#define ALLOC_FLAG_EXECUTABLE (1 << 4)

// Memory regions
typedef enum {
    MEMORY_TYPE_AVAILABLE = 1,
    MEMORY_TYPE_RESERVED = 2,
    MEMORY_TYPE_ACPI_RECLAIMABLE = 3,
    MEMORY_TYPE_ACPI_NVS = 4,
    MEMORY_TYPE_BAD = 5,
    MEMORY_TYPE_KERNEL = 6,
    MEMORY_TYPE_INITRD = 7,
    MEMORY_TYPE_FRAMEBUFFER = 8
} memory_type_t;

typedef struct memory_region {
    phys_addr_t start;
    phys_addr_t end;
    memory_type_t type;
    struct memory_region* next;
} memory_region_t;

// Page table structures (x64)
typedef struct {
    u64 entries[512];
} __attribute__((packed, aligned(4096))) page_table_t;

typedef struct {
    page_table_t* pml4;
    page_table_t* pdpt;
    page_table_t* pd;
    page_table_t* pt;
} page_tables_t;

// Virtual memory area
typedef struct vma {
    virt_addr_t start;
    virt_addr_t end;
    u32 flags;
    u32 prot;
    struct vma* next;
    struct vma* prev;
} vma_t;

// Address space
typedef struct address_space {
    page_table_t* page_directory;
    vma_t* vma_list;
    u32 ref_count;
    virt_addr_t heap_start;
    virt_addr_t heap_end;
    virt_addr_t stack_start;
    virt_addr_t stack_end;
} address_space_t;

// Physical memory allocator
typedef struct page_frame {
    u32 ref_count;
    u32 flags;
    struct page_frame* next;
} page_frame_t;

typedef struct {
    page_frame_t* frames;
    u32 total_frames;
    u32 free_frames;
    u32* free_bitmap;
    u32 bitmap_size;
} physical_allocator_t;

// Heap allocator structures
typedef struct heap_block {
    size_t size;
    bool free;
    struct heap_block* next;
    struct heap_block* prev;
    u32 magic;
} heap_block_t;

typedef struct {
    void* start;
    void* end;
    size_t size;
    heap_block_t* free_list;
    u32 total_blocks;
    u32 free_blocks;
} heap_t;

// Global memory statistics
typedef struct {
    u64 total_physical;
    u64 available_physical;
    u64 used_physical;
    u64 total_virtual;
    u64 used_virtual;
    u64 kernel_heap_used;
    u64 user_heap_used;
    u32 page_faults;
    u32 swapped_pages;
} memory_stats_t;

// Core memory management functions
error_t memory_init(void);
void memory_shutdown(void);
memory_stats_t* memory_get_stats(void);

// Physical memory management
error_t physical_memory_init(memory_region_t* regions);
error_t pmm_init_from_handoff(bootloader_handoff_t* handoff);
void pmm_mark_page_used(uint32_t physical_addr);
phys_addr_t physical_alloc_page(void);
phys_addr_t physical_alloc_pages(u32 count);
void physical_free_page(phys_addr_t addr);
void physical_free_pages(phys_addr_t addr, u32 count);
bool physical_is_available(phys_addr_t addr);

// Virtual memory management
error_t virtual_memory_init(void);
error_t virtual_memory_init_from_handoff(bootloader_handoff_t* handoff);
error_t vmm_init_from_handoff(bootloader_handoff_t* handoff);  // Wrapper for compatibility
address_space_t* address_space_create(void);
void address_space_destroy(address_space_t* as);
error_t address_space_switch(address_space_t* as);
address_space_t* address_space_current(void);

// Page table management
error_t page_table_init(void);
error_t page_table_map(address_space_t* as, virt_addr_t virt, phys_addr_t phys, u32 flags);
error_t page_table_unmap(address_space_t* as, virt_addr_t virt);
phys_addr_t page_table_get_physical(address_space_t* as, virt_addr_t virt);
error_t page_table_set_flags(address_space_t* as, virt_addr_t virt, u32 flags);
u32 page_table_get_flags(address_space_t* as, virt_addr_t virt);

// Virtual memory areas
vma_t* vma_create(virt_addr_t start, virt_addr_t end, u32 flags, u32 prot);
void vma_destroy(vma_t* vma);
error_t vma_insert(address_space_t* as, vma_t* vma);
error_t vma_remove(address_space_t* as, vma_t* vma);
vma_t* vma_find(address_space_t* as, virt_addr_t addr);
error_t vma_split(vma_t* vma, virt_addr_t addr);
error_t vma_merge(vma_t* vma1, vma_t* vma2);

// Memory allocation
void* memory_alloc(size_t size);
void* memory_alloc_aligned(size_t size, size_t alignment);
void* memory_calloc(size_t count, size_t size);
void* memory_realloc(void* ptr, size_t new_size);
void memory_free(void* ptr);

// Kernel heap management
error_t kernel_heap_init(void);
void* kernel_heap_alloc(size_t size, u32 flags);
void* kernel_heap_alloc_aligned(size_t size, size_t alignment, u32 flags);
void kernel_heap_free(void* ptr);
size_t kernel_heap_size(void* ptr);

// User memory management
void* user_alloc(address_space_t* as, size_t size, u32 prot);
void* user_alloc_at(address_space_t* as, virt_addr_t addr, size_t size, u32 prot);
void user_free(address_space_t* as, void* ptr);
error_t user_protect(address_space_t* as, void* ptr, size_t size, u32 prot);

// Memory mapping
void* memory_map(virt_addr_t virt, phys_addr_t phys, size_t size, u32 flags);
void memory_unmap(void* virt, size_t size);
void* memory_map_device(phys_addr_t phys, size_t size);
void memory_unmap_device(void* virt, size_t size);

// Copy operations with safety
error_t memory_copy_to_user(address_space_t* as, void* user_ptr, const void* kernel_ptr, size_t size);
error_t memory_copy_from_user(address_space_t* as, void* kernel_ptr, const void* user_ptr, size_t size);
error_t memory_copy_string_from_user(address_space_t* as, char* kernel_str, const char* user_str, size_t max_len);

// Page fault handling
void page_fault_handler(virt_addr_t fault_addr, u32 error_code);

// Cache and TLB management
void memory_flush_tlb(void);
void memory_flush_tlb_page(virt_addr_t addr);
void memory_invalidate_cache(void);

// Memory debugging and diagnostics
void memory_dump_stats(void);
void memory_dump_page_tables(address_space_t* as);
void memory_dump_vmas(address_space_t* as);
bool memory_check_heap_integrity(void);

// Utility macros
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGES_FOR_SIZE(size) (((size) + PAGE_SIZE - 1) / PAGE_SIZE)
#define VIRT_TO_PHYS(virt) page_table_get_physical(address_space_current(), (virt_addr_t)(virt))
#define PHYS_TO_VIRT(phys) ((void*)((phys) + KERNEL_SPACE_START))

// Memory protection flags
#define PROT_READ       (1 << 0)
#define PROT_WRITE      (1 << 1)
#define PROT_EXEC       (1 << 2)
#define PROT_USER       (1 << 3)
#define PROT_KERNEL     (1 << 4)

// Error codes specific to memory management
#define E_MEMORY_OUT_OF_MEMORY     -2000
#define E_MEMORY_INVALID_ADDRESS   -2001
#define E_MEMORY_ACCESS_VIOLATION  -2002
#define E_MEMORY_NOT_MAPPED        -2003
#define E_MEMORY_ALREADY_MAPPED    -2004
#define E_MEMORY_ALIGNMENT_ERROR   -2005
#define E_MEMORY_HEAP_CORRUPTION   -2006

// Magic numbers for heap corruption detection
#define HEAP_BLOCK_MAGIC_ALLOCATED 0xDEADBEEF
#define HEAP_BLOCK_MAGIC_FREE      0xFEEDFACE

#endif // MEMORY_H