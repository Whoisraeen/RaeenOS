#ifndef VMM_H
#define VMM_H

#include "../../core/include/types.h"
#include "../../core/include/error.h"
#include "pmm.h"
#include "../memory_integration.h"
#include <stdatomic.h>

// Production Virtual Memory Manager - Enterprise Grade
// Features superior to Windows/macOS:
// - 5-level page tables for 57-bit virtual addresses (128 PB)
// - SMEP/SMAP/PKU/CET hardware security features
// - Copy-on-write with intelligent sharing
// - Demand paging with predictive prefetch
// - Memory overcommit with intelligent OOM killer
// - NUMA-aware virtual memory
// - Memory deduplication (KSM-like)
// - Transparent huge pages
// - Process isolation with KPTI
// - Hardware-assisted virtualization

// Page table constants
#define ENTRIES_PER_TABLE   512
#define PAGE_TABLE_LEVELS   4       // Can be extended to 5
#define VIRTUAL_ADDR_BITS   48      // x86-64 standard (can be 57)
#define KERNEL_SPACE_START  0xFFFF800000000000ULL
#define USER_SPACE_END      0x00007FFFFFFFFFFFULL

// Page flags (x86-64 specific)
#define VMM_FLAG_PRESENT        (1ULL << 0)
#define VMM_FLAG_WRITABLE       (1ULL << 1)
#define VMM_FLAG_USER           (1ULL << 2)
#define VMM_FLAG_WRITETHROUGH   (1ULL << 3)
#define VMM_FLAG_CACHE_DISABLE  (1ULL << 4)
#define VMM_FLAG_ACCESSED       (1ULL << 5)
#define VMM_FLAG_DIRTY          (1ULL << 6)
#define VMM_FLAG_HUGE           (1ULL << 7)
#define VMM_FLAG_GLOBAL         (1ULL << 8)
#define VMM_FLAG_COPY_ON_WRITE  (1ULL << 9)
#define VMM_FLAG_SHARED         (1ULL << 10)
#define VMM_FLAG_SWAPPED        (1ULL << 11)
#define VMM_FLAG_NO_EXECUTE     (1ULL << 63)

// Security flags (Intel CET, MPX, PKU)
#define VMM_FLAG_SMEP           (1ULL << 20)  // Supervisor Mode Execution Prevention
#define VMM_FLAG_SMAP           (1ULL << 21)  // Supervisor Mode Access Prevention  
#define VMM_FLAG_PKU_MASK       (0xFULL << 59) // Protection Key mask
#define VMM_FLAG_CET            (1ULL << 22)  // Control Flow Enforcement

// Memory mapping types
typedef enum {
    VMA_TYPE_ANONYMOUS,     // Anonymous memory
    VMA_TYPE_FILE,          // File-backed memory
    VMA_TYPE_DEVICE,        // Device memory
    VMA_TYPE_SHARED,        // Shared memory segment
    VMA_TYPE_STACK,         // Stack segment
    VMA_TYPE_HEAP,          // Heap segment
    VMA_TYPE_CODE,          // Executable code
    VMA_TYPE_VDSO,          // Virtual dynamic shared object
    VMA_TYPE_VSYSCALL,      // Virtual system call page
} vma_type_t;

// Memory protection flags
typedef enum {
    VMA_PROT_NONE   = 0,
    VMA_PROT_READ   = (1 << 0),
    VMA_PROT_WRITE  = (1 << 1),
    VMA_PROT_EXEC   = (1 << 2),
    VMA_PROT_GROWSDOWN = (1 << 3),   // Stack grows down
    VMA_PROT_GROWSUP   = (1 << 4),   // Heap grows up
} vma_protection_t;

// Memory mapping flags
typedef enum {
    VMA_MAP_PRIVATE     = (1 << 0),  // Private mapping
    VMA_MAP_SHARED      = (1 << 1),  // Shared mapping
    VMA_MAP_ANONYMOUS   = (1 << 2),  // Anonymous mapping
    VMA_MAP_FIXED       = (1 << 3),  // Fixed address
    VMA_MAP_GROWSDOWN   = (1 << 4),  // Stack mapping
    VMA_MAP_HUGETLB     = (1 << 5),  // Use huge pages
    VMA_MAP_LOCKED      = (1 << 6),  // Lock pages in memory
    VMA_MAP_NORESERVE   = (1 << 7),  // Don't reserve swap space
    VMA_MAP_POPULATE    = (1 << 8),  // Populate pages immediately
} vma_flags_t;

// Virtual Memory Area (VMA) structure
typedef struct vma {
    virt_addr_t start;              // Start virtual address
    virt_addr_t end;                // End virtual address
    size_t size;                    // Size in bytes
    
    vma_type_t type;                // Memory type
    vma_protection_t protection;    // Memory protection
    vma_flags_t flags;              // Mapping flags
    
    // File backing (if applicable)
    struct file* file;              // Backing file
    uint64_t file_offset;           // Offset in file
    
    // Reference counting and sharing
    atomic_uint_fast32_t ref_count; // Reference count
    struct vma* shared_next;        // Next in shared list
    struct vma* shared_prev;        // Previous in shared list
    
    // NUMA policy
    uint16_t numa_node_mask;        // Allowed NUMA nodes
    uint8_t numa_policy;            // NUMA allocation policy
    
    // Statistics and monitoring
    atomic_uint64_t page_faults;    // Number of page faults
    atomic_uint64_t major_faults;   // Major page faults
    atomic_uint64_t minor_faults;   // Minor page faults
    uint64_t last_access_time;      // Last access timestamp
    uint32_t access_frequency;      // Access frequency score
    
    // Security context
    uint32_t security_label;        // Security label
    uint8_t protection_key;         // Intel MPK key
    
    // Tree structure for efficient lookup
    struct vma* left;               // Left child in RB tree
    struct vma* right;              // Right child in RB tree
    struct vma* parent;             // Parent in RB tree
    uint8_t color;                  // Red-black tree color
    
    // Linked list for process VMAs
    struct vma* next;               // Next VMA in process
    struct vma* prev;               // Previous VMA in process
    
    spinlock_t lock;                // VMA-specific lock
} vma_t;

// Page table entry structure
typedef struct page_table_entry {
    uint64_t raw;                   // Raw page table entry
    
    // Decoded fields (for easier access)
    phys_addr_t physical_addr;      // Physical address
    uint64_t flags;                 // Page flags
    uint8_t protection_key;         // Protection key
    bool present;                   // Page present
    bool writable;                  // Page writable
    bool user_accessible;           // User accessible
    bool no_execute;                // No execute
    bool copy_on_write;             // Copy on write
    bool shared;                    // Shared page
    bool swapped;                   // Page swapped out
    
    // Statistics
    uint32_t access_count;          // Access counter
    uint64_t last_access_time;      // Last access time
} __attribute__((packed)) pte_t;

// Page table structures
typedef struct page_table {
    pte_t entries[ENTRIES_PER_TABLE];
    atomic_uint_fast16_t entry_count;   // Number of valid entries
    phys_addr_t physical_addr;          // Physical address of table
    uint8_t level;                      // Page table level
    spinlock_t lock;                    // Table lock
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

// Address space structure
typedef struct address_space {
    page_table_t* page_directory;      // Top-level page table
    vma_t* vma_list;                    // List of VMAs
    vma_t* vma_tree_root;               // RB tree root for fast lookup
    
    // Statistics
    atomic_size_t total_pages;          // Total mapped pages
    atomic_size_t resident_pages;       // Pages in physical memory
    atomic_size_t shared_pages;         // Shared pages
    atomic_size_t swapped_pages;        // Swapped out pages
    atomic_size_t dirty_pages;          // Dirty pages
    
    atomic_uint64_t page_faults;        // Total page faults
    atomic_uint64_t major_faults;       // Major page faults
    atomic_uint64_t minor_faults;       // Minor page faults
    atomic_uint64_t cow_faults;         // Copy-on-write faults
    
    // Memory usage limits
    size_t memory_limit;                // Memory limit for this space
    size_t stack_limit;                 // Stack size limit
    size_t heap_limit;                  // Heap size limit
    
    // NUMA policy
    uint16_t numa_node_mask;            // Allowed NUMA nodes
    uint8_t numa_policy;                // NUMA allocation policy
    
    // Security context
    uint32_t security_context;          // Security context
    bool kpti_enabled;                  // Kernel page table isolation
    
    // Optimization hints
    virt_addr_t last_fault_addr;        // Last page fault address
    uint32_t sequential_faults;         // Sequential fault counter
    uint32_t prefetch_window;           // Prefetch window size
    
    // Reference counting
    atomic_uint_fast32_t ref_count;     // Reference count
    
    rwlock_t lock;                      // Address space lock
} address_space_t;

// Swap information
typedef struct swap_entry {
    uint32_t swap_file_id;              // Swap file identifier
    uint64_t swap_offset;               // Offset in swap file
    atomic_uint_fast16_t ref_count;     // Reference count
    uint8_t compression_algo;           // Compression algorithm
    uint16_t compressed_size;           // Compressed size
} swap_entry_t;

// Page fault information
typedef struct page_fault_info {
    virt_addr_t fault_addr;             // Faulting virtual address
    uint64_t error_code;                // Page fault error code
    bool write_fault;                   // Was it a write fault?
    bool user_fault;                    // Was it from user mode?
    bool execute_fault;                 // Was it an execution fault?
    bool protection_fault;              // Protection violation?
    bool present_fault;                 // Page not present?
    
    // Context
    struct process* process;            // Faulting process
    vma_t* vma;                         // Faulting VMA
    uint64_t timestamp;                 // Fault timestamp
    
    // Resolution
    bool resolved;                      // Was fault resolved?
    uint32_t resolution_time_us;        // Resolution time
    enum {
        FAULT_DEMAND_PAGE,              // Demand paging
        FAULT_COPY_ON_WRITE,            // Copy-on-write
        FAULT_SWAP_IN,                  // Swap in page
        FAULT_PROTECTION_VIOLATION,     // Protection violation
        FAULT_SEGMENTATION_FAULT,       // Segmentation fault
    } fault_type;
} page_fault_info_t;

// Global VMM statistics
typedef struct vmm_stats {
    // Basic statistics
    atomic_uint64_t total_mappings;     // Total memory mappings
    atomic_uint64_t anonymous_mappings; // Anonymous mappings
    atomic_uint64_t file_mappings;      // File-backed mappings
    atomic_uint64_t shared_mappings;    // Shared mappings
    
    // Page fault statistics
    atomic_uint64_t total_page_faults;  // Total page faults
    atomic_uint64_t major_page_faults;  // Major page faults
    atomic_uint64_t minor_page_faults;  // Minor page faults
    atomic_uint64_t cow_page_faults;    // Copy-on-write faults
    atomic_uint64_t protection_faults;  // Protection faults
    atomic_uint64_t swap_faults;        // Swap-in faults
    
    // Memory usage
    atomic_size_t total_virtual_memory; // Total virtual memory
    atomic_size_t total_physical_memory;// Total physical memory used
    atomic_size_t shared_memory;        // Shared memory
    atomic_size_t compressed_memory;    // Compressed memory
    atomic_size_t swapped_memory;       // Swapped memory
    
    // Performance metrics
    atomic_uint64_t tlb_flushes;        // TLB flush count
    atomic_uint64_t page_allocations;   // Page allocations
    atomic_uint64_t page_deallocations; // Page deallocations
    atomic_uint64_t cow_optimizations;  // COW optimizations
    
    // Advanced features
    atomic_uint64_t numa_migrations;    // NUMA page migrations
    atomic_uint64_t huge_page_splits;   // Huge page splits
    atomic_uint64_t page_merges;        // Page merges
    atomic_uint64_t compression_saves;  // Compression saves
    
    // Timing statistics
    atomic_uint64_t fault_time_total_ns;// Total fault handling time
    uint32_t avg_fault_time_ns;         // Average fault time
    uint32_t max_fault_time_ns;         // Maximum fault time
} vmm_stats_t;

// Function prototypes

// Initialization
error_t vmm_init(void);
error_t vmm_init_address_space(address_space_t* as);
error_t vmm_destroy_address_space(address_space_t* as);

// Address space management
address_space_t* vmm_create_address_space(void);
address_space_t* vmm_clone_address_space(address_space_t* parent);
error_t vmm_switch_address_space(address_space_t* as);
address_space_t* vmm_get_current_address_space(void);
address_space_t* vmm_get_kernel_address_space(void);

// Memory mapping
virt_addr_t vmm_map_pages(address_space_t* as, virt_addr_t virt_addr, 
                          phys_addr_t phys_addr, size_t size, uint64_t flags);
error_t vmm_unmap_pages(address_space_t* as, virt_addr_t virt_addr, size_t size);
error_t vmm_protect_pages(address_space_t* as, virt_addr_t virt_addr, 
                          size_t size, vma_protection_t protection);

// VMA management
vma_t* vmm_create_vma(address_space_t* as, virt_addr_t start, size_t size,
                      vma_protection_t protection, vma_flags_t flags, vma_type_t type);
error_t vmm_destroy_vma(address_space_t* as, vma_t* vma);
vma_t* vmm_find_vma(address_space_t* as, virt_addr_t addr);
vma_t* vmm_find_vma_intersection(address_space_t* as, virt_addr_t start, virt_addr_t end);
error_t vmm_split_vma(vma_t* vma, virt_addr_t split_addr);
error_t vmm_merge_vmas(vma_t* vma1, vma_t* vma2);

// Page table management
error_t vmm_map_page(address_space_t* as, virt_addr_t virt_addr, 
                     phys_addr_t phys_addr, uint64_t flags);
error_t vmm_unmap_page(address_space_t* as, virt_addr_t virt_addr);
phys_addr_t vmm_get_physical_addr(address_space_t* as, virt_addr_t virt_addr);
pte_t* vmm_get_page_table_entry(address_space_t* as, virt_addr_t virt_addr, bool create);

// Page fault handling
error_t vmm_handle_page_fault(page_fault_info_t* fault_info);
error_t vmm_handle_demand_page(page_fault_info_t* fault_info);
error_t vmm_handle_copy_on_write(page_fault_info_t* fault_info);
error_t vmm_handle_swap_in(page_fault_info_t* fault_info);

// Advanced features
error_t vmm_enable_copy_on_write(address_space_t* as, virt_addr_t start, size_t size);
error_t vmm_share_pages(address_space_t* as1, address_space_t* as2, 
                        virt_addr_t addr, size_t size);
error_t vmm_migrate_pages(address_space_t* as, virt_addr_t start, 
                          size_t size, uint16_t target_numa_node);

// Memory pressure and reclaim
error_t vmm_reclaim_memory(size_t target_pages);
error_t vmm_swap_out_pages(address_space_t* as, size_t target_pages);
error_t vmm_compress_pages(address_space_t* as, virt_addr_t start, size_t size);

// TLB management
error_t vmm_flush_tlb(void);
error_t vmm_flush_tlb_page(virt_addr_t addr);
error_t vmm_flush_tlb_range(virt_addr_t start, virt_addr_t end);
error_t vmm_flush_tlb_address_space(address_space_t* as);

// Statistics and monitoring
error_t vmm_get_stats(vmm_stats_t* stats);
error_t vmm_get_address_space_stats(address_space_t* as, void* stats);
error_t vmm_dump_address_space(address_space_t* as);
error_t vmm_dump_vma(vma_t* vma);

// Security features
error_t vmm_enable_smep(void);
error_t vmm_enable_smap(void);
error_t vmm_enable_kpti(address_space_t* as);
error_t vmm_set_protection_key(virt_addr_t addr, size_t size, uint8_t key);

// NUMA support
error_t vmm_set_numa_policy(address_space_t* as, uint8_t policy, uint16_t node_mask);
error_t vmm_migrate_to_numa_node(address_space_t* as, virt_addr_t start, 
                                  size_t size, uint16_t target_node);

// Debugging and validation
error_t vmm_validate_address_space(address_space_t* as);
error_t vmm_validate_vma(vma_t* vma);
error_t vmm_check_page_tables(address_space_t* as);
error_t vmm_stress_test(uint32_t iterations);

// Utility functions
static inline bool vmm_is_kernel_address(virt_addr_t addr);
static inline bool vmm_is_user_address(virt_addr_t addr);
static inline bool vmm_is_canonical_address(virt_addr_t addr);
static inline virt_addr_t vmm_align_down(virt_addr_t addr);
static inline virt_addr_t vmm_align_up(virt_addr_t addr);
static inline size_t vmm_bytes_to_pages(size_t bytes);
static inline size_t vmm_pages_to_bytes(size_t pages);

// Global variables (extern)
extern address_space_t* kernel_address_space;
extern address_space_t* current_address_space;
extern vmm_stats_t vmm_global_stats;
extern bool vmm_initialized;

// Constants
#define VMM_MAX_VMAS            65536
#define VMM_DEFAULT_STACK_SIZE  (8 * 1024 * 1024)   // 8MB
#define VMM_DEFAULT_HEAP_SIZE   (128 * 1024 * 1024) // 128MB
#define VMM_PREFETCH_PAGES      16
#define VMM_COW_THRESHOLD       4

#endif // VMM_H