#include "pe_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

// Data directory indices
#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT 7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define IMAGE_DIRECTORY_ENTRY_IAT 12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14

// Maximum number of data directories
#define MAX_DATA_DIRECTORIES 16

error_t pe_parse_file(const char* file_path, pe_file_info_t* info) {
    if (!file_path || !info) {
        return E_INVAL;
    }
    
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        return E_NOENT;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory for file data
    void* file_data = malloc(file_size);
    if (!file_data) {
        fclose(file);
        return E_NOMEM;
    }
    
    // Read entire file
    size_t bytes_read = fread(file_data, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != file_size) {
        free(file_data);
        return E_IO;
    }
    
    // Parse from memory
    error_t result = pe_parse_memory(file_data, file_size, info);
    
    free(file_data);
    return result;
}

error_t pe_parse_memory(const void* data, size_t size, pe_file_info_t* info) {
    if (!data || !info || size < DOS_HEADER_SIZE) {
        return E_INVAL;
    }
    
    // Clear info structure
    memset(info, 0, sizeof(pe_file_info_t));
    
    const u8* file_data = (const u8*)data;
    
    // Parse DOS header
    const dos_header_t* dos_header = (const dos_header_t*)file_data;
    if (dos_header->e_magic != DOS_SIGNATURE) {
        return E_INVAL;  // Not a valid DOS executable
    }
    
    info->dos_header = *dos_header;
    
    // Check if PE header exists
    if (dos_header->e_lfanew >= size || dos_header->e_lfanew < DOS_HEADER_SIZE) {
        return E_INVAL;
    }
    
    // Parse PE file header
    const pe_file_header_t* pe_header = (const pe_file_header_t*)(file_data + dos_header->e_lfanew);
    if (pe_header->signature != PE_SIGNATURE) {
        return E_INVAL;  // Not a valid PE file
    }
    
    info->file_header = *pe_header;
    info->number_of_sections = pe_header->number_of_sections;
    
    // Parse optional header
    const u8* optional_header_ptr = (const u8*)(pe_header + 1);
    u16 magic = *(u16*)optional_header_ptr;
    
    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        // 32-bit PE
        const pe_optional_header32_t* opt_header = (const pe_optional_header32_t*)optional_header_ptr;
        info->optional_header.optional_header32 = *opt_header;
        info->is_64bit = false;
        info->entry_point = opt_header->address_of_entry_point;
        info->image_base = opt_header->image_base;
        info->image_size = opt_header->size_of_image;
        info->number_of_data_directories = opt_header->number_of_rva_and_sizes;
        
        // Get data directories
        const pe_data_directory_t* data_dirs = (const pe_data_directory_t*)(opt_header + 1);
        if (info->number_of_data_directories > MAX_DATA_DIRECTORIES) {
            info->number_of_data_directories = MAX_DATA_DIRECTORIES;
        }
        
        info->data_directories = malloc(info->number_of_data_directories * sizeof(pe_data_directory_t));
        if (info->data_directories) {
            memcpy(info->data_directories, data_dirs, 
                   info->number_of_data_directories * sizeof(pe_data_directory_t));
        }
        
    } else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        // 64-bit PE
        const pe_optional_header64_t* opt_header = (const pe_optional_header64_t*)optional_header_ptr;
        info->optional_header.optional_header64 = *opt_header;
        info->is_64bit = true;
        info->entry_point = opt_header->address_of_entry_point;
        info->image_base = opt_header->image_base;
        info->image_size = opt_header->size_of_image;
        info->number_of_data_directories = opt_header->number_of_rva_and_sizes;
        
        // Get data directories
        const pe_data_directory_t* data_dirs = (const pe_data_directory_t*)(opt_header + 1);
        if (info->number_of_data_directories > MAX_DATA_DIRECTORIES) {
            info->number_of_data_directories = MAX_DATA_DIRECTORIES;
        }
        
        info->data_directories = malloc(info->number_of_data_directories * sizeof(pe_data_directory_t));
        if (info->data_directories) {
            memcpy(info->data_directories, data_dirs, 
                   info->number_of_data_directories * sizeof(pe_data_directory_t));
        }
        
    } else {
        return E_INVAL;  // Invalid optional header magic
    }
    
    // Parse section headers
    const pe_data_directory_t* data_dirs = info->data_directories;
    const pe_section_header_t* section_headers = (const pe_section_header_t*)(data_dirs + info->number_of_data_directories);
    
    info->sections = malloc(info->number_of_sections * sizeof(pe_section_header_t));
    if (info->sections) {
        memcpy(info->sections, section_headers, 
               info->number_of_sections * sizeof(pe_section_header_t));
    }
    
    return SUCCESS;
}

error_t pe_validate_header(const pe_file_info_t* info) {
    if (!info) {
        return E_INVAL;
    }
    
    // Validate DOS header
    if (info->dos_header.e_magic != DOS_SIGNATURE) {
        return E_INVAL;
    }
    
    // Validate PE header
    if (info->file_header.signature != PE_SIGNATURE) {
        return E_INVAL;
    }
    
    // Validate optional header magic
    if (info->is_64bit) {
        if (info->optional_header.optional_header64.magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            return E_INVAL;
        }
    } else {
        if (info->optional_header.optional_header32.magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            return E_INVAL;
        }
    }
    
    // Validate number of sections
    if (info->number_of_sections > 96) {  // Reasonable limit
        return E_INVAL;
    }
    
    return SUCCESS;
}

error_t pe_get_imports(const pe_file_info_t* info, pe_import_info_t* imports, u32* num_imports) {
    if (!info || !imports || !num_imports) {
        return E_INVAL;
    }
    
    // Get import directory
    if (!info->data_directories || 
        info->number_of_data_directories <= IMAGE_DIRECTORY_ENTRY_IMPORT ||
        info->data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT].virtual_address == 0) {
        *num_imports = 0;
        return SUCCESS;
    }
    
    // TODO: Implement import parsing
    // This requires reading the import directory and resolving function names
    *num_imports = 0;
    
    return SUCCESS;
}

error_t pe_get_exports(const pe_file_info_t* info, pe_export_info_t* exports) {
    if (!info || !exports) {
        return E_INVAL;
    }
    
    // Get export directory
    if (!info->data_directories || 
        info->number_of_data_directories <= IMAGE_DIRECTORY_ENTRY_EXPORT ||
        info->data_directories[IMAGE_DIRECTORY_ENTRY_EXPORT].virtual_address == 0) {
        exports->number_of_functions = 0;
        return SUCCESS;
    }
    
    // TODO: Implement export parsing
    // This requires reading the export directory and resolving function names
    exports->number_of_functions = 0;
    
    return SUCCESS;
}

error_t pe_get_section_by_name(const pe_file_info_t* info, const char* name, pe_section_header_t* section) {
    if (!info || !name || !section) {
        return E_INVAL;
    }
    
    for (u32 i = 0; i < info->number_of_sections; i++) {
        if (strncmp(info->sections[i].name, name, 8) == 0) {
            *section = info->sections[i];
            return SUCCESS;
        }
    }
    
    return E_NOENT;
}

error_t pe_get_section_by_address(const pe_file_info_t* info, u32 address, pe_section_header_t* section) {
    if (!info || !section) {
        return E_INVAL;
    }
    
    for (u32 i = 0; i < info->number_of_sections; i++) {
        u32 section_start = info->sections[i].virtual_address;
        u32 section_end = section_start + info->sections[i].virtual_size;
        
        if (address >= section_start && address < section_end) {
            *section = info->sections[i];
            return SUCCESS;
        }
    }
    
    return E_NOENT;
}

void pe_free_info(pe_file_info_t* info) {
    if (!info) {
        return;
    }
    
    if (info->sections) {
        free(info->sections);
        info->sections = NULL;
    }
    
    if (info->data_directories) {
        free(info->data_directories);
        info->data_directories = NULL;
    }
    
    memset(info, 0, sizeof(pe_file_info_t));
}

void pe_free_imports(pe_import_info_t* imports, u32 num_imports) {
    if (!imports) {
        return;
    }
    
    for (u32 i = 0; i < num_imports; i++) {
        if (imports[i].dll_name) {
            free(imports[i].dll_name);
        }
        if (imports[i].function_names) {
            for (u32 j = 0; j < imports[i].number_of_functions; j++) {
                if (imports[i].function_names[j]) {
                    free(imports[i].function_names[j]);
                }
            }
            free(imports[i].function_names);
        }
        if (imports[i].function_ordinals) {
            free(imports[i].function_ordinals);
        }
    }
}

void pe_free_exports(pe_export_info_t* exports) {
    if (!exports) {
        return;
    }
    
    if (exports->module_name) {
        free(exports->module_name);
    }
    if (exports->function_names) {
        for (u32 i = 0; i < exports->number_of_functions; i++) {
            if (exports->function_names[i]) {
                free(exports->function_names[i]);
            }
        }
        free(exports->function_names);
    }
    if (exports->function_addresses) {
        free(exports->function_addresses);
    }
    if (exports->function_ordinals) {
        free(exports->function_ordinals);
    }
}

// Utility functions
bool pe_is_valid_pe_file(const char* file_path) {
    pe_file_info_t info;
    error_t result = pe_parse_file(file_path, &info);
    if (result == SUCCESS) {
        pe_free_info(&info);
        return true;
    }
    return false;
}

bool pe_is_64bit(const pe_file_info_t* info) {
    return info ? info->is_64bit : false;
}

bool pe_is_dll(const pe_file_info_t* info) {
    if (!info) {
        return false;
    }
    return (info->file_header.characteristics & IMAGE_FILE_DLL) != 0;
}

u32 pe_rva_to_file_offset(const pe_file_info_t* info, u32 rva) {
    if (!info) {
        return 0;
    }
    
    // Find section containing this RVA
    for (u32 i = 0; i < info->number_of_sections; i++) {
        u32 section_start = info->sections[i].virtual_address;
        u32 section_end = section_start + info->sections[i].virtual_size;
        
        if (rva >= section_start && rva < section_end) {
            u32 offset_in_section = rva - section_start;
            return info->sections[i].pointer_to_raw_data + offset_in_section;
        }
    }
    
    return 0;
}

u32 pe_file_offset_to_rva(const pe_file_info_t* info, u32 file_offset) {
    if (!info) {
        return 0;
    }
    
    // Find section containing this file offset
    for (u32 i = 0; i < info->number_of_sections; i++) {
        u32 section_file_start = info->sections[i].pointer_to_raw_data;
        u32 section_file_end = section_file_start + info->sections[i].size_of_raw_data;
        
        if (file_offset >= section_file_start && file_offset < section_file_end) {
            u32 offset_in_section = file_offset - section_file_start;
            return info->sections[i].virtual_address + offset_in_section;
        }
    }
    
    return 0;
} 