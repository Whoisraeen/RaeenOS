#ifndef RAEENOS_EXECUTABLE_LOADER_H
#define RAEENOS_EXECUTABLE_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "process.h"

// Flat binary executable header
typedef struct {
    uint32_t magic;           // Magic number: 0xRAEENOS
    uint32_t entry_point;     // Entry point offset
    uint32_t code_size;       // Size of code section
    uint32_t data_size;       // Size of data section
    uint32_t bss_size;        // Size of BSS section
    uint32_t stack_size;      // Size of initial stack
    uint32_t flags;           // Load flags
} flat_binary_header_t;

#define FLAT_BINARY_MAGIC 0x4E454152  // "RAEN" in little-endian
#define FLAT_BINARY_FLAG_EXECUTABLE 0x00000001
#define FLAT_BINARY_FLAG_READONLY    0x00000002

// Executable loader functions
int load_flat_binary(const char* filename, process_t* process);
int setup_user_address_space(process_t* process, void* binary_data, size_t binary_size);
void* map_user_stack(process_t* process, size_t stack_size);
int jump_to_user_mode(process_t* process, uintptr_t entry_point);

// RAM disk functions for loading binaries
void* ramdisk_read_file(const char* filename, size_t* size);
int ramdisk_init(void);

#endif // RAEENOS_EXECUTABLE_LOADER_H 