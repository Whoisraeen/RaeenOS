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

// Multiboot header structure
struct multiboot_header {
    u32 magic;
    u32 flags;
    u32 checksum;
    u32 header_addr;
    u32 load_addr;
    u32 load_end_addr;
    u32 bss_end_addr;
    u32 entry_addr;
    u32 mode_type;
    u32 width;
    u32 height;
    u32 depth;
} __attribute__((packed));

// Multiboot information structure
struct multiboot_info {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    
    union {
        struct {
            u32 tabsize;
            u32 strsize;
            u32 addr;
            u32 reserved;
        } aout_sym;
        
        struct {
            u32 num;
            u32 size;
            u32 addr;
            u32 shndx;
        } elf_sec;
    } u;
    
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    
    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8  framebuffer_bpp;
    u8  framebuffer_type;
    
    union {
        struct {
            u32 framebuffer_palette_addr;
            u16 framebuffer_palette_num_colors;
        };
        struct {
            u8 framebuffer_red_field_position;
            u8 framebuffer_red_mask_size;
            u8 framebuffer_green_field_position;
            u8 framebuffer_green_mask_size;
            u8 framebuffer_blue_field_position;
            u8 framebuffer_blue_mask_size;
        };
    };
} __attribute__((packed));

// Memory map entry structure
struct multiboot_mmap_entry {
    u32 size;
    u64 addr;
    u64 len;
    u32 type;
} __attribute__((packed));

// Memory map types
#define MULTIBOOT_MEMORY_AVAILABLE   1
#define MULTIBOOT_MEMORY_RESERVED    2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS         4
#define MULTIBOOT_MEMORY_BADRAM      5

// Module structure
struct multiboot_mod_list {
    u32 mod_start;
    u32 mod_end;
    u32 cmdline;
    u32 pad;
} __attribute__((packed));

// Drive info structure
struct multiboot_drive_info {
    u32 size;
    u8  drive_number;
    u8  drive_mode;
    u16 drive_cylinders;
    u8  drive_heads;
    u8  drive_sectors;
    u16 drive_ports[0];
} __attribute__((packed));

// APM table structure
struct multiboot_apm_info {
    u16 version;
    u16 cseg;
    u32 offset;
    u16 cseg_16;
    u16 dseg;
    u16 flags;
    u16 cseg_len;
    u16 cseg_16_len;
    u16 dseg_len;
} __attribute__((packed));

// Utility functions for multiboot
bool multiboot_is_valid(u32 magic, struct multiboot_info* mbi);
void multiboot_parse_memory_map(struct multiboot_info* mbi);
void multiboot_parse_modules(struct multiboot_info* mbi);
const char* multiboot_get_cmdline(struct multiboot_info* mbi);
const char* multiboot_get_bootloader_name(struct multiboot_info* mbi);

#endif // MULTIBOOT_H