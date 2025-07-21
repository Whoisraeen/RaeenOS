#include "pe.h"
#include "kernel.h"
#include "memory/memory.h"

// TODO: Implement pe_get_imports
error_t pe_get_imports(void* pe_base, import_t** imports, size_t* num_imports) {
    if (!pe_base || !imports || !num_imports) {
        return E_INVAL;
    }

    *imports = NULL;
    *num_imports = 0;
    KWARN("pe_get_imports is not implemented yet");
    return E_NOT_IMPLEMENTED;
}

// TODO: Implement pe_get_exports
error_t pe_get_exports(void* pe_base, export_t** exports, size_t* num_exports) {
    if (!pe_base || !exports || !num_exports) {
        return E_INVAL;
    }

    *exports = NULL;
    *num_exports = 0;
    KWARN("pe_get_exports is not implemented yet");
    return E_NOT_IMPLEMENTED;
}

// Add relocation parsing
error_t pe_apply_relocations(void* pe_base, phys_addr_t image_base) {
    if (!pe_base) {
        return E_INVAL;
    }

    dos_header_t* dos_header = (dos_header_t*)pe_base;
    if (dos_header->magic != 0x5A4D) { // MZ magic
        KERROR("Invalid DOS header magic: 0x%x", dos_header->magic);
        return E_INVALID_FILE_FORMAT;
    }

    if (dos_header->lfanew[0] == 0xE8) {
        KINFO("DOS stub program present");
    } else if (*(u32*)(pe_base + dos_header->lfanew[0]) != 0x00004550) {
        KERROR("Invalid PE signature: 0x%x", *(u32*)(pe_base + dos_header->lfanew[0]));
        return E_INVALID_FILE_FORMAT;
    }

    file_header_t* file_header = (file_header_t*)(pe_base + dos_header->lfanew[0] + 4);
    optional_header_t* optional_header = (optional_header_t*)(pe_base + dos_header->lfanew[0] + 4 + sizeof(file_header_t));

    // Check magic value in optional header
    if (optional_header->magic != 0x10B) {
        KERROR("Unsupported Optional Header magic value: 0x%x", optional_header->magic);
        return E_INVALID_FILE_FORMAT;
    }

    // Get relocation directory entry
    u32 reloc_rva = optional_header->data_directory[5 * 2];
    u32 reloc_size = optional_header->data_directory[5 * 2 + 1];

    if (reloc_rva == 0 || reloc_size == 0) {
        KINFO("No relocations found, assuming position independent executable");
        return SUCCESS;
    }

    u8* reloc_base = pe_base + reloc_rva;
    u8* reloc_end = reloc_base + reloc_size;
    u64 delta = image_base - optional_header->image_base;

    for (u8* block = reloc_base; block < reloc_end; ) {
        relocation_block_t* header = (relocation_block_t*)block;
        u32 num_entries = (header->block_size - sizeof(relocation_block_t)) / sizeof(u16);
        u16* entries = (u16*)(block + sizeof(relocation_block_t));

        for (u32 i = 0; i < num_entries; i++) {
            u16 entry = entries[i];
            u8 type = entry >> 12;
            u16 offset = entry & 0xFFF;

            if (type == 0xA) { // IMAGE_REL_BASED_HIGHLOW
                u32* address = (u32*)(pe_base + header->virtual_address + offset);
                *address += delta;
            } else if (type != 0) {
                KWARN("Unsupported relocation type: %d", type);
            }
        }
        block += header->block_size;
    }

    KDEBUG("Applied %u relocations to PE image", num_entries);
    return SUCCESS;
}