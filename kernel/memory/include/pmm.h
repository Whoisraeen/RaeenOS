#ifndef PMM_H
#define PMM_H

#include "../../core/include/types.h"
#include "../../core/include/error.h"
#include "../../core/bootloader_handoff.h"
#include "../memory_integration.h"
#include <stdatomic.h>

// Advanced Physical Memory Manager - Production Grade
// Features superior to Windows/macOS:
// - O(1) allocation/deallocation 
// - NUMA-aware allocation
// - Memory compression
// - Hardware memory encryption support
// - AI-powered optimization
// - Real-time defragmentation

// Page size constants
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define HUGE_PAGE_SIZE      (2 * 1024 * 1024)   // 2MB
#define HUGE_PAGE_SHIFT     21
#define GIGANTIC_PAGE_SIZE  (1024 * 1024 * 1024) // 1GB
#define GIGANTIC_PAGE_SHIFT 30

// Memory zones for optimal allocation
typedef enum {
    MEMORY_ZONE_DMA,        // < 16MB for old DMA devices
    MEMORY_ZONE_DMA32,      // < 4GB for 32-bit DMA
    MEMORY_ZONE_NORMAL,     // Normal system memory
    MEMORY_ZONE_HIGH,       // High memory (>4GB)
    MEMORY_ZONE_DEVICE,     // Device-specific memory
    MEMORY_ZONE_COUNT
} memory_zone_t;

// Memory allocation orders (2^order pages)
typedef enum {
    PAGE_ORDER_4K   = 0,    // 4KB pages
    PAGE_ORDER_8K   = 1,    // 8KB blocks
    PAGE_ORDER_16K  = 2,    // 16KB blocks
    PAGE_ORDER_32K  = 3,    // 32KB blocks
    PAGE_ORDER_64K  = 4,    // 64KB blocks
    PAGE_ORDER_128K = 5,    // 128KB blocks
    PAGE_ORDER_256K = 6,    // 256KB blocks
    PAGE_ORDER_512K = 7,    // 512KB blocks
    PAGE_ORDER_1M   = 8,    // 1MB blocks
    PAGE_ORDER_2M   = 9,    // 2MB huge pages
    PAGE_ORDER_4M   = 10,   // 4MB blocks
    PAGE_ORDER_8M   = 11,   // 8MB blocks
    PAGE_ORDER_16M  = 12,   // 16MB blocks
    PAGE_ORDER_MAX  = 12
} page_order_t;

// Memory allocation flags
#define PMM_FLAG_ZERO       (1 << 0)   // Zero the allocated pages
#define PMM_FLAG_DMA        (1 << 1)   // DMA-capable memory
#define PMM_FLAG_DMA32      (1 << 2)   // 32-bit DMA memory
#define PMM_FLAG_KERNEL     (1 << 3)   // Kernel memory
#define PMM_FLAG_USER       (1 << 4)   // User memory
#define PMM_FLAG_ATOMIC     (1 << 5)   // Atomic allocation (no sleep)
#define PMM_FLAG_NOWAIT     (1 << 6)   // Don't wait for memory
#define PMM_FLAG_HIGHMEM    (1 << 7)   // High memory allowed
#define PMM_FLAG_NUMA_LOCAL (1 << 8)   // NUMA-local allocation
#define PMM_FLAG_ENCRYPTED  (1 << 9)   // Hardware encrypted memory
#define PMM_FLAG_COMPRESSED (1 << 10)  // Allow compressed pages

// Page frame structure - tracks every physical page
typedef struct page_frame {
    atomic_uint_fast32_t flags;         // Page flags and reference count
    atomic_uint_fast16_t ref_count;     // Reference count for sharing
    uint16_t zone;                      // Memory zone
    uint16_t order;                     // Allocation order
    uint16_t numa_node;                 // NUMA node
    
    // Linked list for free pages
    struct page_frame* next;
    struct page_frame* prev;
    
    // Advanced features
    uint32_t compression_ratio;         // Compression ratio (0-1000)
    uint64_t last_access_time;          // For LRU/aging
    uint32_t heat_score;                // Access frequency
    
    // Security and debugging
    uint32_t allocation_tag;            // Allocation source tracking
    uint64_t allocation_time;           // When allocated
    
    // Hardware features
    uint8_t memory_type;                // DDR4, DDR5, HBM, etc.
    uint8_t encryption_key_id;          // Hardware encryption key
    uint8_t ecc_status;                 // ECC error status
    uint8_t reserved;
} __attribute__((aligned(64))) page_frame_t;

// Free page lists for buddy allocator
typedef struct free_list {
    page_frame_t* head;
    atomic_size_t count;
    spinlock_t lock;
} free_list_t;

// Memory zone descriptor
typedef struct memory_zone {
    const char* name;
    phys_addr_t start_addr;
    phys_addr_t end_addr;
    size_t total_pages;
    atomic_size_t free_pages;
    atomic_size_t active_pages;
    atomic_size_t inactive_pages;
    
    // Free lists for different orders
    free_list_t free_lists[PAGE_ORDER_MAX + 1];
    
    // Zone statistics
    atomic_uint64_t allocations;
    atomic_uint64_t deallocations;
    atomic_uint64_t fragmentation_events;
    
    // NUMA information
    uint16_t numa_node;
    uint16_t numa_distance[MAX_NUMA_NODES];
    
    // Advanced features
    bool supports_encryption;
    bool supports_compression;
    bool supports_ecc;
    uint32_t memory_speed_mhz;
    
    spinlock_t lock;
} memory_zone_t;

// NUMA node information
typedef struct numa_node {
    uint16_t node_id;
    memory_zone_t* zones[MEMORY_ZONE_COUNT];
    
    // CPU affinity
    uint64_t cpu_mask;
    
    // Performance characteristics
    uint32_t memory_bandwidth_gbps;
    uint32_t memory_latency_ns;
    
    // Statistics
    atomic_uint64_t local_allocations;
    atomic_uint64_t remote_allocations;
    atomic_uint64_t migrations;
} numa_node_t;

// Global PMM statistics and configuration
typedef struct pmm_global {
    // Basic statistics
    atomic_size_t total_pages;
    atomic_size_t free_pages;
    atomic_size_t allocated_pages;
    atomic_size_t reserved_pages;
    
    // Advanced statistics
    atomic_uint64_t total_allocations;
    atomic_uint64_t total_deallocations;
    atomic_uint64_t allocation_failures;
    atomic_uint64_t compaction_events;
    atomic_uint64_t compression_saves;
    
    // Performance metrics
    atomic_uint64_t allocation_time_total_ns;
    atomic_uint64_t deallocation_time_total_ns;
    uint32_t avg_allocation_time_ns;
    uint32_t avg_deallocation_time_ns;
    
    // Memory pressure and watermarks
    size_t low_watermark;
    size_t high_watermark;
    size_t emergency_watermark;
    atomic_bool memory_pressure;
    
    // Configuration
    bool numa_enabled;
    bool compression_enabled;
    bool encryption_enabled;
    bool defragmentation_enabled;
    
    // AI optimization
    bool ai_optimization_enabled;
    uint32_t allocation_pattern_score;
    uint32_t predicted_memory_usage;
    
    rwlock_t global_lock;
} pmm_global_t;

// Memory allocation request structure
typedef struct alloc_request {
    size_t size;                    // Size in bytes
    page_order_t order;             // Page order
    memory_zone_t preferred_zone;   // Preferred zone
    uint16_t numa_node;             // Preferred NUMA node
    uint32_t flags;                 // Allocation flags
    uint32_t alignment;             // Required alignment
    uint32_t tag;                   // Allocation tag for tracking
    
    // Advanced options
    uint32_t max_retry_count;       // Maximum retry attempts
    uint32_t timeout_ms;            // Allocation timeout
    bool allow_fallback;            // Allow zone fallback
    bool allow_migration;           // Allow page migration
} alloc_request_t;

// Function prototypes

// Initialization
error_t pmm_init(bootloader_handoff_t* handoff);
error_t pmm_init_zones(void);
error_t pmm_init_numa(void);
error_t pmm_late_init(void);

// Core allocation functions
phys_addr_t pmm_alloc_page(uint32_t flags);
phys_addr_t pmm_alloc_pages(page_order_t order, uint32_t flags);
phys_addr_t pmm_alloc_pages_zone(memory_zone_t zone, page_order_t order, uint32_t flags);
phys_addr_t pmm_alloc_pages_numa(uint16_t numa_node, page_order_t order, uint32_t flags);
phys_addr_t pmm_alloc_advanced(alloc_request_t* request);

// Deallocation functions
error_t pmm_free_page(phys_addr_t addr);
error_t pmm_free_pages(phys_addr_t addr, page_order_t order);
error_t pmm_free_pages_bulk(phys_addr_t* addrs, size_t count);

// Page frame management
page_frame_t* pmm_get_page_frame(phys_addr_t addr);
error_t pmm_pin_page(phys_addr_t addr);
error_t pmm_unpin_page(phys_addr_t addr);
uint32_t pmm_get_ref_count(phys_addr_t addr);
error_t pmm_inc_ref_count(phys_addr_t addr);
error_t pmm_dec_ref_count(phys_addr_t addr);

// Advanced features
error_t pmm_compress_page(phys_addr_t addr);
error_t pmm_decompress_page(phys_addr_t addr);
error_t pmm_encrypt_page(phys_addr_t addr, uint8_t key_id);
error_t pmm_decrypt_page(phys_addr_t addr);
error_t pmm_migrate_page(phys_addr_t src, phys_addr_t dst);

// Memory reclaim and compaction
error_t pmm_reclaim_memory(size_t target_pages);
error_t pmm_compact_memory(memory_zone_t zone);
error_t pmm_defragment_zone(memory_zone_t zone);

// NUMA operations
error_t pmm_migrate_to_node(phys_addr_t addr, uint16_t target_node);
uint16_t pmm_get_page_numa_node(phys_addr_t addr);
size_t pmm_get_numa_free_pages(uint16_t node);

// Statistics and monitoring
error_t pmm_get_stats(pmm_global_t* stats);
error_t pmm_get_zone_stats(memory_zone_t zone, void* stats);
error_t pmm_dump_free_lists(void);
error_t pmm_dump_page_frame(phys_addr_t addr);

// AI optimization
error_t pmm_ai_analyze_patterns(void);
error_t pmm_ai_predict_usage(void);
error_t pmm_ai_optimize_zones(void);

// Watermark and pressure management
error_t pmm_set_watermarks(size_t low, size_t high, size_t emergency);
bool pmm_is_memory_pressure(void);
error_t pmm_handle_memory_pressure(void);

// Hardware feature detection
error_t pmm_detect_memory_features(void);
bool pmm_supports_encryption(void);
bool pmm_supports_compression(void);
bool pmm_supports_ecc(void);

// Debug and validation
error_t pmm_validate_page_frame(page_frame_t* frame);
error_t pmm_validate_free_lists(void);
error_t pmm_check_memory_integrity(void);
error_t pmm_stress_test(uint32_t iterations);

// Utility functions
static inline phys_addr_t page_frame_to_addr(page_frame_t* frame);
static inline page_frame_t* addr_to_page_frame(phys_addr_t addr);
static inline bool is_page_aligned(phys_addr_t addr);
static inline size_t pages_to_bytes(size_t pages);
static inline size_t bytes_to_pages(size_t bytes);

// Constants
#define MAX_NUMA_NODES      64
#define PMM_MAX_ORDER       PAGE_ORDER_MAX
#define PMM_MIN_FREE_PAGES  1024
#define PMM_EMERGENCY_PAGES 256

// Global variables (extern declarations)
extern pmm_global_t* pmm_global;
extern memory_zone_t memory_zones[MEMORY_ZONE_COUNT];
extern numa_node_t numa_nodes[MAX_NUMA_NODES];
extern page_frame_t* page_frames;
extern uint16_t numa_node_count;
extern bool pmm_initialized;

#endif // PMM_H