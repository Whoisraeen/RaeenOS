#include "memory/include/memory.h"
#include "kernel.h"
#include "hal/hal.h"
#include "filesystem/vfs.h"
#include <string.h>

// Demand paging configuration
#define PAGE_FAULT_HANDLER_VECTOR 14
#define SWAP_FILE_SIZE (1024 * 1024 * 1024) // 1GB swap file
#define SWAP_PAGE_SIZE PAGE_SIZE
#define SWAP_SIGNATURE 0x53574150 // "SWAP"

// Page fault types
typedef enum {
    PAGE_FAULT_PRESENT = 0x01,
    PAGE_FAULT_WRITE = 0x02,
    PAGE_FAULT_USER = 0x04,
    PAGE_FAULT_RESERVED = 0x08,
    PAGE_FAULT_INSTRUCTION = 0x10
} page_fault_type_t;

// Swap entry structure
typedef struct {
    u32 page_number;
    u32 swap_offset;
    u32 flags;
    u32 timestamp;
} swap_entry_t;

// Swap file header
typedef struct {
    u32 signature;
    u32 version;
    u32 page_size;
    u32 total_pages;
    u32 used_pages;
    u32 free_list_head;
    u32 checksum;
} swap_header_t;

// Demand paging state
static bool demand_paging_initialized = false;
static swap_entry_t* swap_table = NULL;
static u32 swap_table_size = 0;
static u32 swap_file_fd = -1;
static swap_header_t swap_header = {0};
static u32* swap_free_list = NULL;
static u32 swap_free_list_head = 0;
static u32 swap_free_pages = 0;

// Page fault statistics
static u64 page_faults_total = 0;
static u64 page_faults_resolved = 0;
static u64 page_faults_swapped_in = 0;
static u64 page_faults_swapped_out = 0;

// Forward declarations
static error_t init_swap_file(void);
static error_t swap_out_page(virt_addr_t addr);
static error_t swap_in_page(virt_addr_t addr, u32 swap_offset);
static u32 allocate_swap_page(void);
static void free_swap_page(u32 swap_offset);
static bool is_swap_page(virt_addr_t addr);
static u32 get_swap_offset(virt_addr_t addr);
static void set_swap_offset(virt_addr_t addr, u32 swap_offset);
static void clear_swap_entry(virt_addr_t addr);

// Initialize demand paging system
error_t demand_paging_init(void) {
    if (demand_paging_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing demand paging system");
    
    // Initialize swap file
    error_t result = init_swap_file();
    if (result != SUCCESS) {
        KERROR("Failed to initialize swap file");
        return result;
    }
    
    // Allocate swap table
    swap_table_size = (USER_SPACE_END - USER_SPACE_START) / PAGE_SIZE;
    swap_table = (swap_entry_t*)memory_alloc(swap_table_size * sizeof(swap_entry_t));
    if (!swap_table) {
        KERROR("Failed to allocate swap table");
        return E_NOMEM;
    }
    
    memset(swap_table, 0, swap_table_size * sizeof(swap_entry_t));
    
    // Allocate swap free list
    swap_free_list = (u32*)memory_alloc(swap_header.total_pages * sizeof(u32));
    if (!swap_free_list) {
        KERROR("Failed to allocate swap free list");
        memory_free(swap_table);
        return E_NOMEM;
    }
    
    // Initialize free list
    for (u32 i = 0; i < swap_header.total_pages - 1; i++) {
        swap_free_list[i] = i + 1;
    }
    swap_free_list[swap_header.total_pages - 1] = 0xFFFFFFFF; // End marker
    swap_free_list_head = 0;
    swap_free_pages = swap_header.total_pages;
    
    demand_paging_initialized = true;
    
    KINFO("Demand paging initialized: %u swap pages available", swap_free_pages);
    return SUCCESS;
}

// Initialize swap file
static error_t init_swap_file(void) {
    // Create or open swap file
    swap_file_fd = vfs_open("/swap", O_RDWR | O_CREAT, 0644);
    if (swap_file_fd < 0) {
        KERROR("Failed to open swap file");
        return E_IO;
    }
    
    // Check if swap file is already initialized
    ssize_t bytes_read = vfs_read(swap_file_fd, &swap_header, sizeof(swap_header));
    if (bytes_read == sizeof(swap_header) && swap_header.signature == SWAP_SIGNATURE) {
        KINFO("Found existing swap file: %u pages, %u used", 
              swap_header.total_pages, swap_header.used_pages);
        return SUCCESS;
    }
    
    // Initialize new swap file
    KINFO("Initializing new swap file");
    
    swap_header.signature = SWAP_SIGNATURE;
    swap_header.version = 1;
    swap_header.page_size = PAGE_SIZE;
    swap_header.total_pages = SWAP_FILE_SIZE / PAGE_SIZE;
    swap_header.used_pages = 0;
    swap_header.free_list_head = 0;
    swap_header.checksum = 0; // TODO: Calculate checksum
    
    // Write header
    ssize_t bytes_written = vfs_write(swap_file_fd, &swap_header, sizeof(swap_header));
    if (bytes_written != sizeof(swap_header)) {
        KERROR("Failed to write swap header");
        vfs_close(swap_file_fd);
        return E_IO;
    }
    
    // Extend file to full size
    if (vfs_ftruncate(swap_file_fd, SWAP_FILE_SIZE) != 0) {
        KERROR("Failed to extend swap file");
        vfs_close(swap_file_fd);
        return E_IO;
    }
    
    KINFO("Swap file initialized: %u pages", swap_header.total_pages);
    return SUCCESS;
}

// Handle page fault
error_t handle_page_fault(virt_addr_t fault_addr, u32 error_code) {
    page_faults_total++;
    
    KDEBUG("Page fault at 0x%llx, error code: 0x%x", fault_addr, error_code);
    
    // Decode error code
    bool present = (error_code & PAGE_FAULT_PRESENT) != 0;
    bool write = (error_code & PAGE_FAULT_WRITE) != 0;
    bool user = (error_code & PAGE_FAULT_USER) != 0;
    bool reserved = (error_code & PAGE_FAULT_RESERVED) != 0;
    bool instruction = (error_code & PAGE_FAULT_INSTRUCTION) != 0;
    
    // Check for kernel page faults
    if (!user && fault_addr >= KERNEL_SPACE_START) {
        KERROR("Kernel page fault at 0x%llx", fault_addr);
        return E_FAULT;
    }
    
    // Check for reserved bit violations
    if (reserved) {
        KERROR("Reserved bit violation at 0x%llx", fault_addr);
        return E_FAULT;
    }
    
    // Handle different types of page faults
    if (!present) {
        // Page not present - need to load it
        if (is_swap_page(fault_addr)) {
            // Page is in swap
            u32 swap_offset = get_swap_offset(fault_addr);
            error_t result = swap_in_page(fault_addr, swap_offset);
            if (result == SUCCESS) {
                page_faults_swapped_in++;
                page_faults_resolved++;
                return SUCCESS;
            } else {
                KERROR("Failed to swap in page at 0x%llx", fault_addr);
                return result;
            }
        } else {
            // Page not mapped - create new page
            phys_addr_t phys_addr = physical_alloc_page();
            if (phys_addr == 0) {
                KERROR("Out of physical memory during page fault");
                return E_NOMEM;
            }
            
            // Map the page
            u32 flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
            if (user) {
                flags |= PAGE_FLAG_USER;
            }
            
            error_t result = page_table_map(NULL, fault_addr, phys_addr, flags);
            if (result != SUCCESS) {
                physical_free_page(phys_addr);
                KERROR("Failed to map page at 0x%llx", fault_addr);
                return result;
            }
            
            // Clear the page
            memset((void*)fault_addr, 0, PAGE_SIZE);
            
            page_faults_resolved++;
            return SUCCESS;
        }
    } else if (write && !(error_code & PAGE_FAULT_WRITE)) {
        // Write to read-only page - handle copy-on-write
        KDEBUG("Copy-on-write fault at 0x%llx", fault_addr);
        
        // Allocate new page
        phys_addr_t new_phys = physical_alloc_page();
        if (new_phys == 0) {
            KERROR("Out of physical memory during copy-on-write");
            return E_NOMEM;
        }
        
        // Get old physical address
        phys_addr_t old_phys = page_table_get_physical(NULL, fault_addr);
        
        // Copy data
        memcpy((void*)new_phys, (void*)old_phys, PAGE_SIZE);
        
        // Update mapping
        u32 flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
        if (user) {
            flags |= PAGE_FLAG_USER;
        }
        
        error_t result = page_table_map(NULL, fault_addr, new_phys, flags);
        if (result != SUCCESS) {
            physical_free_page(new_phys);
            KERROR("Failed to map copy-on-write page at 0x%llx", fault_addr);
            return result;
        }
        
        page_faults_resolved++;
        return SUCCESS;
    }
    
    KERROR("Unhandled page fault at 0x%llx", fault_addr);
    return E_FAULT;
}

// Swap out a page to swap file
static error_t swap_out_page(virt_addr_t addr) {
    if (!demand_paging_initialized) {
        return E_NOTINIT;
    }
    
    // Allocate swap space
    u32 swap_offset = allocate_swap_page();
    if (swap_offset == 0xFFFFFFFF) {
        KERROR("No swap space available");
        return E_NOSPC;
    }
    
    // Get physical address
    phys_addr_t phys_addr = page_table_get_physical(NULL, addr);
    if (phys_addr == 0) {
        free_swap_page(swap_offset);
        return E_INVAL;
    }
    
    // Calculate swap file offset
    u64 file_offset = sizeof(swap_header) + (u64)swap_offset * PAGE_SIZE;
    
    // Seek to swap file position
    if (vfs_lseek(swap_file_fd, file_offset, SEEK_SET) != file_offset) {
        KERROR("Failed to seek in swap file");
        free_swap_page(swap_offset);
        return E_IO;
    }
    
    // Write page to swap file
    ssize_t bytes_written = vfs_write(swap_file_fd, (void*)phys_addr, PAGE_SIZE);
    if (bytes_written != PAGE_SIZE) {
        KERROR("Failed to write page to swap file");
        free_swap_page(swap_offset);
        return E_IO;
    }
    
    // Update swap table
    set_swap_offset(addr, swap_offset);
    
    // Unmap the page
    page_table_unmap(NULL, addr);
    
    // Free physical page
    physical_free_page(phys_addr);
    
    page_faults_swapped_out++;
    
    KDEBUG("Swapped out page at 0x%llx to swap offset %u", addr, swap_offset);
    return SUCCESS;
}

// Swap in a page from swap file
static error_t swap_in_page(virt_addr_t addr, u32 swap_offset) {
    if (!demand_paging_initialized) {
        return E_NOTINIT;
    }
    
    // Allocate physical page
    phys_addr_t phys_addr = physical_alloc_page();
    if (phys_addr == 0) {
        KERROR("Out of physical memory during swap in");
        return E_NOMEM;
    }
    
    // Calculate swap file offset
    u64 file_offset = sizeof(swap_header) + (u64)swap_offset * PAGE_SIZE;
    
    // Seek to swap file position
    if (vfs_lseek(swap_file_fd, file_offset, SEEK_SET) != file_offset) {
        KERROR("Failed to seek in swap file");
        physical_free_page(phys_addr);
        return E_IO;
    }
    
    // Read page from swap file
    ssize_t bytes_read = vfs_read(swap_file_fd, (void*)phys_addr, PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        KERROR("Failed to read page from swap file");
        physical_free_page(phys_addr);
        return E_IO;
    }
    
    // Map the page
    u32 flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_USER;
    error_t result = page_table_map(NULL, addr, phys_addr, flags);
    if (result != SUCCESS) {
        physical_free_page(phys_addr);
        KERROR("Failed to map swapped-in page at 0x%llx", addr);
        return result;
    }
    
    // Clear swap table entry
    clear_swap_entry(addr);
    
    // Free swap space
    free_swap_page(swap_offset);
    
    KDEBUG("Swapped in page at 0x%llx from swap offset %u", addr, swap_offset);
    return SUCCESS;
}

// Allocate a swap page
static u32 allocate_swap_page(void) {
    if (swap_free_pages == 0) {
        return 0xFFFFFFFF;
    }
    
    u32 swap_offset = swap_free_list_head;
    swap_free_list_head = swap_free_list[swap_offset];
    swap_free_pages--;
    
    return swap_offset;
}

// Free a swap page
static void free_swap_page(u32 swap_offset) {
    if (swap_offset >= swap_header.total_pages) {
        return;
    }
    
    swap_free_list[swap_offset] = swap_free_list_head;
    swap_free_list_head = swap_offset;
    swap_free_pages++;
}

// Check if a page is in swap
static bool is_swap_page(virt_addr_t addr) {
    u32 page_index = (addr - USER_SPACE_START) / PAGE_SIZE;
    if (page_index >= swap_table_size) {
        return false;
    }
    
    return swap_table[page_index].swap_offset != 0;
}

// Get swap offset for a page
static u32 get_swap_offset(virt_addr_t addr) {
    u32 page_index = (addr - USER_SPACE_START) / PAGE_SIZE;
    if (page_index >= swap_table_size) {
        return 0;
    }
    
    return swap_table[page_index].swap_offset;
}

// Set swap offset for a page
static void set_swap_offset(virt_addr_t addr, u32 swap_offset) {
    u32 page_index = (addr - USER_SPACE_START) / PAGE_SIZE;
    if (page_index >= swap_table_size) {
        return;
    }
    
    swap_table[page_index].swap_offset = swap_offset;
    swap_table[page_index].timestamp = hal_get_timestamp();
}

// Clear swap entry for a page
static void clear_swap_entry(virt_addr_t addr) {
    u32 page_index = (addr - USER_SPACE_START) / PAGE_SIZE;
    if (page_index >= swap_table_size) {
        return;
    }
    
    swap_table[page_index].swap_offset = 0;
    swap_table[page_index].timestamp = 0;
}

// Memory pressure handler - called when physical memory is low
error_t handle_memory_pressure(void) {
    if (!demand_paging_initialized) {
        return E_NOTINIT;
    }
    
    KDEBUG("Handling memory pressure");
    
    // Find least recently used pages to swap out
    // This is a simple implementation - real systems use more sophisticated algorithms
    
    u32 pages_to_swap = 10; // Swap out 10 pages at a time
    u32 pages_swapped = 0;
    
    for (u32 i = 0; i < swap_table_size && pages_swapped < pages_to_swap; i++) {
        virt_addr_t addr = USER_SPACE_START + i * PAGE_SIZE;
        
        // Check if page is mapped and not in swap
        if (page_table_get_physical(NULL, addr) != 0 && !is_swap_page(addr)) {
            error_t result = swap_out_page(addr);
            if (result == SUCCESS) {
                pages_swapped++;
            }
        }
    }
    
    KDEBUG("Swapped out %u pages due to memory pressure", pages_swapped);
    return SUCCESS;
}

// Get demand paging statistics
void get_demand_paging_stats(u64* total_faults, u64* resolved_faults, 
                           u64* swapped_in, u64* swapped_out) {
    if (total_faults) *total_faults = page_faults_total;
    if (resolved_faults) *resolved_faults = page_faults_resolved;
    if (swapped_in) *swapped_in = page_faults_swapped_in;
    if (swapped_out) *swapped_out = page_faults_swapped_out;
}

// Dump demand paging statistics
void dump_demand_paging_stats(void) {
    KINFO("=== Demand Paging Statistics ===");
    KINFO("Total page faults: %llu", page_faults_total);
    KINFO("Resolved page faults: %llu", page_faults_resolved);
    KINFO("Pages swapped in: %llu", page_faults_swapped_in);
    KINFO("Pages swapped out: %llu", page_faults_swapped_out);
    KINFO("Swap file: %u total pages, %u free pages", 
          swap_header.total_pages, swap_free_pages);
    KINFO("Resolution rate: %.2f%%", 
          (double)page_faults_resolved * 100.0 / page_faults_total);
}

// Shutdown demand paging system
void demand_paging_shutdown(void) {
    if (!demand_paging_initialized) {
        return;
    }
    
    KINFO("Shutting down demand paging system");
    
    // Close swap file
    if (swap_file_fd >= 0) {
        vfs_close(swap_file_fd);
        swap_file_fd = -1;
    }
    
    // Free memory
    if (swap_table) {
        memory_free(swap_table);
        swap_table = NULL;
    }
    
    if (swap_free_list) {
        memory_free(swap_free_list);
        swap_free_list = NULL;
    }
    
    demand_paging_initialized = false;
} 