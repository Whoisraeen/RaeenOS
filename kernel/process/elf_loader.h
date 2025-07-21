#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "types.h"
#include "process/process.h"
#include "memory/memory.h"

// ELF magic number
#define ELF_MAGIC 0x464C457F  // "\x7fELF"

// ELF file classes
#define ELF_CLASS_NONE 0
#define ELF_CLASS_32   1
#define ELF_CLASS_64   2

// ELF data encodings
#define ELF_DATA_NONE 0
#define ELF_DATA_LSB  1
#define ELF_DATA_MSB  2

// ELF versions
#define ELF_VERSION_NONE 0
#define ELF_VERSION_CURRENT 1

// ELF OS/ABI
#define ELF_OSABI_NONE    0
#define ELF_OSABI_LINUX   3
#define ELF_OSABI_FREEBSD 9

// ELF machine types
#define ELF_MACHINE_NONE    0
#define ELF_MACHINE_X86     3
#define ELF_MACHINE_X86_64  62
#define ELF_MACHINE_ARM     40
#define ELF_MACHINE_AARCH64 183

// ELF object file types
#define ELF_TYPE_NONE   0
#define ELF_TYPE_REL    1
#define ELF_TYPE_EXEC   2
#define ELF_TYPE_DYN    3
#define ELF_TYPE_CORE   4

// ELF section header types
#define ELF_SHT_NULL     0
#define ELF_SHT_PROGBITS 1
#define ELF_SHT_SYMTAB   2
#define ELF_SHT_STRTAB   3
#define ELF_SHT_RELA     4
#define ELF_SHT_HASH     5
#define ELF_SHT_DYNAMIC  6
#define ELF_SHT_NOTE     7
#define ELF_SHT_NOBITS   8
#define ELF_SHT_REL      9
#define ELF_SHT_SHLIB    10
#define ELF_SHT_DYNSYM   11

// ELF section header flags
#define ELF_SHF_WRITE     0x1
#define ELF_SHF_ALLOC     0x2
#define ELF_SHF_EXECINSTR 0x4
#define ELF_SHF_MASKPROC  0xF0000000

// ELF program header types
#define ELF_PT_NULL    0
#define ELF_PT_LOAD    1
#define ELF_PT_DYNAMIC 2
#define ELF_PT_INTERP  3
#define ELF_PT_NOTE    4
#define ELF_PT_SHLIB   5
#define ELF_PT_PHDR    6
#define ELF_PT_TLS     7

// ELF program header flags
#define ELF_PF_X 0x1
#define ELF_PF_W 0x2
#define ELF_PF_R 0x4

// ELF 64-bit header
typedef struct {
    u8  e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} __attribute__((packed)) elf64_header_t;

// ELF 64-bit section header
typedef struct {
    u32 sh_name;
    u32 sh_type;
    u64 sh_flags;
    u64 sh_addr;
    u64 sh_offset;
    u64 sh_size;
    u32 sh_link;
    u32 sh_info;
    u64 sh_addralign;
    u64 sh_entsize;
} __attribute__((packed)) elf64_section_header_t;

// ELF 64-bit program header
typedef struct {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
} __attribute__((packed)) elf64_program_header_t;

// ELF 64-bit symbol
typedef struct {
    u32 st_name;
    u8  st_info;
    u8  st_other;
    u16 st_shndx;
    u64 st_value;
    u64 st_size;
} __attribute__((packed)) elf64_symbol_t;

// ELF 64-bit relocation
typedef struct {
    u64 r_offset;
    u64 r_info;
    s64 r_addend;
} __attribute__((packed)) elf64_rela_t;

// ELF dynamic entry
typedef struct {
    s64 d_tag;
    u64 d_val;
} __attribute__((packed)) elf64_dyn_t;

// ELF load information
typedef struct {
    u64 entry_point;
    u64 base_address;
    u64 stack_base;
    u64 stack_size;
    u64 heap_base;
    u64 heap_size;
    u64 text_start;
    u64 text_size;
    u64 data_start;
    u64 data_size;
    u64 bss_start;
    u64 bss_size;
} elf_load_info_t;

// ELF loader functions
error_t elf_load_binary(const char* path, process_t* process, elf_load_info_t* load_info);
error_t elf_validate_header(const elf64_header_t* header);
error_t elf_load_segments(const elf64_header_t* header, process_t* process, elf_load_info_t* load_info);
error_t elf_setup_stack(process_t* process, elf_load_info_t* load_info);
error_t elf_setup_heap(process_t* process, elf_load_info_t* load_info);
error_t elf_relocate_symbols(const elf64_header_t* header, process_t* process, elf_load_info_t* load_info);

// ELF utility functions
bool elf_is_valid(const void* data, size_t size);
elf64_header_t* elf_get_header(const void* data);
elf64_program_header_t* elf_get_program_header(const elf64_header_t* header, u32 index);
elf64_section_header_t* elf_get_section_header(const elf64_header_t* header, u32 index);
const char* elf_get_string(const elf64_header_t* header, u32 string_table_index, u32 string_index);
elf64_symbol_t* elf_find_symbol(const elf64_header_t* header, const char* name);

// ELF dynamic linking
error_t elf_load_dependencies(process_t* process, const elf64_header_t* header);
error_t elf_resolve_symbols(process_t* process, const elf64_header_t* header);
error_t elf_apply_relocations(process_t* process, const elf64_header_t* header);

// ELF debugging
void elf_dump_header(const elf64_header_t* header);
void elf_dump_program_headers(const elf64_header_t* header);
void elf_dump_section_headers(const elf64_header_t* header);
void elf_dump_symbols(const elf64_header_t* header);

// ELF error handling
const char* elf_get_error_string(error_t error);
error_t elf_validate_segment(const elf64_program_header_t* phdr, size_t file_size);

// ELF memory management
error_t elf_allocate_segment(process_t* process, const elf64_program_header_t* phdr, u64* address);
error_t elf_load_segment_data(process_t* process, const elf64_program_header_t* phdr, const void* file_data);
error_t elf_clear_bss(process_t* process, const elf64_program_header_t* phdr);

// ELF security
error_t elf_validate_permissions(const elf64_program_header_t* phdr);
error_t elf_check_address_range(process_t* process, u64 address, u64 size);
bool elf_is_executable(const elf64_program_header_t* phdr);

// ELF optimization
error_t elf_optimize_loading(const elf64_header_t* header, elf_load_info_t* load_info);
error_t elf_preload_segments(const elf64_header_t* header, process_t* process);

#endif // ELF_LOADER_H 