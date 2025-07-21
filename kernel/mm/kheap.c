#include "memory/kheap.h"
#include "kernel.h"
#include <string.h>

typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block_t;

static free_block_t* free_list_head = NULL;
static virt_addr_t heap_start;
static size_t heap_size;

void kheap_init(virt_addr_t start, size_t size) {
    heap_start = start;
    heap_size = size;

    // Initially, the entire heap is one large free block
    free_list_head = (free_block_t*)heap_start;
    free_list_head->size = heap_size;
    free_list_head->next = NULL;
}

void* kmalloc(size_t size) {
    // Align size to 16 bytes and add header size
    size = (size + sizeof(size_t) + 15) & ~15;

    free_block_t* current = free_list_head;
    free_block_t* prev = NULL;

    while (current) {
        if (current->size >= size) {
            // Found a suitable block
            if (current->size > size + sizeof(free_block_t)) {
                // Split the block
                free_block_t* new_free_block = (free_block_t*)((u8*)current + size);
                new_free_block->size = current->size - size;
                new_free_block->next = current->next;

                if (prev) {
                    prev->next = new_free_block;
                } else {
                    free_list_head = new_free_block;
                }

                // Set up the allocated block header (just the size)
                *(size_t*)current = size;
            } else {
                // Use the whole block
                if (prev) {
                    prev->next = current->next;
                } else {
                    free_list_head = current->next;
                }
            }
            // Return a pointer to the usable memory area (after the size header)
            return (void*)((u8*)current + sizeof(size_t));
        }
        prev = current;
        current = current->next;
    }

    // Out of memory
    KWARN("kmalloc: out of memory!");
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;

    // Get a pointer to the block header
    free_block_t* block = (free_block_t*)((u8*)ptr - sizeof(size_t));

    // Find the correct place to insert the freed block to keep the list sorted by address
    free_block_t* current = free_list_head;
    free_block_t* prev = NULL;
    while (current && current < block) {
        prev = current;
        current = current->next;
    }

    // Insert into free list
    if (prev) {
        prev->next = block;
    } else {
        free_list_head = block;
    }
    block->next = current;

    // Coalesce with next block if possible
    if (current && (u8*)block + block->size == (u8*)current) {
        block->size += current->size;
        block->next = current->next;
    }

    // Coalesce with previous block if possible
    if (prev && (u8*)prev + prev->size == (u8*)block) {
        prev->size += block->size;
        prev->next = block->next;
    }
}