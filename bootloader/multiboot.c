#include "../kernel/core/include/multiboot.h"
#include "../kernel/core/include/kernel.h"
#include "../kernel/hal/include/hal/hal.h"

// Use multiboot definitions from kernel headers



// Global multiboot information
static multiboot_info_t* multiboot_info = NULL;
static bool multiboot_valid = false;
static u32 multiboot_magic = 0;

// Memory map information
static multiboot_mmap_entry_t* multiboot_memory_map = NULL;
static u32 memory_map_count = 0;
static u32 total_memory = 0;
static u32 available_memory = 0;

// Module information
static multiboot_module_t* modules = NULL;
static u32 module_count = 0;

// VBE information
static vbe_mode_info_t* vbe_mode_info = NULL;
static bool vbe_available = false;

// Forward declarations
static error_t multiboot_parse_memory_map(void);
static error_t multiboot_parse_modules(void);
static error_t multiboot_parse_vbe_info(void);
static error_t multiboot_validate_info(void);
static void multiboot_dump_info(void);

// Initialize multiboot support
error_t multiboot_init(u32 magic, multiboot_info_t* info) {
    KINFO("Initializing multiboot support");
    
    multiboot_magic = magic;
    multiboot_info = info;
    
    // Validate multiboot magic
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        KERROR("Invalid multiboot magic: 0x%x", magic);
        return E_INVAL;
    }
    
    if (!info) {
        KERROR("Invalid multiboot info pointer");
        return E_INVAL;
    }
    
    // Validate multiboot information
    error_t result = multiboot_validate_info();
    if (result != SUCCESS) {
        KERROR("Invalid multiboot information");
        return result;
    }
    
    // Parse memory map
    if (info->flags & MULTIBOOT_HEADER_FLAG_MMAP) {
        result = multiboot_parse_memory_map();
        if (result != SUCCESS) {
            KERROR("Failed to parse memory map");
            return result;
        }
    }
    
    // Parse modules
    if (info->flags & MULTIBOOT_HEADER_FLAG_MODS) {
        result = multiboot_parse_modules();
        if (result != SUCCESS) {
            KERROR("Failed to parse modules");
            return result;
        }
    }
    
    // Parse VBE information
    if (info->flags & MULTIBOOT_HEADER_FLAG_VBE) {
        result = multiboot_parse_vbe_info();
        if (result != SUCCESS) {
            KERROR("Failed to parse VBE information");
            return result;
        }
    }
    
    multiboot_valid = true;
    
    // Dump multiboot information
    multiboot_dump_info();
    
    KINFO("Multiboot support initialized successfully");
    return SUCCESS;
}

// Validate multiboot information
static error_t multiboot_validate_info(void) {
    if (!multiboot_info) {
        return E_INVAL;
    }
    
    // Check for required flags
    if (!(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MEMORY)) {
        KERROR("Multiboot memory information not available");
        return E_INVAL;
    }
    
    // Validate memory information
    if (multiboot_info->mem_lower == 0 && multiboot_info->mem_upper == 0) {
        KERROR("Invalid memory information");
        return E_INVAL;
    }
    
    // Validate memory map if present
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MMAP) {
        if (multiboot_info->mmap_addr == 0 || multiboot_info->mmap_length == 0) {
            KERROR("Invalid memory map information");
            return E_INVAL;
        }
    }
    
    // Validate modules if present
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MODS) {
        if (multiboot_info->mods_addr == 0) {
            KERROR("Invalid module information");
            return E_INVAL;
        }
    }
    
    return SUCCESS;
}

// Parse memory map
static error_t multiboot_parse_memory_map(void) {
    if (!multiboot_info || !(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MMAP)) {
        return E_INVAL;
    }
    
    multiboot_memory_map = (multiboot_mmap_entry_t*)multiboot_info->mmap_addr;
    memory_map_count = multiboot_info->mmap_length / sizeof(multiboot_mmap_entry_t);
    
    KINFO("Memory map: %u entries", memory_map_count);
    
    // Calculate total and available memory
    total_memory = 0;
    available_memory = 0;
    
    for (u32 i = 0; i < memory_map_count; i++) {
        multiboot_mmap_entry_t* entry = &multiboot_memory_map[i];
        
        u64 start_addr = entry->addr;
        u64 end_addr = entry->addr + entry->len;
        u64 size = entry->len;
        
        KDEBUG("Memory region %u: 0x%llx-0x%llx (%llu bytes, type: %u)",
               i, start_addr, end_addr, size, entry->type);
        
        // Type 1 = available memory
        if (entry->type == 1) {
            available_memory += size;
        }
        
        total_memory += size;
    }
    
    KINFO("Total memory: %u MB, Available: %u MB",
          total_memory / (1024 * 1024), available_memory / (1024 * 1024));
    
    return SUCCESS;
}

// Parse modules
static error_t multiboot_parse_modules(void) {
    if (!multiboot_info || !(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MODS)) {
        return E_INVAL;
    }
    
    modules = (multiboot_module_t*)multiboot_info->mods_addr;
    module_count = multiboot_info->mods_count;
    
    KINFO("Modules: %u", module_count);
    
    for (u32 i = 0; i < module_count; i++) {
        multiboot_module_t* module = &modules[i];
        char* module_name = (char*)module->string;
        
        KINFO("Module %u: %s (0x%x-0x%x, %u bytes)",
              i, module_name ? module_name : "unnamed",
              module->mod_start, module->mod_end,
              module->mod_end - module->mod_start);
    }
    
    return SUCCESS;
}

// Parse VBE information
static error_t multiboot_parse_vbe_info(void) {
    if (!multiboot_info || !(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_VBE)) {
        return E_INVAL;
    }
    
    if (multiboot_info->vbe_mode_info == 0) {
        KERROR("VBE mode info not available");
        return E_INVAL;
    }
    
    vbe_mode_info = (vbe_mode_info_t*)multiboot_info->vbe_mode_info;
    
    // Validate VBE information
    if (vbe_mode_info->framebuffer == 0) {
        KERROR("VBE framebuffer not available");
        return E_INVAL;
    }
    
    vbe_available = true;
    
    KINFO("VBE: %ux%u, %u bpp, framebuffer: 0x%x",
          vbe_mode_info->width, vbe_mode_info->height,
          vbe_mode_info->bpp, vbe_mode_info->framebuffer);
    
    return SUCCESS;
}

// Get memory information
error_t multiboot_get_memory_info(u32* total, u32* available) {
    if (!multiboot_valid) {
        return E_INVAL;
    }
    
    if (total) {
        *total = total_memory;
    }
    
    if (available) {
        *available = available_memory;
    }
    
    return SUCCESS;
}

// Get memory map
error_t multiboot_get_memory_map(multiboot_mmap_entry_t** map, u32* count) {
    if (!multiboot_valid || !multiboot_memory_map) {
        return E_INVAL;
    }
    
    if (map) {
        *map = multiboot_memory_map;
    }
    
    if (count) {
        *count = memory_map_count;
    }
    
    return SUCCESS;
}

// Get modules
error_t multiboot_get_modules(multiboot_module_t** mods, u32* count) {
    if (!multiboot_valid || !modules) {
        return E_INVAL;
    }
    
    if (mods) {
        *mods = modules;
    }
    
    if (count) {
        *count = module_count;
    }
    
    return SUCCESS;
}

// Get VBE information
error_t multiboot_get_vbe_info(vbe_mode_info_t** info) {
    if (!multiboot_valid || !vbe_available) {
        return E_INVAL;
    }
    
    if (info) {
        *info = vbe_mode_info;
    }
    
    return SUCCESS;
}

// Get command line
error_t multiboot_get_cmdline(char** cmdline) {
    if (!multiboot_valid || !multiboot_info) {
        return E_INVAL;
    }
    
    if (!(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_CMDLINE)) {
        return E_INVAL;
    }
    
    if (cmdline) {
        *cmdline = (char*)multiboot_info->cmdline;
    }
    
    return SUCCESS;
}

// Get boot device
error_t multiboot_get_boot_device(u32* device) {
    if (!multiboot_valid || !multiboot_info) {
        return E_INVAL;
    }
    
    if (!(multiboot_info->flags & MULTIBOOT_HEADER_FLAG_BOOT_DEVICE)) {
        return E_INVAL;
    }
    
    if (device) {
        *device = multiboot_info->boot_device;
    }
    
    return SUCCESS;
}

// Check if multiboot is valid
bool multiboot_is_valid(void) {
    return multiboot_valid;
}

// Dump multiboot information
static void multiboot_dump_info(void) {
    if (!multiboot_info) {
        return;
    }
    
    KINFO("=== Multiboot Information ===");
    KINFO("Magic: 0x%x", multiboot_magic);
    KINFO("Flags: 0x%x", multiboot_info->flags);
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MEMORY) {
        KINFO("Memory: %u KB lower, %u KB upper",
              multiboot_info->mem_lower, multiboot_info->mem_upper);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_BOOT_DEVICE) {
        KINFO("Boot device: 0x%x", multiboot_info->boot_device);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_CMDLINE) {
        char* cmdline = (char*)multiboot_info->cmdline;
        KINFO("Command line: %s", cmdline ? cmdline : "none");
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MODS) {
        KINFO("Modules: %u", multiboot_info->mods_count);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_MMAP) {
        KINFO("Memory map: %u bytes at 0x%x",
              multiboot_info->mmap_length, multiboot_info->mmap_addr);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_DRIVES) {
        KINFO("Drives: %u bytes at 0x%x",
              multiboot_info->drives_length, multiboot_info->drives_addr);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_CONFIG_TABLE) {
        KINFO("Config table: 0x%x", multiboot_info->config_table);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_BOOT_LOADER_NAME) {
        char* loader_name = (char*)multiboot_info->boot_loader_name;
        KINFO("Boot loader: %s", loader_name ? loader_name : "unknown");
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_APM_TABLE) {
        KINFO("APM table: 0x%x", multiboot_info->apm_table);
    }
    
    if (multiboot_info->flags & MULTIBOOT_HEADER_FLAG_VBE) {
        KINFO("VBE: control info 0x%x, mode info 0x%x, mode %u",
              multiboot_info->vbe_control_info,
              multiboot_info->vbe_mode_info,
              multiboot_info->vbe_mode);
    }
} 