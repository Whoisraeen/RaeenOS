#include "memory/memory.h"
#include "memory/pmm.h"
#include "memory/kheap.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

#define KERNEL_HEAP_VIRT_START 0xFFFFFFFFC0000000ULL // Virtual address for the start of the kernel heap
#define KERNEL_BASE      0xFFFFFFFF80000000ULL // Example kernel virtual base
#define KERNEL_HEAP_SIZE 0x1000000  // 16MB

static phys_addr_t pml4_phys;
static virt_addr_t kernel_heap_start;
static virt_addr_t kernel_heap_end;

// Extern kernel start/end symbols from linker script
extern u8 _kernel_start[], _kernel_end[];

static void memory_map_page(u64* pml4, virt_addr_t virt, phys_addr_t phys, u64 flags);

error_t memory_init(void) {
    KINFO("Initializing Memory Management...");

    // 1. Create a new kernel page map (PML4)
    phys_addr_t kernel_pml4_phys = pmm_alloc_page();
    if (!kernel_pml4_phys) {
        KERNEL_PANIC("Failed to allocate PML4");
    }
    u64* pml4 = (u64*)kernel_pml4_phys; // We can access this directly for now
    memset(pml4, 0, PAGE_SIZE);

    // 2. Map the kernel's code and data into the higher half
    phys_addr_t kernel_phys_start = (phys_addr_t)_kernel_start;
    phys_addr_t kernel_phys_end = (phys_addr_t)_kernel_end;
    for (phys_addr_t p = kernel_phys_start; p < kernel_phys_end; p += PAGE_SIZE) {
        virt_addr_t v = p + KERNEL_BASE;
        memory_map_page(pml4, v, p, PAGE_PRESENT | PAGE_WRITE);
    }

    // 3. Map the kernel heap
    kernel_heap_start = KERNEL_HEAP_VIRT_START;
    kernel_heap_end = kernel_heap_start + KERNEL_HEAP_SIZE;
    for (virt_addr_t v = kernel_heap_start; v < kernel_heap_end; v += PAGE_SIZE) {
        phys_addr_t p = pmm_alloc_page();
        if (!p) KERNEL_PANIC("Out of memory for kernel heap");
        memory_map_page(pml4, v, p, PAGE_PRESENT | PAGE_WRITE);
    }

    // 4. Identity map first 4GB for easy access to BIOS/VGA/etc. during boot
    // A real OS would remove this after boot.
    for (phys_addr_t p = 0; p < 0x100000000; p += 0x200000) { // Using 2MB pages
         memory_map_page(pml4, p, p, PAGE_PRESENT | PAGE_WRITE | PAGE_LARGE);
    }

    // 5. Load the new page map
    pml4_phys = kernel_pml4_phys;
    hal_set_page_directory(pml4_phys);
    
    KINFO("Paging enabled, kernel mapped to 0x%llx", KERNEL_BASE);

    // 6. Initialize the kernel heap allocator in its new virtual address space
    kheap_init(kernel_heap_start, KERNEL_HEAP_SIZE);
    KINFO("Kernel heap initialized at 0x%llx (size %uMB)", kernel_heap_start, KERNEL_HEAP_SIZE / 1024 / 1024);

    return SUCCESS;
}

void* memory_alloc(size_t size) {
    return kmalloc(size);
}

void memory_free(void* ptr) {
    kfree(ptr);
}

static void memory_map_page(u64* pml4_virt, virt_addr_t virt, phys_addr_t phys, u64 flags) {
    u64 pml4_index = (virt >> 39) & 0x1FF;
    u64 pdpt_index = (virt >> 30) & 0x1FF;
    u64 pd_index = (virt >> 21) & 0x1FF;
    u64 pt_index = (virt >> 12) & 0x1FF;

    u64* pdpt_virt;
    if (pml4_virt[pml4_index] & PAGE_PRESENT) {
        pdpt_virt = (u64*)(pml4_virt[pml4_index] & ~0xFFF);
    } else {
        phys_addr_t pdpt_phys = pmm_alloc_page();
        if (!pdpt_phys) KERNEL_PANIC("OOM mapping page");
        pdpt_virt = (u64*)pdpt_phys;
        memset(pdpt_virt, 0, PAGE_SIZE);
        pml4_virt[pml4_index] = pdpt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    if (flags & PAGE_LARGE) {
        // 2MB large page mapping in PD
        // TODO: Implement this logic if needed
        return;
    }

    u64* pd_virt;
    if (pdpt_virt[pdpt_index] & PAGE_PRESENT) {
        pd_virt = (u64*)(pdpt_virt[pdpt_index] & ~0xFFF);
    } else {
        phys_addr_t pd_phys = pmm_alloc_page();
        if (!pd_phys) KERNEL_PANIC("OOM mapping page");
        pd_virt = (u64*)pd_phys;
        memset(pd_virt, 0, PAGE_SIZE);
        pdpt_virt[pdpt_index] = pd_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    u64* pt_virt;
    if (pd_virt[pd_index] & PAGE_PRESENT) {
        pt_virt = (u64*)(pd_virt[pd_index] & ~0xFFF);
    } else {
        phys_addr_t pt_phys = pmm_alloc_page();
        if (!pt_phys) KERNEL_PANIC("OOM mapping page");
        pt_virt = (u64*)pt_phys;
        memset(pt_virt, 0, PAGE_SIZE);
        pd_virt[pd_index] = pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    pt_virt[pt_index] = phys | flags;
}