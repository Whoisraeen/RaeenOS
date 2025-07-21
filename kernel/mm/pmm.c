#include "memory/pmm.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

static u8* pmm_bitmap = NULL;
static u64 pmm_total_pages = 0;
static u64 pmm_last_alloc_page = 0;

// Helper to set a bit in the bitmap
static void pmm_set_bit(u64 page_index) {
    pmm_bitmap[page_index / 8] |= (1 << (page_index % 8));
}

// Helper to clear a bit in the bitmap
static void pmm_clear_bit(u64 page_index) {
    pmm_bitmap[page_index / 8] &= ~(1 << (page_index % 8));
}

// Helper to test a bit in the bitmap
static bool pmm_test_bit(u64 page_index) {
    return (pmm_bitmap[page_index / 8] & (1 << (page_index % 8))) != 0;
}

error_t pmm_init(struct multiboot_info* mbi) {
    if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
        return E_NOT_SUPPORTED;
    }

    // 1. Find the total amount of memory to determine bitmap size
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    phys_addr_t highest_addr = 0;

    while ((u32)mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            phys_addr_t top = mmap->addr + mmap->len;
            if (top > highest_addr) {
                highest_addr = top;
            }
        }
        mmap = (multiboot_memory_map_t*)((u32)mmap + mmap->size + sizeof(mmap->size));
    }

    pmm_total_pages = highest_addr / PAGE_SIZE;
    u64 bitmap_size = pmm_total_pages / 8;

    // 2. Find a place to store the bitmap
    mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    while ((u32)mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->len >= bitmap_size) {
            // Use the first available region (we assume it's after the kernel)
            pmm_bitmap = (u8*)(u32)mmap->addr;
            if ((u32)pmm_bitmap < 0x100000) pmm_bitmap = (u8*)0x100000; // Must be > 1MB
            break;
        }
        mmap = (multiboot_memory_map_t*)((u32)mmap + mmap->size + sizeof(mmap->size));
    }

    if (pmm_bitmap == NULL) {
        return E_NO_MEMORY;
    }

    // 3. Initialize bitmap: mark all memory as used initially
    memset(pmm_bitmap, 0xFF, bitmap_size);

    // 4. Mark available memory regions as free
    mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    while ((u32)mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (u64 i = 0; i < mmap->len; i += PAGE_SIZE) {
                pmm_clear_bit((mmap->addr + i) / PAGE_SIZE);
            }
        }
        mmap = (multiboot_memory_map_t*)((u32)mmap + mmap->size + sizeof(mmap->size));
    }

    // 5. Mark the bitmap itself as used
    for (u64 i = 0; i < (bitmap_size + PAGE_SIZE -1) / PAGE_SIZE; i++) {
        pmm_set_bit(((u32)pmm_bitmap / PAGE_SIZE) + i);
    }
    
    // 6. Mark first MB as used (BIOS, VGA, etc.)
    for (u64 i = 0; i < 0x100000 / PAGE_SIZE; i++) {
        pmm_set_bit(i);
    }

    return SUCCESS;
}

phys_addr_t pmm_alloc_page(void) {
    for (u64 i = pmm_last_alloc_page; i < pmm_total_pages; i++) {
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            pmm_last_alloc_page = i + 1;
            return i * PAGE_SIZE;
        }
    }
    // TODO: loop back from 0 if needed
    return 0; // Out of memory
}

void pmm_free_page(phys_addr_t page) {
    pmm_clear_bit(page / PAGE_SIZE);
}