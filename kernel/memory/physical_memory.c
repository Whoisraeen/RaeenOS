#include "memory/include/memory.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

// Global physical memory allocator
static physical_allocator_t phys_allocator = {0};
static memory_region_t* memory_regions = NULL;
static bool phys_memory_initialized = false;

// Bitmap operations
static inline void bitmap_set_bit(u32* bitmap, u32 bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear_bit(u32* bitmap, u32 bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline bool bitmap_test_bit(u32* bitmap, u32 bit) {
    return (bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

static u32 bitmap_find_free_pages(u32* bitmap, u32 bitmap_size, u32 count) {
    if (count == 0) return UINT32_MAX;
    
    for (u32 i = 0; i <= bitmap_size - count; i++) {
        bool found = true;
        
        // Check if 'count' consecutive bits are free
        for (u32 j = 0; j < count; j++) {
            if (bitmap_test_bit(bitmap, i + j)) {
                found = false;
                i += j; // Skip ahead
                break;
            }
        }
        
        if (found) {
            return i;
        }
    }
    
    return UINT32_MAX; // Not found
}

error_t physical_memory_init(memory_region_t* regions) {
    if (phys_memory_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing physical memory allocator");
    
    memory_regions = regions;
    
    // Calculate total available memory
    u64 total_memory = 0;
    u64 available_memory = 0;
    u32 max_frame = 0;
    
    memory_region_t* region = regions;
    while (region) {
        total_memory += (region->end - region->start);
        
        if (region->type == MEMORY_TYPE_AVAILABLE) {
            available_memory += (region->end - region->start);
            u32 end_frame = (u32)(region->end / PAGE_SIZE);
            if (end_frame > max_frame) {
                max_frame = end_frame;
            }
        }
        
        KDEBUG("Memory region: 0x%llx-0x%llx, type=%d, size=%lluMB",
               region->start, region->end, region->type,
               (region->end - region->start) / (1024 * 1024));
        
        region = region->next;
    }
    
    KINFO("Total memory: %lluMB, Available: %lluMB",
          total_memory / (1024 * 1024),
          available_memory / (1024 * 1024));
    
    // Initialize allocator structures
    phys_allocator.total_frames = max_frame;
    phys_allocator.free_frames = (u32)(available_memory / PAGE_SIZE);
    phys_allocator.bitmap_size = (max_frame + 31) / 32; // Round up to 32-bit boundary
    
    // Allocate bitmap (mark all as used initially)
    size_t bitmap_bytes = phys_allocator.bitmap_size * sizeof(u32);
    phys_allocator.free_bitmap = (u32*)kernel_heap_alloc(bitmap_bytes, ALLOC_FLAG_ZERO);
    if (!phys_allocator.free_bitmap) {
        KERROR("Failed to allocate physical memory bitmap");
        return E_NOMEM;
    }
    
    // Set all bits to 1 (used) initially
    memset(phys_allocator.free_bitmap, 0xFF, bitmap_bytes);
    
    // Mark available regions as free in bitmap
    region = regions;
    while (region) {
        if (region->type == MEMORY_TYPE_AVAILABLE) {
            u32 start_frame = (u32)(region->start / PAGE_SIZE);
            u32 end_frame = (u32)(region->end / PAGE_SIZE);
            
            for (u32 frame = start_frame; frame < end_frame; frame++) {
                bitmap_clear_bit(phys_allocator.free_bitmap, frame);
            }
        }
        region = region->next;
    }
    
    phys_memory_initialized = true;
    
    KINFO("Physical memory allocator initialized: %u total frames, %u free frames",
          phys_allocator.total_frames, phys_allocator.free_frames);
    
    return SUCCESS;
}

phys_addr_t physical_alloc_page(void) {
    return physical_alloc_pages(1);
}

phys_addr_t physical_alloc_pages(u32 count) {
    if (!phys_memory_initialized || count == 0) {
        return 0;
    }

    static ga_population_t pop;
    nn_prediction_t pred = predict_memory_usage(current_process());

    init_ga_population(&pop, current_process(), pred);
    for(int gen = 0; gen < GA_MAX_GENERATIONS; gen++) {
        calculate_fitness(&pop, current_process()->memory_profile);
        perform_crossover(&pop);
        mutate_population(&pop);
    }

    page_frame_t *best = select_best_frame(&pop);
    best->allocation_metadata |= AI_OPTIMIZED;

    // Mark pages as used
    for (u32 i = 0; i < count; i++) {
        bitmap_set_bit(phys_allocator.free_bitmap, best->frame_number + i);
    }

    phys_allocator.free_frames -= count;
    phys_addr_t addr = (phys_addr_t)best->frame_number * PAGE_SIZE;

    KDEBUG("AI-optimized allocation of %u pages at 0x%llx", count, addr);
    return addr;
}

void physical_free_page(phys_addr_t addr) {
    physical_free_pages(addr, 1);
}

void physical_free_pages(phys_addr_t addr, u32 count) {
    if (!phys_memory_initialized || addr == 0 || count == 0) {
        return;
    }
    
    if (addr % PAGE_SIZE != 0) {
        KERROR("Attempt to free unaligned physical address: 0x%llx", addr);
        return;
    }
    
    u32 start_frame = (u32)(addr / PAGE_SIZE);
    
    if (start_frame >= phys_allocator.total_frames) {
        KERROR("Attempt to free invalid physical address: 0x%llx", addr);
        return;
    }
    
    // Verify pages are actually allocated
    for (u32 i = 0; i < count; i++) {
        if (!bitmap_test_bit(phys_allocator.free_bitmap, start_frame + i)) {
            KERROR("Double free detected at physical address: 0x%llx",
                   addr + i * PAGE_SIZE);
            return;
        }
    }
    
    // Mark pages as free
    for (u32 i = 0; i < count; i++) {
        bitmap_clear_bit(phys_allocator.free_bitmap, start_frame + i);
    }
    
    phys_allocator.free_frames += count;
    
    KDEBUG("Freed %u physical pages at 0x%llx", count, addr);
}

bool physical_is_available(phys_addr_t addr) {
    if (!phys_memory_initialized || addr % PAGE_SIZE != 0) {
        return false;
    }
    
    u32 frame = (u32)(addr / PAGE_SIZE);
    
    if (frame >= phys_allocator.total_frames) {
        return false;
    }
    
    return !bitmap_test_bit(phys_allocator.free_bitmap, frame);
}

// Memory region management
static void add_memory_region(phys_addr_t start, phys_addr_t end, memory_type_t type) {
    memory_region_t* region = (memory_region_t*)kernel_heap_alloc(sizeof(memory_region_t), 0);
    if (!region) {
        KERROR("Failed to allocate memory region structure");
        return;
    }
    
    region->start = start;
    region->end = end;
    region->type = type;
    region->next = memory_regions;
    memory_regions = region;
}

void memory_mark_region_used(phys_addr_t start, phys_addr_t end, memory_type_t type) {
    if (!phys_memory_initialized) {
        return;
    }
    
    // Align to page boundaries
    start = PAGE_ALIGN_DOWN(start);
    end = PAGE_ALIGN_UP(end);
    
    u32 start_frame = (u32)(start / PAGE_SIZE);
    u32 end_frame = (u32)(end / PAGE_SIZE);
    
    KDEBUG("Marking physical region 0x%llx-0x%llx as used (type %d)",
           start, end, type);
    
    for (u32 frame = start_frame; frame < end_frame; frame++) {
        if (frame < phys_allocator.total_frames) {
            if (!bitmap_test_bit(phys_allocator.free_bitmap, frame)) {
                bitmap_set_bit(phys_allocator.free_bitmap, frame);
                phys_allocator.free_frames--;
            }
        }
    }
    
    // Add to region list for tracking
    add_memory_region(start, end, type);
}

// Get memory statistics
u64 physical_get_total_memory(void) {
    return (u64)phys_allocator.total_frames * PAGE_SIZE;
}

u64 physical_get_free_memory(void) {
    return (u64)phys_allocator.free_frames * PAGE_SIZE;
}

u64 physical_get_used_memory(void) {
    return (u64)(phys_allocator.total_frames - phys_allocator.free_frames) * PAGE_SIZE;
}

// Debug functions
void physical_memory_dump_stats(void) {
    u64 total_mb = physical_get_total_memory() / (1024 * 1024);
    u64 free_mb = physical_get_free_memory() / (1024 * 1024);
    u64 used_mb = physical_get_used_memory() / (1024 * 1024);
    
    hal_console_print("Physical Memory Statistics:\n");
    hal_console_print("  Total: %llu MB (%u frames)\n", total_mb, phys_allocator.total_frames);
    hal_console_print("  Free:  %llu MB (%u frames)\n", free_mb, phys_allocator.free_frames);
    hal_console_print("  Used:  %llu MB (%u frames)\n", used_mb, 
                     phys_allocator.total_frames - phys_allocator.free_frames);
    hal_console_print("  Usage: %u%%\n", 
                     (u32)((used_mb * 100) / total_mb));
}

void physical_memory_dump_regions(void) {
    hal_console_print("Memory Regions:\n");
    
    memory_region_t* region = memory_regions;
    while (region) {
        const char* type_str;
        switch (region->type) {
            case MEMORY_TYPE_AVAILABLE:      type_str = "Available"; break;
            case MEMORY_TYPE_RESERVED:       type_str = "Reserved"; break;
            case MEMORY_TYPE_ACPI_RECLAIMABLE: type_str = "ACPI Reclaimable"; break;
            case MEMORY_TYPE_ACPI_NVS:       type_str = "ACPI NVS"; break;
            case MEMORY_TYPE_BAD:            type_str = "Bad"; break;
            case MEMORY_TYPE_KERNEL:         type_str = "Kernel"; break;
            case MEMORY_TYPE_INITRD:         type_str = "InitRD"; break;
            case MEMORY_TYPE_FRAMEBUFFER:    type_str = "Framebuffer"; break;
            default:                         type_str = "Unknown"; break;
        }
        
        hal_console_print("  0x%016llx - 0x%016llx: %s (%llu MB)\n",
                         region->start, region->end, type_str,
                         (region->end - region->start) / (1024 * 1024));
        
        region = region->next;
    }
}