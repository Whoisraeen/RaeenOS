#include "vm.h"
#include "memory.h"
#include "process.h"
#include <string.h>
#include <stddef.h>

// Page replacement algorithm constants
#define PAGE_REPLACEMENT_LRU    0
#define PAGE_REPLACEMENT_FIFO   1
#define PAGE_REPLACEMENT_CLOCK  2

// Page states
#define PAGE_STATE_FREE         0
#define PAGE_STATE_USED         1
#define PAGE_STATE_DIRTY        2
#define PAGE_STATE_SWAPPED      3
#define PAGE_STATE_COW          4

// Page table entry flags
#define PTE_PRESENT     0x001
#define PTE_WRITABLE    0x002
#define PTE_USER         0x004
#define PTE_WRITETHROUGH 0x008
#define PTE_CACHE_DISABLE 0x010
#define PTE_ACCESSED    0x020
#define PTE_DIRTY       0x040
#define PTE_HUGE        0x080
#define PTE_GLOBAL      0x100
#define PTE_COW         0x200
#define PTE_SWAPPED    0x400

// Page frame structure
typedef struct page_frame {
    uintptr_t physical_addr;    // Physical address
    uint32_t ref_count;         // Reference count
    uint32_t state;             // Page state
    uint64_t last_access;       // Last access time
    uint32_t flags;             // Page flags
    vm_space_t* owner;          // Owning address space
    uintptr_t virtual_addr;     // Virtual address in owner
    struct page_frame* next;    // Next in list
    struct page_frame* prev;    // Previous in list
} page_frame_t;

// Page replacement context
typedef struct page_replacement {
    uint32_t algorithm;         // Replacement algorithm
    page_frame_t* free_list;    // Free page frames
    page_frame_t* used_list;    // Used page frames
    uint32_t total_frames;      // Total page frames
    uint32_t free_frames;       // Free page frames
    uint32_t clock_hand;        // Clock hand for clock algorithm
    spinlock_t lock;            // Replacement lock
} page_replacement_t;

// Swap space structure
typedef struct swap_space {
    uintptr_t start_addr;       // Start address of swap space
    size_t size;                // Size of swap space
    uint32_t* bitmap;           // Allocation bitmap
    uint32_t total_pages;       // Total pages in swap
    uint32_t free_pages;        // Free pages in swap
    spinlock_t lock;            // Swap space lock
} swap_space_t;

// Global variables
static page_replacement_t page_replacement = {0};
static swap_space_t swap_space = {0};
static page_frame_t* page_frames = NULL;
static uint32_t total_page_frames = 0;

// Initialize advanced memory management
int vm_advanced_init(void) {
    // Initialize page replacement
    memset(&page_replacement, 0, sizeof(page_replacement_t));
    page_replacement.algorithm = PAGE_REPLACEMENT_LRU;
    page_replacement.lock = SPINLOCK_INIT;
    
    // Calculate total page frames
    total_page_frames = pmm_get_total_pages();
    page_frames = kmalloc(sizeof(page_frame_t) * total_page_frames);
    if (!page_frames) {
        return -1;
    }
    
    // Initialize page frames
    memset(page_frames, 0, sizeof(page_frame_t) * total_page_frames);
    for (uint32_t i = 0; i < total_page_frames; i++) {
        page_frames[i].physical_addr = i * PAGE_SIZE;
        page_frames[i].state = PAGE_STATE_FREE;
        page_frames[i].ref_count = 0;
        page_frames[i].last_access = 0;
        page_frames[i].flags = 0;
        page_frames[i].owner = NULL;
        page_frames[i].virtual_addr = 0;
        
        // Add to free list
        page_frames[i].next = page_replacement.free_list;
        if (page_replacement.free_list) {
            page_replacement.free_list->prev = &page_frames[i];
        }
        page_replacement.free_list = &page_frames[i];
    }
    
    page_replacement.total_frames = total_page_frames;
    page_replacement.free_frames = total_page_frames;
    
    // Initialize swap space (for now, use a portion of RAM)
    swap_space.start_addr = 0x1000000; // 16MB
    swap_space.size = 64 * 1024 * 1024; // 64MB
    swap_space.total_pages = swap_space.size / PAGE_SIZE;
    swap_space.free_pages = swap_space.total_pages;
    
    // Allocate bitmap
    uint32_t bitmap_size = (swap_space.total_pages + 31) / 32;
    swap_space.bitmap = kmalloc(bitmap_size * sizeof(uint32_t));
    if (!swap_space.bitmap) {
        kfree(page_frames);
        return -1;
    }
    
    memset(swap_space.bitmap, 0, bitmap_size * sizeof(uint32_t));
    swap_space.lock = SPINLOCK_INIT;
    
    KINFO("Advanced memory management initialized");
    KINFO("Total page frames: %d", total_page_frames);
    KINFO("Swap space: %d pages", swap_space.total_pages);
    
    return 0;
}

// Handle page fault
int vm_handle_page_fault(vm_space_t* space, uintptr_t addr, uint32_t error_code) {
    if (!space) {
        return -1;
    }
    
    // Check if it's a protection fault
    bool is_write = (error_code & 0x2) != 0;
    bool is_user = (error_code & 0x4) != 0;
    bool is_present = (error_code & 0x1) != 0;
    
    if (is_present) {
        // Page is present but access denied
        return -1; // EACCES
    }
    
    // Handle demand paging
    return vm_demand_page_fault(space, addr);
}

// Demand page fault handler
int vm_demand_page_fault(vm_space_t* space, uintptr_t addr) {
    if (!space) {
        return -1;
    }
    
    // Find the memory region containing this address
    vm_region_t* region = vm_find_region(space, addr);
    if (!region) {
        return -1; // EFAULT
    }
    
    // Check if the address is within the region
    if (addr < region->start || addr >= region->end) {
        return -1; // EFAULT
    }
    
    // Allocate a new page
    page_frame_t* frame = vm_allocate_page_frame();
    if (!frame) {
        // No free frames, need to swap out
        frame = vm_swap_out_page_frame();
        if (!frame) {
            return -1; // ENOMEM
        }
    }
    
    // Map the page
    if (vm_map_page(space, addr, frame->physical_addr, region->prot) != 0) {
        vm_free_page_frame(frame);
        return -1;
    }
    
    // Set up the page frame
    frame->owner = space;
    frame->virtual_addr = addr;
    frame->ref_count = 1;
    frame->state = PAGE_STATE_USED;
    frame->last_access = hal_get_timestamp();
    frame->flags = region->prot;
    
    // Load page content if needed
    if (region->file_path) {
        vm_load_page_from_file(region, addr);
    } else {
        // Zero the page
        memset((void*)frame->physical_addr, 0, PAGE_SIZE);
    }
    
    return 0;
}

// Allocate a page frame
page_frame_t* vm_allocate_page_frame(void) {
    spinlock_acquire(&page_replacement.lock);
    
    page_frame_t* frame = page_replacement.free_list;
    if (frame) {
        // Remove from free list
        page_replacement.free_list = frame->next;
        if (frame->next) {
            frame->next->prev = NULL;
        }
        
        // Add to used list
        frame->next = page_replacement.used_list;
        if (page_replacement.used_list) {
            page_replacement.used_list->prev = frame;
        }
        page_replacement.used_list = frame;
        frame->prev = NULL;
        
        page_replacement.free_frames--;
    }
    
    spinlock_release(&page_replacement.lock);
    return frame;
}

// Free a page frame
void vm_free_page_frame(page_frame_t* frame) {
    if (!frame) {
        return;
    }
    
    spinlock_acquire(&page_replacement.lock);
    
    // Remove from used list
    if (frame->prev) {
        frame->prev->next = frame->next;
    } else {
        page_replacement.used_list = frame->next;
    }
    
    if (frame->next) {
        frame->next->prev = frame->prev;
    }
    
    // Add to free list
    frame->next = page_replacement.free_list;
    if (page_replacement.free_list) {
        page_replacement.free_list->prev = frame;
    }
    page_replacement.free_list = frame;
    frame->prev = NULL;
    
    // Reset frame
    frame->ref_count = 0;
    frame->state = PAGE_STATE_FREE;
    frame->owner = NULL;
    frame->virtual_addr = 0;
    frame->flags = 0;
    
    page_replacement.free_frames++;
    
    spinlock_release(&page_replacement.lock);
}

// Swap out a page frame
page_frame_t* vm_swap_out_page_frame(void) {
    spinlock_acquire(&page_replacement.lock);
    
    page_frame_t* victim = NULL;
    
    // Select victim based on replacement algorithm
    switch (page_replacement.algorithm) {
        case PAGE_REPLACEMENT_LRU:
            victim = vm_select_lru_victim();
            break;
        case PAGE_REPLACEMENT_FIFO:
            victim = vm_select_fifo_victim();
            break;
        case PAGE_REPLACEMENT_CLOCK:
            victim = vm_select_clock_victim();
            break;
        default:
            victim = vm_select_lru_victim();
            break;
    }
    
    if (!victim) {
        spinlock_release(&page_replacement.lock);
        return NULL;
    }
    
    // Swap out the victim
    if (vm_swap_out_page(victim) != 0) {
        spinlock_release(&page_replacement.lock);
        return NULL;
    }
    
    // Remove from used list
    if (victim->prev) {
        victim->prev->next = victim->next;
    } else {
        page_replacement.used_list = victim->next;
    }
    
    if (victim->next) {
        victim->next->prev = victim->prev;
    }
    
    // Reset frame
    victim->ref_count = 0;
    victim->state = PAGE_STATE_FREE;
    victim->owner = NULL;
    victim->virtual_addr = 0;
    victim->flags = 0;
    
    spinlock_release(&page_replacement.lock);
    return victim;
}

// Select LRU victim
page_frame_t* vm_select_lru_victim(void) {
    page_frame_t* victim = NULL;
    uint64_t oldest_access = UINT64_MAX;
    
    page_frame_t* frame = page_replacement.used_list;
    while (frame) {
        if (frame->ref_count == 0 && frame->last_access < oldest_access) {
            victim = frame;
            oldest_access = frame->last_access;
        }
        frame = frame->next;
    }
    
    return victim;
}

// Select FIFO victim
page_frame_t* vm_select_fifo_victim(void) {
    // For FIFO, we can just return the oldest frame in the list
    page_frame_t* frame = page_replacement.used_list;
    while (frame && frame->next) {
        frame = frame->next;
    }
    
    return frame;
}

// Select clock victim
page_frame_t* vm_select_clock_victim(void) {
    // Simple clock algorithm implementation
    page_frame_t* frame = page_replacement.used_list;
    uint32_t checked = 0;
    
    while (frame && checked < page_replacement.total_frames) {
        if (frame->ref_count == 0) {
            return frame;
        }
        
        // Clear reference bit
        frame->ref_count = 0;
        
        frame = frame->next;
        if (!frame) {
            frame = page_replacement.used_list;
        }
        
        checked++;
    }
    
    return NULL;
}

// Swap out a page
int vm_swap_out_page(page_frame_t* frame) {
    if (!frame || !frame->owner) {
        return -1;
    }
    
    // Allocate swap space
    uint32_t swap_page = vm_allocate_swap_page();
    if (swap_page == UINT32_MAX) {
        return -1; // No swap space available
    }
    
    // Copy page to swap space
    uintptr_t swap_addr = swap_space.start_addr + (swap_page * PAGE_SIZE);
    memcpy((void*)swap_addr, (void*)frame->physical_addr, PAGE_SIZE);
    
    // Update page table entry
    vm_space_t* space = frame->owner;
    uintptr_t virtual_addr = frame->virtual_addr;
    
    // Mark page as swapped in page table
    // This would require page table manipulation
    // For now, just mark the frame as swapped
    
    frame->state = PAGE_STATE_SWAPPED;
    frame->flags |= PTE_SWAPPED;
    
    return 0;
}

// Swap in a page
int vm_swap_in_page(vm_space_t* space, uintptr_t addr) {
    if (!space) {
        return -1;
    }
    
    // Find the swap page number (this would be stored in page table)
    // For now, we'll assume it's stored somewhere
    
    // Allocate a new page frame
    page_frame_t* frame = vm_allocate_page_frame();
    if (!frame) {
        return -1;
    }
    
    // Copy from swap space
    // This would require knowing the swap page number
    // For now, just zero the page
    
    memset((void*)frame->physical_addr, 0, PAGE_SIZE);
    
    // Map the page
    if (vm_map_page(space, addr, frame->physical_addr, PTE_PRESENT | PTE_USER) != 0) {
        vm_free_page_frame(frame);
        return -1;
    }
    
    // Set up the page frame
    frame->owner = space;
    frame->virtual_addr = addr;
    frame->ref_count = 1;
    frame->state = PAGE_STATE_USED;
    frame->last_access = hal_get_timestamp();
    frame->flags = PTE_PRESENT | PTE_USER;
    
    return 0;
}

// Allocate swap page
uint32_t vm_allocate_swap_page(void) {
    spinlock_acquire(&swap_space.lock);
    
    if (swap_space.free_pages == 0) {
        spinlock_release(&swap_space.lock);
        return UINT32_MAX;
    }
    
    // Find free page in bitmap
    for (uint32_t i = 0; i < swap_space.total_pages; i++) {
        uint32_t word = i / 32;
        uint32_t bit = i % 32;
        
        if (!(swap_space.bitmap[word] & (1 << bit))) {
            // Mark as allocated
            swap_space.bitmap[word] |= (1 << bit);
            swap_space.free_pages--;
            
            spinlock_release(&swap_space.lock);
            return i;
        }
    }
    
    spinlock_release(&swap_space.lock);
    return UINT32_MAX;
}

// Free swap page
void vm_free_swap_page(uint32_t page) {
    if (page >= swap_space.total_pages) {
        return;
    }
    
    spinlock_acquire(&swap_space.lock);
    
    uint32_t word = page / 32;
    uint32_t bit = page % 32;
    
    // Clear bit
    swap_space.bitmap[word] &= ~(1 << bit);
    swap_space.free_pages++;
    
    spinlock_release(&swap_space.lock);
}

// Load page from file
int vm_load_page_from_file(vm_region_t* region, uintptr_t addr) {
    if (!region || !region->file_path) {
        return -1;
    }
    
    // This would involve reading from the file system
    // For now, just zero the page
    
    page_frame_t* frame = vm_find_page_frame(addr);
    if (frame) {
        memset((void*)frame->physical_addr, 0, PAGE_SIZE);
    }
    
    return 0;
}

// Find page frame by virtual address
page_frame_t* vm_find_page_frame(uintptr_t virtual_addr) {
    page_frame_t* frame = page_replacement.used_list;
    while (frame) {
        if (frame->virtual_addr == virtual_addr) {
            return frame;
        }
        frame = frame->next;
    }
    
    return NULL;
}

// Mark page as dirty
int vm_mark_page_dirty(vm_space_t* space, uintptr_t addr) {
    page_frame_t* frame = vm_find_page_frame(addr);
    if (frame) {
        frame->state = PAGE_STATE_DIRTY;
        frame->flags |= PTE_DIRTY;
        return 0;
    }
    
    return -1;
}

// Check if page is present
bool vm_is_page_present(vm_space_t* space, uintptr_t addr) {
    page_frame_t* frame = vm_find_page_frame(addr);
    return frame != NULL && frame->state == PAGE_STATE_USED;
}

// Copy-on-write fault handler
int vm_cow_fault(vm_space_t* space, uintptr_t addr) {
    if (!space) {
        return -1;
    }
    
    // Find the shared page
    page_frame_t* shared_frame = vm_find_page_frame(addr);
    if (!shared_frame || !(shared_frame->flags & PTE_COW)) {
        return -1;
    }
    
    // Allocate new page frame
    page_frame_t* new_frame = vm_allocate_page_frame();
    if (!new_frame) {
        new_frame = vm_swap_out_page_frame();
        if (!new_frame) {
            return -1;
        }
    }
    
    // Copy content
    memcpy((void*)new_frame->physical_addr, (void*)shared_frame->physical_addr, PAGE_SIZE);
    
    // Map new page
    if (vm_map_page(space, addr, new_frame->physical_addr, shared_frame->flags & ~PTE_COW) != 0) {
        vm_free_page_frame(new_frame);
        return -1;
    }
    
    // Set up new frame
    new_frame->owner = space;
    new_frame->virtual_addr = addr;
    new_frame->ref_count = 1;
    new_frame->state = PAGE_STATE_USED;
    new_frame->last_access = hal_get_timestamp();
    new_frame->flags = shared_frame->flags & ~PTE_COW;
    
    // Decrease reference count on shared frame
    shared_frame->ref_count--;
    if (shared_frame->ref_count == 0) {
        vm_free_page_frame(shared_frame);
    }
    
    return 0;
}

// Check if page is copy-on-write
bool vm_is_cow_page(vm_space_t* space, uintptr_t addr) {
    page_frame_t* frame = vm_find_page_frame(addr);
    return frame != NULL && (frame->flags & PTE_COW);
}

// Copy-on-write page
int vm_cow_copy_page(vm_space_t* space, uintptr_t addr) {
    page_frame_t* frame = vm_find_page_frame(addr);
    if (!frame) {
        return -1;
    }
    
    frame->flags |= PTE_COW;
    frame->state = PAGE_STATE_COW;
    
    return 0;
}

// Get memory statistics
void vm_get_stats(vm_stats_t* stats) {
    if (!stats) {
        return;
    }
    
    spinlock_acquire(&page_replacement.lock);
    
    stats->total_frames = page_replacement.total_frames;
    stats->free_frames = page_replacement.free_frames;
    stats->used_frames = page_replacement.total_frames - page_replacement.free_frames;
    stats->swap_total = swap_space.total_pages;
    stats->swap_free = swap_space.free_pages;
    stats->swap_used = swap_space.total_pages - swap_space.free_pages;
    
    spinlock_release(&page_replacement.lock);
} 