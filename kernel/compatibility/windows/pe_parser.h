#ifndef PE_PARSER_H
#define PE_PARSER_H

#include "types.h"
#include <stddef.h>

// PE file constants
#define PE_SIGNATURE 0x00004550  // "PE\0\0"
#define DOS_SIGNATURE 0x5A4D     // "MZ"
#define DOS_HEADER_SIZE 64
#define PE_HEADER_SIZE 248

// PE file characteristics
#define IMAGE_FILE_EXECUTABLE_IMAGE    0x0002
#define IMAGE_FILE_DLL                 0x2000
#define IMAGE_FILE_SYSTEM              0x1000
#define IMAGE_FILE_DLL_CHARACTERISTICS 0x2000

// PE optional header magic values
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b

// Section characteristics
#define IMAGE_SCN_CNT_CODE             0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE          0x20000000
#define IMAGE_SCN_MEM_READ             0x40000000
#define IMAGE_SCN_MEM_WRITE            0x80000000

// Import/Export structures
typedef struct {
    u32 characteristics;
    u32 time_date_stamp;
    u16 major_version;
    u16 minor_version;
    u32 name;
    u32 base;
    u32 number_of_functions;
    u32 number_of_names;
    u32 address_of_functions;
    u32 address_of_names;
    u32 address_of_name_ordinals;
} pe_export_directory_t;

typedef struct {
    u32 characteristics;
    u32 time_date_stamp;
    u16 major_version;
    u16 minor_version;
    u32 name;
    u32 first_thunk;
} pe_import_directory_t;

// DOS header
typedef struct {
    u16 e_magic;      // Magic number
    u16 e_cblp;       // Bytes on last page of file
    u16 e_cp;         // Pages in file
    u16 e_crlc;       // Relocations
    u16 e_cparhdr;    // Size of header in paragraphs
    u16 e_minalloc;   // Minimum extra paragraphs needed
    u16 e_maxalloc;   // Maximum extra paragraphs needed
    u16 e_ss;         // Initial (relative) SS value
    u16 e_sp;         // Initial SP value
    u16 e_csum;       // Checksum
    u16 e_ip;         // Initial IP value
    u16 e_cs;         // Initial (relative) CS value
    u16 e_lfarlc;     // File address of relocation table
    u16 e_ovno;       // Overlay number
    u16 e_res[4];     // Reserved words
    u16 e_oemid;      // OEM identifier
    u16 e_oeminfo;    // OEM information
    u16 e_res2[10];   // Reserved words
    u32 e_lfanew;     // File address of new exe header
} dos_header_t;

// PE file header
typedef struct {
    u32 signature;           // PE signature
    u16 machine;            // Target machine
    u16 number_of_sections; // Number of sections
    u32 time_date_stamp;    // Time/date stamp
    u32 pointer_to_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header;
    u16 characteristics;
} pe_file_header_t;

// PE optional header (32-bit)
typedef struct {
    u16 magic;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    u32 base_of_data;
    u32 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_operating_system_version;
    u16 minor_operating_system_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 win32_version_value;
    u32 size_of_image;
    u32 size_of_headers;
    u32 checksum;
    u16 subsystem;
    u16 dll_characteristics;
    u32 size_of_stack_reserve;
    u32 size_of_stack_commit;
    u32 size_of_heap_reserve;
    u32 size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
} pe_optional_header32_t;

// PE optional header (64-bit)
typedef struct {
    u16 magic;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    u64 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_operating_system_version;
    u16 minor_operating_system_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 win32_version_value;
    u32 size_of_image;
    u32 size_of_headers;
    u32 checksum;
    u16 subsystem;
    u16 dll_characteristics;
    u64 size_of_stack_reserve;
    u64 size_of_stack_commit;
    u64 size_of_heap_reserve;
    u64 size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
} pe_optional_header64_t;

// PE section header
typedef struct {
    char name[8];
    u32 virtual_size;
    u32 virtual_address;
    u32 size_of_raw_data;
    u32 pointer_to_raw_data;
    u32 pointer_to_relocations;
    u32 pointer_to_line_numbers;
    u16 number_of_relocations;
    u16 number_of_line_numbers;
    u32 characteristics;
} pe_section_header_t;

// PE data directory
typedef struct {
    u32 virtual_address;
    u32 size;
} pe_data_directory_t;

// PE file information structure
typedef struct {
    dos_header_t dos_header;
    pe_file_header_t file_header;
    union {
        pe_optional_header32_t optional_header32;
        pe_optional_header64_t optional_header64;
    } optional_header;
    pe_section_header_t* sections;
    pe_data_directory_t* data_directories;
    bool is_64bit;
    u32 entry_point;
    u64 image_base;
    u32 image_size;
    u32 number_of_sections;
    u32 number_of_data_directories;
} pe_file_info_t;

// Import information
typedef struct {
    char* dll_name;
    char** function_names;
    u32* function_ordinals;
    u32 number_of_functions;
} pe_import_info_t;

// Export information
typedef struct {
    char* module_name;
    char** function_names;
    u32* function_addresses;
    u32* function_ordinals;
    u32 number_of_functions;
} pe_export_info_t;

// PE parser functions
error_t pe_parse_file(const char* file_path, pe_file_info_t* info);
error_t pe_parse_memory(const void* data, size_t size, pe_file_info_t* info);
error_t pe_validate_header(const pe_file_info_t* info);
error_t pe_get_imports(const pe_file_info_t* info, pe_import_info_t* imports, u32* num_imports);
error_t pe_get_exports(const pe_file_info_t* info, pe_export_info_t* exports);
error_t pe_get_section_by_name(const pe_file_info_t* info, const char* name, pe_section_header_t* section);
error_t pe_get_section_by_address(const pe_file_info_t* info, u32 address, pe_section_header_t* section);
void pe_free_info(pe_file_info_t* info);
void pe_free_imports(pe_import_info_t* imports, u32 num_imports);
void pe_free_exports(pe_export_info_t* exports);

// Utility functions
bool pe_is_valid_pe_file(const char* file_path);
bool pe_is_64bit(const pe_file_info_t* info);
bool pe_is_dll(const pe_file_info_t* info);
u32 pe_rva_to_file_offset(const pe_file_info_t* info, u32 rva);
u32 pe_file_offset_to_rva(const pe_file_info_t* info, u32 file_offset);

#endif // PE_PARSER_H 