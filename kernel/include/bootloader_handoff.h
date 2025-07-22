#ifndef BOOTLOADER_HANDOFF_H
#define BOOTLOADER_HANDOFF_H

#include "core/include/error.h"
#include "core/include/multiboot.h"

typedef struct {
    multiboot_info_t* mb_info;
    uint32_t magic;
    uint64_t kernel_start;
    uint64_t kernel_end;
    uint64_t total_memory;
} bootloader_handoff_t;

error_t bootloader_handoff_init(multiboot_info_t* mb_info);
error_t bootloader_handoff_complete(void);
error_t bootloader_get_handoff_info(bootloader_handoff_t** info);

#endif