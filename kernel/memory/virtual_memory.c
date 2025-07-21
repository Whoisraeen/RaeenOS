#include "memory/include/memory.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

// Global virtual memory state
static address_space_t* kernel_address_space = NULL;
static address_space_t* current_address_space = NULL;
static bool virtual_memory_initialized = false;

// Page table management
static page_table_t* create_page_table(void) {
    phys_addr_t phys = physical_alloc_page();
    if (phys == 0) {
        return NULL;
    }
    
    // Map the page table into kernel space for initialization
    page_table_t* table = (page_table_t*)PHYS_TO_VIRT(phys);
    memset(table, 0, PAGE_SIZE);
    
    return table;
}

static void destroy_page_table(page_table_t* table) {
    if (!table) return;
    
    phys_addr_t phys = VIRT_TO_PHYS(table);
    physical_free_page(phys);
}

static u64* get_page_entry(address_space_t* as, virt_addr_t virt_addr, bool create) {
    if (!as || !as->page_directory) {
        return NULL;
    }
    
    // Extract page table indices
    u32 pml4_idx = (virt_addr >> 39) & 0x1FF;
    u32 pdpt_idx = (virt_addr >> 30) & 0x1FF;
    u32 pd_idx   = (virt_addr >> 21) & 0x1FF;
    u32 pt_idx   = (virt_addr >> 12) & 0x1FF;
    
    page_table_t* pml4 = as->page_directory;
    
    // Walk PML4 -> PDPT
    if (!(pml4->entries[pml4_idx] & PAGE_FLAG_PRESENT)) {
        if (!create) return NULL;
        
        page_table_t* pdpt = create_page_table();
        if (!pdpt) return NULL;
        
        pml4->entries[pml4_idx] = VIRT_TO_PHYS(pdpt) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    
    page_table_t* pdpt = (page_table_t*)PHYS_TO_VIRT(pml4->entries[pml4_idx] & ~0xFFF);
    
    // Walk PDPT -> PD
    if (!(pdpt->entries[pdpt_idx] & PAGE_FLAG_PRESENT)) {
        if (!create) return NULL;
        
        page_table_t* pd = create_page_table();
        if (!pd) return NULL;
        
        pdpt->entries[pdpt_idx] = VIRT_TO_PHYS(pd) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    
    page_table_t* pd = (page_table_t*)PHYS_TO_VIRT(pdpt->entries[pdpt_idx] & ~0xFFF);
    
    // Walk PD -> PT
    if (!(pd->entries[pd_idx] & PAGE_FLAG_PRESENT)) {
        if (!create) return NULL;
        
        page_table_t* pt = create_page_table();
        if (!pt) return NULL;
        
        pd->entries[pd_idx] = VIRT_TO_PHYS(pt) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE;
    }
    
    page_table_t* pt = (page_table_t*)PHYS_TO_VIRT(pd->entries[pd_idx] & ~0xFFF);
    
    return &pt->entries[pt_idx];
}

error_t virtual_memory_init(void) {
    if (virtual_memory_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing virtual memory management");
    
    // Create kernel address space
    kernel_address_space = address_space_create();
    if (!kernel_address_space) {
        KERROR("Failed to create kernel address space");
        return E_NOMEM;
    }
    
    // Map kernel space (identity mapping for now)
    // TODO: Map kernel properly with correct permissions
    
    current_address_space = kernel_address_space;
    virtual_memory_initialized = true;
    
    KINFO("Virtual memory management initialized");
    return SUCCESS;
}

address_space_t* address_space_create(void) {
    address_space_t* as = (address_space_t*)kernel_heap_alloc(sizeof(address_space_t), ALLOC_FLAG_ZERO);
    if (!as) {
        return NULL;
    }
    
    // Create page directory (PML4)
    as->page_directory = create_page_table();
    if (!as->page_directory) {
        kernel_heap_free(as);
        return NULL;
    }
    
    as->ref_count = 1;
    as->vma_list = NULL;
    as->heap_start = USER_SPACE_START;
    as->heap_end = USER_SPACE_START;
    as->stack_start = USER_SPACE_END - PAGE_SIZE; // Start stack at top
    as->stack_end = USER_SPACE_END;
    
    // Copy kernel mappings if this is not the kernel address space
    if (kernel_address_space && as != kernel_address_space) {
        // Copy upper half (kernel space) mappings
        for (u32 i = 256; i < 512; i++) {
            as->page_directory->entries[i] = kernel_address_space->page_directory->entries[i];
        }
    }
    
    KDEBUG("Created address space at %p", as);
    return as;
}

void address_space_destroy(address_space_t* as) {
    if (!as) return;
    
    as->ref_count--;
    if (as->ref_count > 0) {
        return;
    }
    
    KDEBUG("Destroying address space at %p", as);
    
    // Free all VMAs
    vma_t* vma = as->vma_list;
    while (vma) {
        vma_t* next = vma->next;
        vma_destroy(vma);
        vma = next;
    }
    
    // Free page tables (but preserve kernel mappings)
    if (as != kernel_address_space) {
        // TODO: Free user-space page tables
    }
    
    destroy_page_table(as->page_directory);
    kernel_heap_free(as);
}

error_t address_space_switch(address_space_t* as) {
    if (!as || !as->page_directory) {
        return E_INVAL;
    }
    
    current_address_space = as;
    
    // Load CR3 with physical address of page directory
    phys_addr_t pml4_phys = VIRT_TO_PHYS(as->page_directory);
    hal_set_page_directory(pml4_phys);
    
    return SUCCESS;
}

address_space_t* address_space_current(void) {
    return current_address_space;
}

error_t page_table_map(address_space_t* as, virt_addr_t virt, phys_addr_t phys, u32 flags) {
    if (!as || virt % PAGE_SIZE != 0 || phys % PAGE_SIZE != 0) {
        return E_INVAL;
    }
    
    u64* entry = get_page_entry(as, virt, true);
    if (!entry) {
        return E_NOMEM;
    }
    
    if (*entry & PAGE_FLAG_PRESENT) {
        KWARN("Attempting to map already mapped page: 0x%llx", virt);
        return E_MEMORY_ALREADY_MAPPED;
    }
    
    *entry = phys | flags;
    
    // Flush TLB for this page
    memory_flush_tlb_page(virt);
    
    KDEBUG("Mapped virtual 0x%llx to physical 0x%llx with flags 0x%x", virt, phys, flags);
    return SUCCESS;
}

error_t page_table_unmap(address_space_t* as, virt_addr_t virt) {
    if (!as || virt % PAGE_SIZE != 0) {
        return E_INVAL;
    }
    
    u64* entry = get_page_entry(as, virt, false);
    if (!entry || !(*entry & PAGE_FLAG_PRESENT)) {
        return E_MEMORY_NOT_MAPPED;
    }
    
    *entry = 0;
    
    // Flush TLB for this page
    memory_flush_tlb_page(virt);
    
    KDEBUG("Unmapped virtual address 0x%llx", virt);
    return SUCCESS;
}

phys_addr_t page_table_get_physical(address_space_t* as, virt_addr_t virt) {
    if (!as) {
        return 0;
    }
    
    u64* entry = get_page_entry(as, virt, false);
    if (!entry || !(*entry & PAGE_FLAG_PRESENT)) {
        return 0;
    }
    
    return (*entry & ~0xFFF) | (virt & 0xFFF);
}

error_t page_table_set_flags(address_space_t* as, virt_addr_t virt, u32 flags) {
    if (!as || virt % PAGE_SIZE != 0) {
        return E_INVAL;
    }
    
    u64* entry = get_page_entry(as, virt, false);
    if (!entry || !(*entry & PAGE_FLAG_PRESENT)) {
        return E_MEMORY_NOT_MAPPED;
    }
    
    phys_addr_t phys = *entry & ~0xFFF;
    *entry = phys | flags;
    
    memory_flush_tlb_page(virt);
    return SUCCESS;
}

u32 page_table_get_flags(address_space_t* as, virt_addr_t virt) {
    if (!as) {
        return 0;
    }
    
    u64* entry = get_page_entry(as, virt, false);
    if (!entry || !(*entry & PAGE_FLAG_PRESENT)) {
        return 0;
    }
    
    return (u32)(*entry & 0xFFF);
}

// VMA (Virtual Memory Area) management
vma_t* vma_create(virt_addr_t start, virt_addr_t end, u32 flags, u32 prot) {
    if (start >= end || start % PAGE_SIZE != 0 || end % PAGE_SIZE != 0) {
        return NULL;
    }
    
    vma_t* vma = (vma_t*)kernel_heap_alloc(sizeof(vma_t), ALLOC_FLAG_ZERO);
    if (!vma) {
        return NULL;
    }
    
    vma->start = start;
    vma->end = end;
    vma->flags = flags;
    vma->prot = prot;
    vma->next = NULL;
    vma->prev = NULL;
    
    KDEBUG("Created VMA: 0x%llx-0x%llx, flags=0x%x, prot=0x%x", start, end, flags, prot);
    return vma;
}

void vma_destroy(vma_t* vma) {
    if (!vma) return;
    
    KDEBUG("Destroying VMA: 0x%llx-0x%llx", vma->start, vma->end);
    kernel_heap_free(vma);
}

error_t vma_insert(address_space_t* as, vma_t* vma) {
    if (!as || !vma) {
        return E_INVAL;
    }
    
    // Check for overlaps with existing VMAs
    vma_t* current = as->vma_list;
    vma_t* prev = NULL;
    
    while (current) {
        if ((vma->start < current->end) && (vma->end > current->start)) {
            // Overlap detected
            return E_MEMORY_ALREADY_MAPPED;
        }
        
        if (current->start > vma->end) {
            // Found insertion point
            break;
        }
        
        prev = current;
        current = current->next;
    }
    
    // Insert VMA into list
    vma->next = current;
    vma->prev = prev;
    
    if (prev) {
        prev->next = vma;
    } else {
        as->vma_list = vma;
    }
    
    if (current) {
        current->prev = vma;
    }
    
    return SUCCESS;
}

error_t vma_remove(address_space_t* as, vma_t* vma) {
    if (!as || !vma) {
        return E_INVAL;
    }
    
    // Remove from linked list
    if (vma->prev) {
        vma->prev->next = vma->next;
    } else {
        as->vma_list = vma->next;
    }
    
    if (vma->next) {
        vma->next->prev = vma->prev;
    }
    
    return SUCCESS;
}

vma_t* vma_find(address_space_t* as, virt_addr_t addr) {
    if (!as) {
        return NULL;
    }
    
    vma_t* vma = as->vma_list;
    while (vma) {
        if (addr >= vma->start && addr < vma->end) {
            return vma;
        }
        vma = vma->next;
    }
    
    return NULL;
}

// Page fault handler
void page_fault_handler(virt_addr_t fault_addr, u32 error_code) {
    KDEBUG("Page fault at 0x%llx, error code: 0x%x", fault_addr, error_code);
    
    address_space_t* as = address_space_current();
    if (!as) {
        KERNEL_PANIC("Page fault with no current address space");
    }
    
    // Check if fault is in a valid VMA
    vma_t* vma = vma_find(as, fault_addr);
    if (!vma) {
        KERROR("Page fault in unmapped region: 0x%llx", fault_addr);
        // TODO: Send SIGSEGV to process
        return;
    }
    
    // Check permissions
    bool is_write = (error_code & 0x02) != 0;
    bool is_user = (error_code & 0x04) != 0;
    
    if (is_write && !(vma->prot & PROT_WRITE)) {
        KERROR("Write access violation at 0x%llx", fault_addr);
        // TODO: Send SIGSEGV to process
        return;
    }
    
    if (is_user && !(vma->prot & PROT_USER)) {
        KERROR("User access violation at 0x%llx", fault_addr);
        // TODO: Send SIGSEGV to process
        return;
    }
    
    // Allocate physical page and map it
    phys_addr_t phys = physical_alloc_page();
    if (phys == 0) {
        KERROR("Out of memory during page fault handling");
        // TODO: Send SIGKILL to process
        return;
    }
    
    u32 flags = PAGE_FLAG_PRESENT;
    if (vma->prot & PROT_WRITE) flags |= PAGE_FLAG_WRITABLE;
    if (vma->prot & PROT_USER) flags |= PAGE_FLAG_USER;
    
    virt_addr_t page_addr = PAGE_ALIGN_DOWN(fault_addr);
    error_t result = page_table_map(as, page_addr, phys, flags);
    if (result != SUCCESS) {
        KERROR("Failed to map page during fault handling: %d", result);
        physical_free_page(phys);
        // TODO: Send SIGKILL to process
        return;
    }
    
    // Zero the page if needed
    if (vma->flags & ALLOC_FLAG_ZERO) {
        memset((void*)page_addr, 0, PAGE_SIZE);
    }
    
    KDEBUG("Page fault resolved: mapped 0x%llx to 0x%llx", page_addr, phys);
}

// Memory mapping functions
void* memory_map(virt_addr_t virt, phys_addr_t phys, size_t size, u32 flags) {
    if (virt % PAGE_SIZE != 0 || phys % PAGE_SIZE != 0 || size == 0) {
        return NULL;
    }
    
    address_space_t* as = address_space_current();
    if (!as) {
        return NULL;
    }
    
    size = PAGE_ALIGN_UP(size);
    u32 page_count = size / PAGE_SIZE;
    
    for (u32 i = 0; i < page_count; i++) {
        virt_addr_t v = virt + i * PAGE_SIZE;
        phys_addr_t p = phys + i * PAGE_SIZE;
        
        error_t result = page_table_map(as, v, p, flags);
        if (result != SUCCESS) {
            // Unmap pages we've already mapped
            for (u32 j = 0; j < i; j++) {
                page_table_unmap(as, virt + j * PAGE_SIZE);
            }
            return NULL;
        }
    }
    
    return (void*)virt;
}

void memory_unmap(void* virt, size_t size) {
    if (!virt || size == 0) {
        return;
    }
    
    virt_addr_t addr = (virt_addr_t)virt;
    if (addr % PAGE_SIZE != 0) {
        return;
    }
    
    address_space_t* as = address_space_current();
    if (!as) {
        return;
    }
    
    size = PAGE_ALIGN_UP(size);
    u32 page_count = size / PAGE_SIZE;
    
    for (u32 i = 0; i < page_count; i++) {
        page_table_unmap(as, addr + i * PAGE_SIZE);
    }
}

// TLB and cache management
void memory_flush_tlb(void) {
    hal_flush_tlb();
}

void memory_flush_tlb_page(virt_addr_t addr) {
    hal_invalidate_tlb_entry(addr);
}

void memory_invalidate_cache(void) {
    hal_invalidate_cache();
}