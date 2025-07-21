#ifndef BOOTLOADER_HANDOFF_H
#define BOOTLOADER_HANDOFF_H

#include "types.h"
#include "error.h"
#include "multiboot.h"

// Bootloader handoff magic number
#define BOOTLOADER_HANDOFF_MAGIC    0xDEADBEEF

// Maximum number of memory regions and modules
#define MAX_MEMORY_REGIONS          64
#define MAX_MODULES                 16

// Memory region types
typedef enum {
    MEMORY_TYPE_AVAILABLE       = 1,
    MEMORY_TYPE_RESERVED        = 2,
    MEMORY_TYPE_ACPI_RECLAIMABLE = 3,
    MEMORY_TYPE_ACPI_NVS        = 4,
    MEMORY_TYPE_BAD             = 5
} memory_region_type_t;

// Memory region structure
typedef struct {
    uint64_t base_addr;
    uint64_t length;
    memory_region_type_t type;
    bool available;
} memory_region_t;

// Module information structure
typedef struct {
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t size;
    char name[64];
} module_info_t;

// Framebuffer information structure
typedef struct {
    uint64_t addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    uint8_t type;
    bool available;
} framebuffer_info_t;

// Bootloader handoff structure
typedef struct {
    uint32_t magic;
    bool initialized;
    
    // Multiboot information
    multiboot_info_t* multiboot_info;
    
    // Memory information
    memory_region_t memory_regions[MAX_MEMORY_REGIONS];
    uint32_t memory_map_count;
    uint64_t total_memory;
    uint64_t available_memory;
    
    // Module information
    module_info_t modules[MAX_MODULES];
    uint32_t module_count;
    
    // Framebuffer information
    framebuffer_info_t framebuffer;
    
    // Command line
    char cmdline[256];
    
    // Boot device
    uint32_t boot_device;
} bootloader_handoff_t;

// Function prototypes

// Initialization
error_t bootloader_handoff_init(multiboot_info_t* mb_info);
error_t bootloader_handoff_complete(void);

// Parsing functions
error_t bootloader_parse_memory_map(void);
error_t bootloader_parse_modules(void);
error_t bootloader_parse_framebuffer(void);
error_t bootloader_parse_cmdline(void);

// Memory management integration
error_t bootloader_mark_kernel_regions(void);
error_t bootloader_mark_module_regions(void);

// Information retrieval
error_t bootloader_get_handoff_info(bootloader_handoff_t** info);
error_t bootloader_get_memory_regions(memory_region_t** regions, uint32_t* count);
error_t bootloader_get_modules(module_info_t** modules, uint32_t* count);
error_t bootloader_get_framebuffer(framebuffer_info_t** fb);
error_t bootloader_get_cmdline(char** cmdline);

// Status functions
bool bootloader_handoff_is_completed(void);
error_t bootloader_get_memory_stats(uint64_t* total, uint64_t* available, uint64_t* used);

#endif // BOOTLOADER_HANDOFF_H