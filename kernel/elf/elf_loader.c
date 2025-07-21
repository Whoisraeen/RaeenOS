#include "elf_loader.h"
#include "memory/memory.h"
#include "filesystem/vfs.h"
#include "process/process.h"
#include <string.h>
#include <stdio.h>

// ELF magic number
#define ELF_MAGIC 0x464C457F

// ELF header
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf_header_t;

// Program header
typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} elf_phdr_t;

// Section header
typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} elf_shdr_t;

// Program header types
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

// Section header types
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

// Section flags
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4

// Load an ELF program
error_t elf_load_program(const char* filename, void** entry_point) {
    if (!filename || !entry_point) {
        return E_INVAL;
    }
    
    KINFO("Loading ELF program: %s", filename);
    
    // Open the file
    file_t* file = vfs_open(filename, O_RDONLY, 0);
    if (!file) {
        KERROR("Failed to open file: %s", filename);
        return E_NOENT;
    }
    
    // Read ELF header
    elf_header_t header;
    size_t bytes_read = vfs_read(file, &header, sizeof(header), 0);
    if (bytes_read != sizeof(header)) {
        KERROR("Failed to read ELF header from %s", filename);
        vfs_close(file);
        return E_IO;
    }
    
    // Verify ELF magic
    if (*(uint32_t*)header.e_ident != ELF_MAGIC) {
        KERROR("Invalid ELF magic in %s", filename);
        vfs_close(file);
        return E_INVAL;
    }
    
    // Verify ELF class (64-bit)
    if (header.e_ident[4] != 2) { // ELFCLASS64
        KERROR("Not a 64-bit ELF file: %s", filename);
        vfs_close(file);
        return E_INVAL;
    }
    
    // Verify machine type (x86-64)
    if (header.e_machine != 0x3E) { // EM_X86_64
        KERROR("Not an x86-64 ELF file: %s", filename);
        vfs_close(file);
        return E_INVAL;
    }
    
    KDEBUG("ELF file: type=%u, machine=%u, entry=0x%lx, phoff=0x%lx, phnum=%u",
           header.e_type, header.e_machine, header.e_entry, header.e_phoff, header.e_phnum);
    
    // Load program segments
    error_t result = elf_load_segments(file, &header);
    if (result != SUCCESS) {
        KERROR("Failed to load segments from %s", filename);
        vfs_close(file);
        return result;
    }
    
    // Load sections (for debugging symbols)
    result = elf_load_sections(file, &header);
    if (result != SUCCESS) {
        KWARN("Failed to load sections from %s (continuing anyway)", filename);
    }
    
    // Close file
    vfs_close(file);
    
    // Set entry point
    *entry_point = (void*)header.e_entry;
    
    KINFO("Successfully loaded ELF program: %s (entry=0x%lx)", filename, header.e_entry);
    return SUCCESS;
}

// Load program segments
static error_t elf_load_segments(file_t* file, elf_header_t* header) {
    if (!file || !header) {
        return E_INVAL;
    }
    
    KDEBUG("Loading %u program segments", header->e_phnum);
    
    // Read program headers
    elf_phdr_t* phdrs = (elf_phdr_t*)memory_alloc(sizeof(elf_phdr_t) * header->e_phnum);
    if (!phdrs) {
        return E_NOMEM;
    }
    
    size_t bytes_read = vfs_read(file, phdrs, sizeof(elf_phdr_t) * header->e_phnum, header->e_phoff);
    if (bytes_read != sizeof(elf_phdr_t) * header->e_phnum) {
        KERROR("Failed to read program headers");
        memory_free(phdrs);
        return E_IO;
    }
    
    // Process each program header
    for (uint16_t i = 0; i < header->e_phnum; i++) {
        elf_phdr_t* phdr = &phdrs[i];
        
        KDEBUG("Program header %u: type=%u, vaddr=0x%lx, memsz=%lu, filesz=%lu",
               i, phdr->p_type, phdr->p_vaddr, phdr->p_memsz, phdr->p_filesz);
        
        // Only load PT_LOAD segments
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        
        // Calculate alignment
        uint64_t align = phdr->p_align;
        if (align == 0) {
            align = 0x1000; // Default to 4KB
        }
        
        // Align virtual address
        uint64_t vaddr = phdr->p_vaddr & ~(align - 1);
        uint64_t offset = phdr->p_vaddr - vaddr;
        
        // Allocate memory
        void* memory = memory_alloc_aligned(phdr->p_memsz + offset, align);
        if (!memory) {
            KERROR("Failed to allocate memory for segment %u", i);
            memory_free(phdrs);
            return E_NOMEM;
        }
        
        // Clear memory
        memset(memory, 0, phdr->p_memsz + offset);
        
        // Read file data
        if (phdr->p_filesz > 0) {
            bytes_read = vfs_read(file, (char*)memory + offset, phdr->p_filesz, phdr->p_offset);
            if (bytes_read != phdr->p_filesz) {
                KERROR("Failed to read segment data");
                memory_free(memory);
                memory_free(phdrs);
                return E_IO;
            }
        }
        
        // Map memory to virtual address
        error_t result = memory_map_virtual(vaddr, (uintptr_t)memory, phdr->p_memsz + offset, 
                                           phdr->p_flags);
        if (result != SUCCESS) {
            KERROR("Failed to map virtual memory for segment %u", i);
            memory_free(memory);
            memory_free(phdrs);
            return result;
        }
        
        KDEBUG("Mapped segment %u: vaddr=0x%lx -> phys=0x%lx, size=%lu", 
               i, vaddr, (uintptr_t)memory, phdr->p_memsz + offset);
    }
    
    memory_free(phdrs);
    return SUCCESS;
}

// Load sections (for debugging)
static error_t elf_load_sections(file_t* file, elf_header_t* header) {
    if (!file || !header || header->e_shnum == 0) {
        return SUCCESS;
    }
    
    KDEBUG("Loading %u sections", header->e_shnum);
    
    // Read section headers
    elf_shdr_t* shdrs = (elf_shdr_t*)memory_alloc(sizeof(elf_shdr_t) * header->e_shnum);
    if (!shdrs) {
        return E_NOMEM;
    }
    
    size_t bytes_read = vfs_read(file, shdrs, sizeof(elf_shdr_t) * header->e_shnum, header->e_shoff);
    if (bytes_read != sizeof(elf_shdr_t) * header->e_shnum) {
        KERROR("Failed to read section headers");
        memory_free(shdrs);
        return E_IO;
    }
    
    // Process sections (for debugging information)
    for (uint16_t i = 0; i < header->e_shnum; i++) {
        elf_shdr_t* shdr = &shdrs[i];
        
        // Only process sections that are loaded in memory
        if (shdr->sh_type == SHT_PROGBITS && (shdr->sh_flags & SHF_ALLOC)) {
            KDEBUG("Section %u: name=%u, addr=0x%lx, size=%lu, flags=0x%lx",
                   i, shdr->sh_name, shdr->sh_addr, shdr->sh_size, shdr->sh_flags);
        }
    }
    
    memory_free(shdrs);
    return SUCCESS;
}

// Validate ELF file
error_t elf_validate_file(const char* filename) {
    if (!filename) {
        return E_INVAL;
    }
    
    // Open the file
    file_t* file = vfs_open(filename, O_RDONLY, 0);
    if (!file) {
        return E_NOENT;
    }
    
    // Read ELF header
    elf_header_t header;
    size_t bytes_read = vfs_read(file, &header, sizeof(header), 0);
    vfs_close(file);
    
    if (bytes_read != sizeof(header)) {
        return E_IO;
    }
    
    // Check magic number
    if (*(uint32_t*)header.e_ident != ELF_MAGIC) {
        return E_INVAL;
    }
    
    // Check ELF class
    if (header.e_ident[4] != 2) { // ELFCLASS64
        return E_INVAL;
    }
    
    // Check machine type
    if (header.e_machine != 0x3E) { // EM_X86_64
        return E_INVAL;
    }
    
    return SUCCESS;
}

// Get ELF entry point
error_t elf_get_entry_point(const char* filename, void** entry_point) {
    if (!filename || !entry_point) {
        return E_INVAL;
    }
    
    // Open the file
    file_t* file = vfs_open(filename, O_RDONLY, 0);
    if (!file) {
        return E_NOENT;
    }
    
    // Read ELF header
    elf_header_t header;
    size_t bytes_read = vfs_read(file, &header, sizeof(header), 0);
    vfs_close(file);
    
    if (bytes_read != sizeof(header)) {
        return E_IO;
    }
    
    // Validate ELF
    if (*(uint32_t*)header.e_ident != ELF_MAGIC || 
        header.e_ident[4] != 2 || 
        header.e_machine != 0x3E) {
        return E_INVAL;
    }
    
    *entry_point = (void*)header.e_entry;
    return SUCCESS;
}

// Initialize ELF loader
error_t elf_loader_init(void) {
    KINFO("Initializing ELF loader");
    
    // TODO: Initialize any ELF loader specific data structures
    
    KINFO("ELF loader initialized");
    return SUCCESS;
}

// Shutdown ELF loader
void elf_loader_shutdown(void) {
    KINFO("Shutting down ELF loader");
    
    // TODO: Clean up ELF loader resources
    
    KINFO("ELF loader shutdown complete");
} 