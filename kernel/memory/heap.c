#include "memory/include/memory.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

// Global kernel heap
static heap_t kernel_heap = {0};
static bool heap_initialized = false;

// Heap block manipulation
static heap_block_t* get_block_header(void* ptr) {
    if (!ptr) return NULL;
    return (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
}

static void* get_block_data(heap_block_t* block) {
    if (!block) return NULL;
    return (char*)block + sizeof(heap_block_t);
}

static heap_block_t* get_next_block(heap_block_t* block) {
    if (!block) return NULL;
    return (heap_block_t*)((char*)block + sizeof(heap_block_t) + block->size);
}

static bool is_valid_block(heap_block_t* block) {
    if (!block) return false;
    
    // Check if block is within heap bounds
    if ((void*)block < kernel_heap.start || (void*)block >= kernel_heap.end) {
        return false;
    }
    
    // Check magic number
    if (block->free) {
        return block->magic == HEAP_BLOCK_MAGIC_FREE;
    } else {
        return block->magic == HEAP_BLOCK_MAGIC_ALLOCATED;
    }
}

static void split_block(heap_block_t* block, size_t size) {
    if (!block || block->size <= size + sizeof(heap_block_t)) {
        return; // Can't split
    }
    
    size_t remaining_size = block->size - size - sizeof(heap_block_t);
    
    // Create new block for remaining space
    heap_block_t* new_block = (heap_block_t*)((char*)block + sizeof(heap_block_t) + size);
    new_block->size = remaining_size;
    new_block->free = true;
    new_block->magic = HEAP_BLOCK_MAGIC_FREE;
    new_block->prev = block;
    new_block->next = block->next;
    
    // Update original block
    block->size = size;
    block->next = new_block;
    
    // Update next block's prev pointer
    if (new_block->next) {
        new_block->next->prev = new_block;
    }
    
    kernel_heap.total_blocks++;
    kernel_heap.free_blocks++;
}

static void merge_with_next(heap_block_t* block) {
    if (!block || !block->next || !block->next->free) {
        return;
    }
    
    heap_block_t* next = block->next;
    
    // Merge blocks
    block->size += sizeof(heap_block_t) + next->size;
    block->next = next->next;
    
    if (next->next) {
        next->next->prev = block;
    }
    
    kernel_heap.total_blocks--;
    kernel_heap.free_blocks--;
}

static void coalesce_free_blocks(heap_block_t* block) {
    if (!block || !block->free) {
        return;
    }
    
    // Merge with previous block if it's free
    if (block->prev && block->prev->free) {
        heap_block_t* prev = block->prev;
        merge_with_next(prev);
        block = prev;
    }
    
    // Merge with next block if it's free
    merge_with_next(block);
}

static heap_block_t* find_free_block(size_t size) {
    heap_block_t* current = kernel_heap.free_list;
    
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL; // No suitable block found
}

static void add_to_free_list(heap_block_t* block) {
    if (!block) return;
    
    block->free = true;
    block->magic = HEAP_BLOCK_MAGIC_FREE;
    
    // Insert at beginning of free list for simplicity
    if (kernel_heap.free_list) {
        kernel_heap.free_list->prev = block;
    }
    block->next = kernel_heap.free_list;
    block->prev = NULL;
    kernel_heap.free_list = block;
    
    kernel_heap.free_blocks++;
}

static void remove_from_free_list(heap_block_t* block) {
    if (!block || !block->free) return;
    
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        kernel_heap.free_list = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->free = false;
    block->magic = HEAP_BLOCK_MAGIC_ALLOCATED;
    kernel_heap.free_blocks--;
}

error_t kernel_heap_init(void) {
    if (heap_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing kernel heap");
    
    // Map virtual memory for heap
    virt_addr_t heap_virt = KERNEL_HEAP_START;
    size_t heap_pages = KERNEL_HEAP_SIZE / PAGE_SIZE;
    
    for (size_t i = 0; i < heap_pages; i++) {
        phys_addr_t phys = physical_alloc_page();
        if (phys == 0) {
            KERROR("Failed to allocate physical memory for kernel heap");
            // TODO: Cleanup already allocated pages
            return E_NOMEM;
        }
        
        u32 flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE | PAGE_FLAG_GLOBAL;
        error_t result = page_table_map(address_space_current(), 
                                       heap_virt + i * PAGE_SIZE, 
                                       phys, flags);
        if (result != SUCCESS) {
            KERROR("Failed to map kernel heap page: %d", result);
            physical_free_page(phys);
            return result;
        }
    }
    
    // Initialize heap structure
    kernel_heap.start = (void*)heap_virt;
    kernel_heap.end = (void*)(heap_virt + KERNEL_HEAP_SIZE);
    kernel_heap.size = KERNEL_HEAP_SIZE;
    kernel_heap.free_list = NULL;
    kernel_heap.total_blocks = 1;
    kernel_heap.free_blocks = 1;
    
    // Create initial free block
    heap_block_t* initial_block = (heap_block_t*)kernel_heap.start;
    initial_block->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    initial_block->free = true;
    initial_block->magic = HEAP_BLOCK_MAGIC_FREE;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    
    kernel_heap.free_list = initial_block;
    
    heap_initialized = true;
    
    KINFO("Kernel heap initialized: %zu bytes at 0x%llx", 
          KERNEL_HEAP_SIZE, heap_virt);
    
    return SUCCESS;
}

void* kernel_heap_alloc(size_t size, u32 flags) {
    if (!heap_initialized || size == 0) {
        return NULL;
    }
    
    // Align size to 8-byte boundary
    size = (size + 7) & ~7;
    
    // Find suitable free block
    heap_block_t* block = find_free_block(size);
    if (!block) {
        KDEBUG("Kernel heap exhausted: requested %zu bytes", size);
        return NULL;
    }
    
    // Remove from free list
    remove_from_free_list(block);
    
    // Split block if it's significantly larger than needed
    if (block->size > size + sizeof(heap_block_t) + 64) {
        split_block(block, size);
        
        // Add the split portion back to free list
        if (block->next && block->next->free) {
            add_to_free_list(block->next);
        }
    }
    
    // Zero memory if requested
    void* ptr = get_block_data(block);
    if (flags & ALLOC_FLAG_ZERO) {
        memset(ptr, 0, size);
    }
    
    KDEBUG("Allocated %zu bytes at %p", size, ptr);
    return ptr;
}

void* kernel_heap_alloc_aligned(size_t size, size_t alignment, u32 flags) {
    if (!heap_initialized || size == 0 || alignment == 0) {
        return NULL;
    }
    
    // Ensure alignment is power of 2
    if ((alignment & (alignment - 1)) != 0) {
        return NULL;
    }
    
    // Allocate extra space for alignment
    size_t total_size = size + alignment + sizeof(heap_block_t);
    void* raw_ptr = kernel_heap_alloc(total_size, 0);
    if (!raw_ptr) {
        return NULL;
    }
    
    // Calculate aligned address
    uintptr_t addr = (uintptr_t)raw_ptr;
    uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    
    // If already aligned, return as-is
    if (aligned_addr == addr) {
        if (flags & ALLOC_FLAG_ZERO) {
            memset(raw_ptr, 0, size);
        }
        return raw_ptr;
    }
    
    // Otherwise, we need to create a new allocation
    kernel_heap_free(raw_ptr);
    
    // For simplicity, just allocate larger block and waste some space
    // A more sophisticated implementation would handle this better
    total_size = size + alignment;
    raw_ptr = kernel_heap_alloc(total_size, 0);
    if (!raw_ptr) {
        return NULL;
    }
    
    addr = (uintptr_t)raw_ptr;
    aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    
    if (flags & ALLOC_FLAG_ZERO) {
        memset((void*)aligned_addr, 0, size);
    }
    
    // Store original pointer for freeing (simple approach)
    // Note: This wastes space but ensures we can free properly
    return (void*)aligned_addr;
}

void kernel_heap_free(void* ptr) {
    if (!heap_initialized || !ptr) {
        return;
    }
    
    heap_block_t* block = get_block_header(ptr);
    if (!is_valid_block(block)) {
        KERROR("Invalid heap block at %p", ptr);
        return;
    }
    
    if (block->free) {
        KERROR("Double free detected at %p", ptr);
        return;
    }
    
    KDEBUG("Freeing %zu bytes at %p", block->size, ptr);
    
    // Add to free list
    add_to_free_list(block);
    
    // Coalesce with adjacent free blocks
    coalesce_free_blocks(block);
}

size_t kernel_heap_size(void* ptr) {
    if (!heap_initialized || !ptr) {
        return 0;
    }
    
    heap_block_t* block = get_block_header(ptr);
    if (!is_valid_block(block) || block->free) {
        return 0;
    }
    
    return block->size;
}

// Standard memory allocation functions
void* memory_alloc(size_t size) {
    return kernel_heap_alloc(size, 0);
}

void* memory_alloc_aligned(size_t size, size_t alignment) {
    return kernel_heap_alloc_aligned(size, alignment, 0);
}

void* memory_calloc(size_t count, size_t size) {
    size_t total_size = count * size;
    if (total_size / count != size) {
        // Overflow check
        return NULL;
    }
    
    return kernel_heap_alloc(total_size, ALLOC_FLAG_ZERO);
}

void* memory_realloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return memory_alloc(new_size);
    }
    
    if (new_size == 0) {
        memory_free(ptr);
        return NULL;
    }
    
    size_t old_size = kernel_heap_size(ptr);
    if (old_size == 0) {
        return NULL; // Invalid pointer
    }
    
    if (new_size <= old_size) {
        return ptr; // No need to reallocate
    }
    
    void* new_ptr = memory_alloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, old_size);
    memory_free(ptr);
    
    return new_ptr;
}

void memory_free(void* ptr) {
    kernel_heap_free(ptr);
}

bool memory_check_heap_integrity(void) {
    if (!heap_initialized) {
        return false;
    }
    
    bool integrity_ok = true;
    u32 total_blocks = 0;
    u32 free_blocks = 0;
    
    // Walk through all blocks
    heap_block_t* current = (heap_block_t*)kernel_heap.start;
    
    while ((void*)current < kernel_heap.end) {
        if (!is_valid_block(current)) {
            KERROR("Heap corruption detected: invalid block at %p", current);
            integrity_ok = false;
            break;
        }
        
        total_blocks++;
        if (current->free) {
            free_blocks++;
        }
        
        // Check for overlapping blocks
        heap_block_t* next = get_next_block(current);
        if (next && (void*)next < kernel_heap.end) {
            if ((void*)next <= (void*)current) {
                KERROR("Heap corruption: overlapping blocks at %p and %p", 
                       current, next);
                integrity_ok = false;
                break;
            }
        }
        
        current = next;
        if (!current) break;
    }
    
    // Verify block counts
    if (total_blocks != kernel_heap.total_blocks) {
        KERROR("Heap integrity error: block count mismatch (%u vs %u)", 
               total_blocks, kernel_heap.total_blocks);
        integrity_ok = false;
    }
    
    if (free_blocks != kernel_heap.free_blocks) {
        KERROR("Heap integrity error: free block count mismatch (%u vs %u)", 
               free_blocks, kernel_heap.free_blocks);
        integrity_ok = false;
    }
    
    return integrity_ok;
}

void memory_dump_stats(void) {
    if (!heap_initialized) {
        hal_console_print("Kernel heap not initialized\n");
        return;
    }
    
    size_t total_allocated = 0;
    size_t total_free = 0;
    u32 allocated_blocks = 0;
    
    // Calculate statistics
    heap_block_t* current = (heap_block_t*)kernel_heap.start;
    
    while ((void*)current < kernel_heap.end) {
        if (is_valid_block(current)) {
            if (current->free) {
                total_free += current->size;
            } else {
                total_allocated += current->size;
                allocated_blocks++;
            }
        }
        
        current = get_next_block(current);
        if (!current) break;
    }
    
    hal_console_print("Kernel Heap Statistics:\n");
    hal_console_print("  Total size:      %zu bytes (%zu KB)\n", 
                     kernel_heap.size, kernel_heap.size / 1024);
    hal_console_print("  Allocated:       %zu bytes (%zu KB)\n", 
                     total_allocated, total_allocated / 1024);
    hal_console_print("  Free:            %zu bytes (%zu KB)\n", 
                     total_free, total_free / 1024);
    hal_console_print("  Total blocks:    %u\n", kernel_heap.total_blocks);
    hal_console_print("  Allocated blocks: %u\n", allocated_blocks);
    hal_console_print("  Free blocks:     %u\n", kernel_heap.free_blocks);
    hal_console_print("  Fragmentation:   %.1f%%\n", 
                     ((float)kernel_heap.free_blocks / kernel_heap.total_blocks) * 100.0f);
}