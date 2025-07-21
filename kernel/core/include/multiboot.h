#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "types.h"

// Multiboot header magic numbers
#define MULTIBOOT_HEADER_MAGIC    0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Multiboot header flags
#define MULTIBOOT_PAGE_ALIGN      0x00000001
#define MULTIBOOT_MEMORY_INFO     0x00000002
#define MULTIBOOT_VIDEO_MODE      0x00000004
#define MULTIBOOT_AOUT_KLUDGE     0x00010000

// Multiboot information flags
#define MULTIBOOT_INFO_MEMORY     0x00000001
#define MULTIBOOT_INFO_BOOTDEV    0x00000002
#define MULTIBOOT_INFO_CMDLINE    0x00000004
#define MULTIBOOT_INFO_MODS       0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS  0x00000010
#define MULTIBOOT_INFO_ELF_SHDR   0x00000020
#define MULTIBOOT_INFO_MEM_MAP    0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO 0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE 0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME 0x00000200
#define MULTIBOOT_INFO_APM_TABLE  0x00000400
#define MULTIBOOT_INFO_VBE_INFO   0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x00001000

// Multiboot header flags (for bootloader_handoff compatibility)
#define MULTIBOOT_HEADER_FLAG_MMAP        MULTIBOOT_INFO_MEM_MAP
#define MULTIBOOT_HEADER_FLAG_MODS        MULTIBOOT_INFO_MODS
#define MULTIBOOT_HEADER_FLAG_CMDLINE     MULTIBOOT_INFO_CMDLINE
#define MULTIBOOT_HEADER_FLAG_FRAMEBUFFER MULTIBOOT_INFO_FRAMEBUFFER_INFO

// Multiboot structures
typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t color_info[6];
} __attribute__((packed)) multiboot_info_t;

typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct multiboot_module {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} __attribute__((packed)) multiboot_module_t;

// Multiboot header structure
typedef struct multiboot_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;
    uint32_t mode_type;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} __attribute__((packed)) multiboot_header_t;

// Memory map types
#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5

#endif // MULTIBOOT_H