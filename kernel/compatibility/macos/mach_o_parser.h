#ifndef MACH_O_PARSER_H
#define MACH_O_PARSER_H

#include "types.h"
#include <stddef.h>

// Mach-O magic values
#define MH_MAGIC    0xFEEDFACE  // 32-bit
#define MH_CIGAM    0xCEFAEDFE  // 32-bit swapped
#define MH_MAGIC_64 0xFEEDFACF  // 64-bit
#define MH_CIGAM_64 0xCFFAEDFE  // 64-bit swapped

// CPU types
#define CPU_TYPE_X86     7
#define CPU_TYPE_X86_64  0x1000007
#define CPU_TYPE_ARM     12
#define CPU_TYPE_ARM64   0x100000C

// File types
#define MH_OBJECT     0x1
#define MH_EXECUTE    0x2
#define MH_FVMLIB     0x3
#define MH_CORE       0x4
#define MH_PRELOAD    0x5
#define MH_DYLIB      0x6
#define MH_DYLINKER   0x7
#define MH_BUNDLE     0x8
#define MH_DYLIB_STUB 0x9
#define MH_DSYM       0xA
#define MH_KEXT_BUNDLE 0xB

// Load command types
#define LC_SEGMENT        0x1
#define LC_SYMTAB         0x2
#define LC_SYMSEG         0x3
#define LC_THREAD         0x4
#define LC_UNIXTHREAD     0x5
#define LC_LOADFVMLIB     0x6
#define LC_IDFVMLIB       0x7
#define LC_IDENT          0x8
#define LC_FVMFILE        0x9
#define LC_PREPAGE        0xA
#define LC_DYSYMTAB       0xB
#define LC_LOAD_DYLIB     0xC
#define LC_ID_DYLIB       0xD
#define LC_LOAD_DYLINKER  0xE
#define LC_ID_DYLINKER    0xF
#define LC_PREBOUND_DYLIB 0x10
#define LC_ROUTINES       0x11
#define LC_SUB_FRAMEWORK  0x12
#define LC_SUB_UMBRELLA   0x13
#define LC_SUB_CLIENT     0x14
#define LC_SUB_LIBRARY    0x15
#define LC_TWOLEVEL_HINTS 0x16
#define LC_PREBIND_CKSUM  0x17
#define LC_LOAD_WEAK_DYLIB 0x18
#define LC_SEGMENT_64     0x19
#define LC_ROUTINES_64    0x1A
#define LC_UUID           0x1B
#define LC_RPATH          0x1C
#define LC_CODE_SIGNATURE 0x1D
#define LC_SEGMENT_SPLIT_INFO 0x1E
#define LC_REEXPORT_DYLIB 0x1F
#define LC_LAZY_LOAD_DYLIB 0x20
#define LC_ENCRYPTION_INFO 0x21
#define LC_DYLD_INFO      0x22
#define LC_DYLD_INFO_ONLY 0x22
#define LC_LOAD_UPWARD_DYLIB 0x23
#define LC_VERSION_MIN_MACOSX 0x24
#define LC_VERSION_MIN_IPHONEOS 0x25
#define LC_FUNCTION_STARTS 0x26
#define LC_DYLD_ENVIRONMENT 0x27
#define LC_MAIN           0x28
#define LC_DATA_IN_CODE   0x29
#define LC_SOURCE_VERSION 0x2A
#define LC_DYLIB_CODE_SIGN_DRS 0x2B
#define LC_ENCRYPTION_INFO_64 0x2C
#define LC_LINKER_OPTION  0x2D
#define LC_LINKER_OPTIMIZATION_HINT 0x2E
#define LC_VERSION_MIN_TVOS 0x2F
#define LC_VERSION_MIN_WATCHOS 0x30
#define LC_NOTE           0x31
#define LC_BUILD_VERSION  0x32

// Section flags
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#define S_ATTR_NO_TOC            0x40000000
#define S_ATTR_STRIP_STATIC_SYMS 0x20000000
#define S_ATTR_NO_DEAD_STRIP     0x10000000
#define S_ATTR_LIVE_SUPPORT      0x08000000
#define S_ATTR_SELF_MODIFYING_CODE 0x04000000
#define S_ATTR_DEBUG             0x02000000
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400
#define S_ATTR_EXT_RELOC         0x00000200
#define S_ATTR_LOC_RELOC         0x00000100

// Mach-O header (32-bit)
typedef struct {
    u32 magic;
    u32 cputype;
    u32 cpusubtype;
    u32 filetype;
    u32 ncmds;
    u32 sizeofcmds;
    u32 flags;
} mach_header_t;

// Mach-O header (64-bit)
typedef struct {
    u32 magic;
    u32 cputype;
    u32 cpusubtype;
    u32 filetype;
    u32 ncmds;
    u32 sizeofcmds;
    u32 flags;
    u32 reserved;
} mach_header_64_t;

// Load command header
typedef struct {
    u32 cmd;
    u32 cmdsize;
} load_command_t;

// Segment command (32-bit)
typedef struct {
    u32 cmd;
    u32 cmdsize;
    char segname[16];
    u32 vmaddr;
    u32 vmsize;
    u32 fileoff;
    u32 filesize;
    u32 maxprot;
    u32 initprot;
    u32 nsects;
    u32 flags;
} segment_command_t;

// Segment command (64-bit)
typedef struct {
    u32 cmd;
    u32 cmdsize;
    char segname[16];
    u64 vmaddr;
    u64 vmsize;
    u64 fileoff;
    u64 filesize;
    u32 maxprot;
    u32 initprot;
    u32 nsects;
    u32 flags;
} segment_command_64_t;

// Section (32-bit)
typedef struct {
    char sectname[16];
    char segname[16];
    u32 addr;
    u32 size;
    u32 offset;
    u32 align;
    u32 reloff;
    u32 nreloc;
    u32 flags;
    u32 reserved1;
    u32 reserved2;
} section_t;

// Section (64-bit)
typedef struct {
    char sectname[16];
    char segname[16];
    u64 addr;
    u64 size;
    u32 offset;
    u32 align;
    u32 reloff;
    u32 nreloc;
    u32 flags;
    u32 reserved1;
    u32 reserved2;
    u32 reserved3;
} section_64_t;

// Dynamic library command
typedef struct {
    u32 cmd;
    u32 cmdsize;
    struct dylib {
        u32 name;
        u32 timestamp;
        u32 current_version;
        u32 compatibility_version;
    } dylib;
} dylib_command_t;

// Symbol table command
typedef struct {
    u32 cmd;
    u32 cmdsize;
    u32 symoff;
    u32 nsyms;
    u32 stroff;
    u32 strsize;
} symtab_command_t;

// Dynamic symbol table command
typedef struct {
    u32 cmd;
    u32 cmdsize;
    u32 ilocalsym;
    u32 nlocalsym;
    u32 iextdefsym;
    u32 nextdefsym;
    u32 iundefsym;
    u32 nundefsym;
    u32 tocoff;
    u32 ntoc;
    u32 modtaboff;
    u32 nmodtab;
    u32 extrefsymoff;
    u32 nextrefsyms;
    u32 indirectsymoff;
    u32 nindirectsyms;
    u32 extreloff;
    u32 nextrel;
    u32 locreloff;
    u32 nlocrel;
} dysymtab_command_t;

// UUID command
typedef struct {
    u32 cmd;
    u32 cmdsize;
    u8 uuid[16];
} uuid_command_t;

// Code signature command
typedef struct {
    u32 cmd;
    u32 cmdsize;
    u32 dataoff;
    u32 datasize;
} codesignature_command_t;

// Mach-O file information structure
typedef struct {
    union {
        mach_header_t header32;
        mach_header_64_t header64;
    } header;
    bool is_64bit;
    bool is_swapped;
    load_command_t* load_commands;
    u32 number_of_load_commands;
    u64 entry_point;
    u64 image_base;
    u64 image_size;
} mach_o_file_info_t;

// Segment information
typedef struct {
    char name[16];
    u64 virtual_address;
    u64 virtual_size;
    u64 file_offset;
    u64 file_size;
    u32 max_protection;
    u32 init_protection;
    u32 flags;
    section_t* sections;
    u32 number_of_sections;
} mach_o_segment_info_t;

// Import information
typedef struct {
    char* library_name;
    char** symbol_names;
    u32* symbol_ordinals;
    u32 number_of_symbols;
} mach_o_import_info_t;

// Export information
typedef struct {
    char* module_name;
    char** symbol_names;
    u64* symbol_addresses;
    u32* symbol_ordinals;
    u32 number_of_symbols;
} mach_o_export_info_t;

// Mach-O parser functions
error_t mach_o_parse_file(const char* file_path, mach_o_file_info_t* info);
error_t mach_o_parse_memory(const void* data, size_t size, mach_o_file_info_t* info);
error_t mach_o_validate_header(const mach_o_file_info_t* info);
error_t mach_o_get_segments(const mach_o_file_info_t* info, mach_o_segment_info_t* segments, u32* num_segments);
error_t mach_o_get_imports(const mach_o_file_info_t* info, mach_o_import_info_t* imports, u32* num_imports);
error_t mach_o_get_exports(const mach_o_file_info_t* info, mach_o_export_info_t* exports);
error_t mach_o_get_section_by_name(const mach_o_file_info_t* info, const char* segname, const char* sectname, section_t* section);
error_t mach_o_get_section_by_address(const mach_o_file_info_t* info, u64 address, section_t* section);
void mach_o_free_info(mach_o_file_info_t* info);
void mach_o_free_segments(mach_o_segment_info_t* segments, u32 num_segments);
void mach_o_free_imports(mach_o_import_info_t* imports, u32 num_imports);
void mach_o_free_exports(mach_o_export_info_t* exports);

// Utility functions
bool mach_o_is_valid_mach_o_file(const char* file_path);
bool mach_o_is_64bit(const mach_o_file_info_t* info);
bool mach_o_is_executable(const mach_o_file_info_t* info);
bool mach_o_is_dylib(const mach_o_file_info_t* info);
bool mach_o_is_bundle(const mach_o_file_info_t* info);
u64 mach_o_vm_to_file_offset(const mach_o_file_info_t* info, u64 vm_address);
u64 mach_o_file_to_vm_offset(const mach_o_file_info_t* info, u64 file_offset);

// CPU architecture detection
const char* mach_o_get_cpu_type_string(u32 cputype);
const char* mach_o_get_file_type_string(u32 filetype);

#endif // MACH_O_PARSER_H 