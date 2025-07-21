#ifndef RAEEEN_VM_H
#define RAEEEN_VM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"
#include "process.h"

// Page size constants
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGE_MASK           (PAGE_SIZE - 1)
#define PAGE_ALIGN(addr)    (((addr) + PAGE_MASK) & ~PAGE_MASK)
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~PAGE_MASK)

// Memory protection flags
#define PROT_NONE           0x0
#define PROT_READ           0x1
#define PROT_WRITE          0x2
#define PROT_EXEC           0x4

// VM protection flags (for internal use)
#define VM_PROT_NONE        0x0
#define VM_PROT_READ        0x1
#define VM_PROT_WRITE       0x2
#define VM_PROT_EXEC        0x4

// Memory mapping flags
#define MAP_PRIVATE         0x02
#define MAP_SHARED          0x01
#define MAP_FIXED           0x10
#define MAP_ANONYMOUS       0x20
#define MAP_GROWSDOWN       0x0100
#define MAP_DENYWRITE       0x0800
#define MAP_EXECUTABLE      0x1000
#define MAP_LOCKED          0x2000
#define MAP_NORESERVE       0x4000
#define MAP_POPULATE        0x8000
#define MAP_NONBLOCK        0x10000
#define MAP_STACK           0x20000
#define MAP_HUGETLB         0x40000
#define MAP_SYNC            0x80000
#define MAP_FIXED_NOREPLACE 0x100000

// Virtual memory regions
#define VM_REGION_CODE      0x1
#define VM_REGION_DATA      0x2
#define VM_REGION_STACK     0x4
#define VM_REGION_HEAP      0x8
#define VM_REGION_MMAP      0x10
#define VM_REGION_SHARED    0x20
#define VM_REGION_COW       0x40
#define VM_REGION_ANONYMOUS 0x80

// Page fault types
#define PF_PROT             0x1
#define PF_WRITE            0x2
#define PF_USER             0x4
#define PF_RSVD             0x8
#define PF_INST             0x10

// Memory region structure
typedef struct vm_region {
    uintptr_t start;
    uintptr_t end;
    uintptr_t offset;
    uint32_t flags;
    uint32_t prot;
    struct vm_region *next;
    struct vm_region *prev;
    void *data;  // File mapping data or shared memory data
    size_t size;
    int ref_count;
    bool cow;    // Copy-on-write flag
} vm_region_t;

// Virtual memory space structure
typedef struct vm_space {
    vm_region_t *regions;
    uintptr_t code_start;
    uintptr_t code_end;
    uintptr_t data_start;
    uintptr_t data_end;
    uintptr_t stack_start;
    uintptr_t stack_end;
    uintptr_t heap_start;
    uintptr_t heap_end;
    uintptr_t mmap_start;
    uintptr_t mmap_end;
    uintptr_t brk;
    size_t total_pages;
    spinlock_t lock;
} vm_space_t;

// Page table entry structure
typedef struct {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user:1;
    uint64_t writethrough:1;
    uint64_t cache_disable:1;
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t huge:1;
    uint64_t global:1;
    uint64_t available:3;
    uint64_t frame:40;
    uint64_t reserved:11;
    uint64_t nx:1;
} __attribute__((packed)) page_table_entry_t;

// Page table structure
typedef struct {
    page_table_entry_t entries[512];
} page_table_t;

// Page fault handler context
typedef struct {
    uintptr_t fault_address;
    uint32_t error_code;
    uintptr_t instruction_pointer;
    uintptr_t stack_pointer;
    uint32_t cpu_flags;
} page_fault_context_t;

// Memory mapping structure
typedef struct {
    uintptr_t start;
    uintptr_t end;
    uintptr_t offset;
    uint32_t prot;
    uint32_t flags;
    int fd;
    size_t size;
} memory_mapping_t;

// Shared memory structure
typedef struct {
    uintptr_t physical_addr;
    size_t size;
    int ref_count;
    spinlock_t lock;
    vm_region_t *regions;
} shared_memory_t;

// Function prototypes
void vm_init(void);
vm_space_t *vm_space_create(void);
void vm_space_destroy(vm_space_t *space);
void vm_space_switch(vm_space_t *space);
int vm_map_region(vm_space_t *space, uintptr_t start, uintptr_t end, uint32_t prot, uint32_t flags, void *data);
int vm_unmap_region(vm_space_t *space, uintptr_t start, uintptr_t end);
vm_region_t *vm_find_region(vm_space_t *space, uintptr_t addr);
int vm_protect_region(vm_space_t *space, uintptr_t start, uintptr_t end, uint32_t prot);
int vm_page_fault_handler(page_fault_context_t *context);

// Process-specific virtual memory functions
int vm_create_address_space(process_t *process);
void vm_switch_address_space(process_t *process);
int vm_map_page(process_t *process, uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t prot);
bool vm_validate_user_ptr(process_t *process, void *ptr, size_t size);
int vm_copy_from_user(process_t *process, void *kernel_dst, void *user_src, size_t size);
int vm_copy_to_user(process_t *process, void *user_dst, void *kernel_src, size_t size);

// Memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);

// Memory mapping functions
uintptr_t vm_mmap(vm_space_t *space, uintptr_t addr, size_t length, uint32_t prot, uint32_t flags, int fd, off_t offset);
int vm_munmap(vm_space_t *space, uintptr_t addr, size_t length);
int vm_mprotect(vm_space_t *space, uintptr_t addr, size_t length, uint32_t prot);
int vm_msync(vm_space_t *space, uintptr_t addr, size_t length, int flags);

// Shared memory functions
shared_memory_t *vm_shared_memory_create(size_t size);
void vm_shared_memory_destroy(shared_memory_t *shm);
int vm_shared_memory_map(vm_space_t *space, shared_memory_t *shm, uintptr_t addr, uint32_t prot);
int vm_shared_memory_unmap(vm_space_t *space, uintptr_t addr);

// Copy-on-write functions
int vm_cow_fault(vm_space_t *space, uintptr_t addr);
int vm_cow_copy_page(vm_space_t *space, uintptr_t addr);
bool vm_is_cow_page(vm_space_t *space, uintptr_t addr);

// Demand paging functions
int vm_demand_page_fault(vm_space_t *space, uintptr_t addr);
int vm_load_page_from_file(vm_region_t *region, uintptr_t addr);
int vm_swap_out_page(vm_space_t *space, uintptr_t addr);
int vm_swap_in_page(vm_space_t *space, uintptr_t addr);

// Page fault handling
int vm_handle_page_fault(vm_space_t *space, uintptr_t addr, uint32_t error_code);
bool vm_is_page_present(vm_space_t *space, uintptr_t addr);
int vm_mark_page_dirty(vm_space_t *space, uintptr_t addr);

// Demand paging functions
int vm_demand_page_fault(vm_space_t *space, uintptr_t addr);
int vm_load_page_from_file(vm_region_t *region, uintptr_t addr);
int vm_swap_out_page(vm_space_t *space, uintptr_t addr);
int vm_swap_in_page(vm_space_t *space, uintptr_t addr);

// Page table management
page_table_t *vm_create_page_table(void);
void vm_destroy_page_table(page_table_t *pt);
void vm_map_page(page_table_t *pt, uintptr_t virtual_addr, uintptr_t physical_addr, uint32_t flags);
void vm_unmap_page(page_table_t *pt, uintptr_t virtual_addr);
uintptr_t vm_get_physical_addr(page_table_t *pt, uintptr_t virtual_addr);
void vm_invalidate_tlb(uintptr_t addr);

// Memory allocation functions
uintptr_t vm_alloc_pages(size_t count);
void vm_free_pages(uintptr_t addr, size_t count);
uintptr_t vm_alloc_zeroed_pages(size_t count);
int vm_reserve_pages(uintptr_t start, size_t count);

// Memory statistics
typedef struct {
    size_t total_pages;
    size_t used_pages;
    size_t free_pages;
    size_t shared_pages;
    size_t cow_pages;
    size_t swapped_pages;
} vm_stats_t;

void vm_get_stats(vm_stats_t *stats);
void vm_print_stats(void);

// Memory debugging
void vm_dump_regions(vm_space_t *space);
void vm_dump_page_table(page_table_t *pt, int level);
void vm_validate_space(vm_space_t *space);

// Memory limits and quotas
typedef struct {
    size_t max_virtual_memory;
    size_t max_physical_memory;
    size_t max_mmap_size;
    size_t max_stack_size;
    size_t max_heap_size;
} vm_limits_t;

void vm_set_limits(vm_space_t *space, vm_limits_t *limits);
int vm_check_limits(vm_space_t *space, size_t size);

// Memory protection and security
int vm_mark_executable(vm_space_t *space, uintptr_t start, uintptr_t end);
int vm_mark_readonly(vm_space_t *space, uintptr_t start, uintptr_t end);
int vm_mark_noexec(vm_space_t *space, uintptr_t start, uintptr_t end);
bool vm_is_executable(vm_space_t *space, uintptr_t addr);
bool vm_is_writable(vm_space_t *space, uintptr_t addr);

// Memory locking
int vm_mlock(vm_space_t *space, uintptr_t addr, size_t length);
int vm_munlock(vm_space_t *space, uintptr_t addr, size_t length);
int vm_mlockall(vm_space_t *space, int flags);
int vm_munlockall(vm_space_t *space);

// Memory advice
int vm_madvise(vm_space_t *space, uintptr_t addr, size_t length, int advice);

// Memory synchronization
int vm_msync_region(vm_region_t *region);
int vm_fsync_region(vm_region_t *region);

// Memory compaction
int vm_compact_space(vm_space_t *space);
int vm_defrag_regions(vm_space_t *space);

// Memory monitoring
typedef struct {
    uint64_t page_faults;
    uint64_t cow_faults;
    uint64_t swap_ins;
    uint64_t swap_outs;
    uint64_t mmap_calls;
    uint64_t munmap_calls;
} vm_monitor_t;

void vm_monitor_init(vm_space_t *space);
void vm_monitor_record_fault(vm_space_t *space, int fault_type);
void vm_monitor_get_stats(vm_space_t *space, vm_monitor_t *stats);

#endif // RAEEEN_VM_H 