#include "bootloader_handoff.h"
#include "kernel.h"
#include "types.h"
#include "error.h"
#include "string.h"

// Global handoff information
static bootloader_handoff_t handoff_info = {0};
static bool handoff_completed = false;

// Initialize bootloader handoff
error_t bootloader_handoff_init(multiboot_info_t* mb_info) {
    if (!mb_info) {
        return E_INVAL;
    }
    
    KINFO("Initializing bootloader handoff");
    
    // Clear handoff structure
    memset(&handoff_info, 0, sizeof(bootloader_handoff_t));
    
    // Store multiboot info
    handoff_info.multiboot_info = mb_info;
    handoff_info.magic = BOOTLOADER_HANDOFF_MAGIC;
    
    // Parse memory map
    error_t result = bootloader_parse_memory_map();
    if (result != SUCCESS) {
        KERROR("Failed to parse memory map");
        return result;
    }
    
    // Parse modules
    result = bootloader_parse_modules();
    if (result != SUCCESS) {
        KWARN("Failed to parse modules (non-critical)");
    }
    
    // Parse framebuffer info
    result = bootloader_parse_framebuffer();
    if (result != SUCCESS) {
        KWARN("Failed to parse framebuffer info (non-critical)");
    }
    
    // Parse command line
    result = bootloader_parse_cmdline();
    if (result != SUCCESS) {
        KWARN("Failed to parse command line (non-critical)");
    }
    
    handoff_info.initialized = true;
    
    KINFO("Bootloader handoff initialized successfully");
    return SUCCESS;
}

// Parse memory map from multiboot
error_t bootloader_parse_memory_map(void) {
    multiboot_info_t* mb_info = handoff_info.multiboot_info;
    
    if (!(mb_info->flags & MULTIBOOT_HEADER_FLAG_MMAP)) {
        KERROR("Memory map not provided by bootloader");
        return E_INVAL;
    }
    
    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;
    uint32_t mmap_length = mb_info->mmap_length;
    uint32_t entry_count = mmap_length / sizeof(multiboot_mmap_entry_t);
    
    if (entry_count > MAX_MEMORY_REGIONS) {
        KWARN("Too many memory regions (%u), truncating to %u", 
              entry_count, MAX_MEMORY_REGIONS);
        entry_count = MAX_MEMORY_REGIONS;
    }
    
    handoff_info.memory_map_count = entry_count;
    handoff_info.total_memory = 0;
    handoff_info.available_memory = 0;
    
    KINFO("Processing %u memory regions", entry_count);
    
    for (uint32_t i = 0; i < entry_count; i++) {
        multiboot_mmap_entry_t* mb_entry = &mmap[i];
        memory_region_t* region = &handoff_info.memory_regions[i];
        
        region->base_addr = mb_entry->addr;
        region->length = mb_entry->len;
        region->type = (memory_region_type_t)mb_entry->type;
        region->available = (mb_entry->type == 1); // Type 1 = available RAM
        
        handoff_info.total_memory += region->length;
        if (region->available) {
            handoff_info.available_memory += region->length;
        }
        
        KDEBUG("Memory region %u: 0x%016llx-0x%016llx (%llu KB, type: %s)",
               i, region->base_addr, region->base_addr + region->length,
               region->length / 1024,
               region->available ? "Available" : "Reserved");
    }
    
    KINFO("Total memory: %llu MB, Available: %llu MB",
          handoff_info.total_memory / (1024 * 1024),
          handoff_info.available_memory / (1024 * 1024));
    
    return SUCCESS;
}

// Parse modules from multiboot
error_t bootloader_parse_modules(void) {
    multiboot_info_t* mb_info = handoff_info.multiboot_info;
    
    if (!(mb_info->flags & MULTIBOOT_HEADER_FLAG_MODS)) {
        KDEBUG("No modules provided by bootloader");
        return SUCCESS; // Not an error
    }
    
    multiboot_module_t* modules = (multiboot_module_t*)mb_info->mods_addr;
    uint32_t module_count = mb_info->mods_count;
    
    if (module_count > MAX_MODULES) {
        KWARN("Too many modules (%u), truncating to %u", 
              module_count, MAX_MODULES);
        module_count = MAX_MODULES;
    }
    
    handoff_info.module_count = module_count;
    
    KINFO("Processing %u modules", module_count);
    
    for (uint32_t i = 0; i < module_count; i++) {
        multiboot_module_t* mb_module = &modules[i];
        module_info_t* module = &handoff_info.modules[i];
        
        module->start_addr = mb_module->mod_start;
        module->end_addr = mb_module->mod_end;
        module->size = mb_module->mod_end - mb_module->mod_start;
        
        // Copy module name if available
        if (mb_module->string) {
            strncpy(module->name, (char*)mb_module->string, 
                   sizeof(module->name) - 1);
            module->name[sizeof(module->name) - 1] = '\0';
        } else {
            snprintf(module->name, sizeof(module->name), "module%u", i);
        }
        
        KINFO("Module %u: %s (0x%08x-0x%08x, %u bytes)",
              i, module->name, module->start_addr, module->end_addr, module->size);
    }
    
    return SUCCESS;
}

// Parse framebuffer info from multiboot
error_t bootloader_parse_framebuffer(void) {
    multiboot_info_t* mb_info = handoff_info.multiboot_info;
    
    if (!(mb_info->flags & MULTIBOOT_HEADER_FLAG_FRAMEBUFFER)) {
        KDEBUG("Framebuffer info not provided by bootloader");
        return SUCCESS; // Not an error
    }
    
    framebuffer_info_t* fb = &handoff_info.framebuffer;
    
    fb->addr = mb_info->framebuffer_addr;
    fb->width = mb_info->framebuffer_width;
    fb->height = mb_info->framebuffer_height;
    fb->pitch = mb_info->framebuffer_pitch;
    fb->bpp = mb_info->framebuffer_bpp;
    fb->type = mb_info->framebuffer_type;
    fb->available = true;
    
    KINFO("Framebuffer: %ux%u, %u bpp, pitch: %u, addr: 0x%016llx",
          fb->width, fb->height, fb->bpp, fb->pitch, fb->addr);
    
    return SUCCESS;
}

// Parse command line from multiboot
error_t bootloader_parse_cmdline(void) {
    multiboot_info_t* mb_info = handoff_info.multiboot_info;
    
    if (!(mb_info->flags & MULTIBOOT_HEADER_FLAG_CMDLINE)) {
        KDEBUG("Command line not provided by bootloader");
        return SUCCESS; // Not an error
    }
    
    char* cmdline = (char*)mb_info->cmdline;
    if (!cmdline) {
        return SUCCESS;
    }
    
    strncpy(handoff_info.cmdline, cmdline, sizeof(handoff_info.cmdline) - 1);
    handoff_info.cmdline[sizeof(handoff_info.cmdline) - 1] = '\0';
    
    KINFO("Command line: %s", handoff_info.cmdline);
    
    return SUCCESS;
}

// Complete bootloader handoff
error_t bootloader_handoff_complete(void) {
    if (!handoff_info.initialized) {
        KERROR("Bootloader handoff not initialized");
        return E_INVAL;
    }
    
    if (handoff_completed) {
        KWARN("Bootloader handoff already completed");
        return E_ALREADY;
    }
    
    KINFO("Completing bootloader handoff");
    
    // Initialize physical memory manager with memory map
    error_t result = pmm_init_from_handoff(&handoff_info);
    if (result != SUCCESS) {
        KERROR("Failed to initialize PMM from handoff");
        return result;
    }
    
    // Initialize virtual memory manager
    result = vmm_init_from_handoff(&handoff_info);
    if (result != SUCCESS) {
        KERROR("Failed to initialize VMM from handoff");
        return result;
    }
    
    // Mark kernel regions as used
    result = bootloader_mark_kernel_regions();
    if (result != SUCCESS) {
        KERROR("Failed to mark kernel regions");
        return result;
    }
    
    // Mark module regions as used
    result = bootloader_mark_module_regions();
    if (result != SUCCESS) {
        KERROR("Failed to mark module regions");
        return result;
    }
    
    handoff_completed = true;
    
    KINFO("Bootloader handoff completed successfully");
    return SUCCESS;
}

// Get handoff information
error_t bootloader_get_handoff_info(bootloader_handoff_t** info) {
    if (!handoff_completed) {
        return E_NOTREADY;
    }
    
    if (info) {
        *info = &handoff_info;
    }
    
    return SUCCESS;
}

// Mark kernel regions as used in memory manager
error_t bootloader_mark_kernel_regions(void) {
    // Get kernel start and end from linker symbols
    extern char _kernel_start[];
    extern char _kernel_end[];
    
    uintptr_t kernel_start = (uintptr_t)_kernel_start;
    uintptr_t kernel_end = (uintptr_t)_kernel_end;
    uintptr_t kernel_size = kernel_end - kernel_start;
    
    KINFO("Marking kernel region: 0x%016lx-0x%016lx (%lu KB)",
          kernel_start, kernel_end, kernel_size / 1024);
    
    // Mark kernel pages as used
    uintptr_t page_start = kernel_start & ~(PAGE_SIZE - 1);
    uintptr_t page_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    for (uintptr_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
        pmm_mark_page_used(addr);
    }
    
    return SUCCESS;
}

// Mark module regions as used in memory manager
error_t bootloader_mark_module_regions(void) {
    for (uint32_t i = 0; i < handoff_info.module_count; i++) {
        module_info_t* module = &handoff_info.modules[i];
        
        KINFO("Marking module %u region: 0x%08x-0x%08x (%u KB)",
              i, module->start_addr, module->end_addr, module->size / 1024);
        
        // Mark module pages as used
        uintptr_t page_start = module->start_addr & ~(PAGE_SIZE - 1);
        uintptr_t page_end = (module->end_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        
        for (uintptr_t addr = page_start; addr < page_end; addr += PAGE_SIZE) {
            pmm_mark_page_used(addr);
        }
    }
    
    return SUCCESS;
}

// Get handoff information
error_t bootloader_get_handoff_info(bootloader_handoff_t** info) {
    if (!info) {
        return E_INVAL;
    }
    
    if (!handoff_info.initialized) {
        return E_INVAL;
    }
    
    *info = &handoff_info;
    return SUCCESS;
}

// Get memory regions
error_t bootloader_get_memory_regions(memory_region_t** regions, uint32_t* count) {
    if (!regions || !count) {
        return E_INVAL;
    }
    
    if (!handoff_info.initialized) {
        return E_INVAL;
    }
    
    *regions = handoff_info.memory_regions;
    *count = handoff_info.memory_map_count;
    
    return SUCCESS;
}

// Get modules
error_t bootloader_get_modules(module_info_t** modules, uint32_t* count) {
    if (!modules || !count) {
        return E_INVAL;
    }
    
    if (!handoff_info.initialized) {
        return E_INVAL;
    }
    
    *modules = handoff_info.modules;
    *count = handoff_info.module_count;
    
    return SUCCESS;
}

// Get framebuffer info
error_t bootloader_get_framebuffer(framebuffer_info_t** fb) {
    if (!fb) {
        return E_INVAL;
    }
    
    if (!handoff_info.initialized || !handoff_info.framebuffer.available) {
        return E_INVAL;
    }
    
    *fb = &handoff_info.framebuffer;
    return SUCCESS;
}

// Get command line
error_t bootloader_get_cmdline(char** cmdline) {
    if (!cmdline) {
        return E_INVAL;
    }
    
    if (!handoff_info.initialized) {
        return E_INVAL;
    }
    
    *cmdline = handoff_info.cmdline;
    return SUCCESS;
}

// Check if handoff is completed
bool bootloader_handoff_is_completed(void) {
    return handoff_completed;
}

// Get memory statistics
error_t bootloader_get_memory_stats(uint64_t* total, uint64_t* available, uint64_t* used) {
    if (!handoff_info.initialized) {
        return E_INVAL;
    }
    
    if (total) {
        *total = handoff_info.total_memory;
    }
    
    if (available) {
        *available = handoff_info.available_memory;
    }
    
    if (used) {
        *used = handoff_info.total_memory - handoff_info.available_memory;
    }
    
    return SUCCESS;
}