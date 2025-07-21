#include "elf_loader.h"
#include "kernel.h"
#include "filesystem/vfs.h"
#include "memory/memory.h"
#include <string.h>

// ELF loader error codes
#define ELF_ERROR_INVALID_MAGIC     (E_CUSTOM + 1)
#define ELF_ERROR_UNSUPPORTED_CLASS (E_CUSTOM + 2)
#define ELF_ERROR_UNSUPPORTED_MACHINE (E_CUSTOM + 3)
#define ELF_ERROR_INVALID_TYPE      (E_CUSTOM + 4)
#define ELF_ERROR_LOAD_FAILED       (E_CUSTOM + 5)
#define ELF_ERROR_INVALID_SEGMENT   (E_CUSTOM + 6)

error_t elf_load_binary(const char* path, process_t* process, elf_load_info_t* load_info) {
    if (!path || !process || !load_info) {
        return E_INVAL;
    }
    
    KINFO("Loading ELF binary: %s", path);
    
    // Open and read the file
    file_handle_t file;
    error_t result = vfs_open(path, O_RDONLY, &file);
    if (result != SUCCESS) {
        KERROR("Failed to open ELF file: %s", path);
        return result;
    }
    
    // Read ELF header
    elf64_header_t header;
    size_t bytes_read = vfs_read(&file, &header, sizeof(header));
    if (bytes_read != sizeof(header)) {
        KERROR("Failed to read ELF header");
        vfs_close(&file);
        return E_IO;
    }
    
    // Validate ELF header
    result = elf_validate_header(&header);
    if (result != SUCCESS) {
        KERROR("Invalid ELF header");
        vfs_close(&file);
        return result;
    }
    
    // Initialize load info
    memset(load_info, 0, sizeof(elf_load_info_t));
    load_info->entry_point = header.e_entry;
    
    // Load program segments
    result = elf_load_segments(&header, process, load_info);
    if (result != SUCCESS) {
        KERROR("Failed to load ELF segments");
        vfs_close(&file);
        return result;
    }
    
    // Setup stack
    result = elf_setup_stack(process, load_info);
    if (result != SUCCESS) {
        KERROR("Failed to setup stack");
        vfs_close(&file);
        return result;
    }
    
    // Setup heap
    result = elf_setup_heap(process, load_info);
    if (result != SUCCESS) {
        KERROR("Failed to setup heap");
        vfs_close(&file);
        return result;
    }
    
    // Relocate symbols if needed
    if (header.e_type == ELF_TYPE_DYN) {
        result = elf_relocate_symbols(&header, process, load_info);
        if (result != SUCCESS) {
            KERROR("Failed to relocate symbols");
            vfs_close(&file);
            return result;
        }
    }
    
    vfs_close(&file);
    
    KINFO("ELF binary loaded successfully");
    KINFO("  Entry point: 0x%llx", load_info->entry_point);
    KINFO("  Stack base: 0x%llx", load_info->stack_base);
    KINFO("  Heap base: 0x%llx", load_info->heap_base);
    
    return SUCCESS;
}

error_t elf_validate_header(const elf64_header_t* header) {
    if (!header) {
        return E_INVAL;
    }
    
    // Check magic number
    if (*(u32*)header->e_ident != ELF_MAGIC) {
        KERROR("Invalid ELF magic number");
        return ELF_ERROR_INVALID_MAGIC;
    }
    
    // Check file class
    if (header->e_ident[4] != ELF_CLASS_64) {
        KERROR("Unsupported ELF class: %u", header->e_ident[4]);
        return ELF_ERROR_UNSUPPORTED_CLASS;
    }
    
    // Check data encoding
    if (header->e_ident[5] != ELF_DATA_LSB) {
        KERROR("Unsupported ELF data encoding: %u", header->e_ident[5]);
        return ELF_ERROR_UNSUPPORTED_CLASS;
    }
    
    // Check version
    if (header->e_version != ELF_VERSION_CURRENT) {
        KERROR("Unsupported ELF version: %u", header->e_version);
        return ELF_ERROR_UNSUPPORTED_CLASS;
    }
    
    // Check machine type
    if (header->e_machine != ELF_MACHINE_X86_64) {
        KERROR("Unsupported ELF machine: %u", header->e_machine);
        return ELF_ERROR_UNSUPPORTED_MACHINE;
    }
    
    // Check object file type
    if (header->e_type != ELF_TYPE_EXEC && header->e_type != ELF_TYPE_DYN) {
        KERROR("Unsupported ELF type: %u", header->e_type);
        return ELF_ERROR_INVALID_TYPE;
    }
    
    return SUCCESS;
}

error_t elf_load_segments(const elf64_header_t* header, process_t* process, elf_load_info_t* load_info) {
    if (!header || !process || !load_info) {
        return E_INVAL;
    }
    
    // Get file data
    file_handle_t file;
    error_t result = vfs_open("/proc/current_elf", O_RDONLY, &file);
    if (result != SUCCESS) {
        return result;
    }
    
    // Load each program header
    for (u16 i = 0; i < header->e_phnum; i++) {
        elf64_program_header_t phdr;
        
        // Read program header
        vfs_seek(&file, header->e_phoff + i * sizeof(phdr), SEEK_SET);
        size_t bytes_read = vfs_read(&file, &phdr, sizeof(phdr));
        if (bytes_read != sizeof(phdr)) {
            KERROR("Failed to read program header %u", i);
            vfs_close(&file);
            return E_IO;
        }
        
        // Only load PT_LOAD segments
        if (phdr.p_type != ELF_PT_LOAD) {
            continue;
        }
        
        // Validate segment
        result = elf_validate_segment(&phdr, file.size);
        if (result != SUCCESS) {
            KERROR("Invalid program header %u", i);
            vfs_close(&file);
            return result;
        }
        
        // Allocate memory for segment
        u64 segment_address;
        result = elf_allocate_segment(process, &phdr, &segment_address);
        if (result != SUCCESS) {
            KERROR("Failed to allocate segment %u", i);
            vfs_close(&file);
            return result;
        }
        
        // Load segment data
        result = elf_load_segment_data(process, &phdr, &file);
        if (result != SUCCESS) {
            KERROR("Failed to load segment data %u", i);
            vfs_close(&file);
            return result;
        }
        
        // Clear BSS if needed
        if (phdr.p_memsz > phdr.p_filesz) {
            result = elf_clear_bss(process, &phdr);
            if (result != SUCCESS) {
                KERROR("Failed to clear BSS for segment %u", i);
                vfs_close(&file);
                return result;
            }
        }
        
        // Update load info
        if (phdr.p_flags & ELF_PF_X) {
            // Executable segment (text)
            if (load_info->text_start == 0) {
                load_info->text_start = segment_address;
                load_info->text_size = phdr.p_memsz;
            }
        } else {
            // Data segment
            if (load_info->data_start == 0) {
                load_info->data_start = segment_address;
                load_info->data_size = phdr.p_memsz;
            }
        }
    }
    
    vfs_close(&file);
    return SUCCESS;
}

error_t elf_setup_stack(process_t* process, elf_load_info_t* load_info) {
    if (!process || !load_info) {
        return E_INVAL;
    }
    
    // Allocate stack space (8MB default)
    load_info->stack_size = 8 * 1024 * 1024;
    load_info->stack_base = 0x7FFFFFFFFFFF - load_info->stack_size;
    
    // Map stack pages
    for (u64 addr = load_info->stack_base; addr < load_info->stack_base + load_info->stack_size; addr += PAGE_SIZE) {
        error_t result = memory_map_page(process->address_space, addr, 
                                        MEMORY_USER | MEMORY_WRITE | MEMORY_PRESENT);
        if (result != SUCCESS) {
            KERROR("Failed to map stack page at 0x%llx", addr);
            return result;
        }
    }
    
    // Set initial stack pointer (top of stack, aligned to 16 bytes)
    process->registers.rsp = load_info->stack_base + load_info->stack_size - 16;
    process->registers.rsp &= ~0xF;
    
    KINFO("Stack setup: base=0x%llx, size=%llu, sp=0x%llx", 
          load_info->stack_base, load_info->stack_size, process->registers.rsp);
    
    return SUCCESS;
}

error_t elf_setup_heap(process_t* process, elf_load_info_t* load_info) {
    if (!process || !load_info) {
        return E_INVAL;
    }
    
    // Allocate initial heap space (1MB default)
    load_info->heap_size = 1024 * 1024;
    load_info->heap_base = load_info->data_start + load_info->data_size;
    
    // Align heap base to page boundary
    load_info->heap_base = (load_info->heap_base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // Map initial heap pages
    for (u64 addr = load_info->heap_base; addr < load_info->heap_base + load_info->heap_size; addr += PAGE_SIZE) {
        error_t result = memory_map_page(process->address_space, addr, 
                                        MEMORY_USER | MEMORY_WRITE | MEMORY_PRESENT);
        if (result != SUCCESS) {
            KERROR("Failed to map heap page at 0x%llx", addr);
            return result;
        }
    }
    
    KINFO("Heap setup: base=0x%llx, size=%llu", load_info->heap_base, load_info->heap_size);
    
    return SUCCESS;
}

error_t elf_allocate_segment(process_t* process, const elf64_program_header_t* phdr, u64* address) {
    if (!process || !phdr || !address) {
        return E_INVAL;
    }
    
    // Calculate segment size (aligned to page boundary)
    u64 segment_size = (phdr->p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // Allocate virtual address space
    u64 segment_address = memory_alloc_virtual(process->address_space, segment_size);
    if (segment_address == 0) {
        KERROR("Failed to allocate virtual address space for segment");
        return E_NOMEM;
    }
    
    // Map pages for the segment
    u32 flags = MEMORY_USER | MEMORY_PRESENT;
    if (phdr->p_flags & ELF_PF_W) flags |= MEMORY_WRITE;
    if (phdr->p_flags & ELF_PF_X) flags |= MEMORY_EXECUTE;
    
    for (u64 addr = segment_address; addr < segment_address + segment_size; addr += PAGE_SIZE) {
        error_t result = memory_map_page(process->address_space, addr, flags);
        if (result != SUCCESS) {
            KERROR("Failed to map segment page at 0x%llx", addr);
            return result;
        }
    }
    
    *address = segment_address;
    return SUCCESS;
}

error_t elf_load_segment_data(process_t* process, const elf64_program_header_t* phdr, const file_handle_t* file) {
    if (!process || !phdr || !file) {
        return E_INVAL;
    }
    
    // Seek to segment data
    error_t result = vfs_seek(file, phdr->p_offset, SEEK_SET);
    if (result != SUCCESS) {
        return result;
    }
    
    // Read segment data
    u8* buffer = memory_alloc(phdr->p_filesz);
    if (!buffer) {
        return E_NOMEM;
    }
    
    size_t bytes_read = vfs_read(file, buffer, phdr->p_filesz);
    if (bytes_read != phdr->p_filesz) {
        memory_free(buffer);
        return E_IO;
    }
    
    // Copy data to process address space
    result = memory_copy_to_process(process->address_space, phdr->p_vaddr, buffer, phdr->p_filesz);
    memory_free(buffer);
    
    return result;
}

error_t elf_clear_bss(process_t* process, const elf64_program_header_t* phdr) {
    if (!process || !phdr) {
        return E_INVAL;
    }
    
    // Calculate BSS size
    u64 bss_size = phdr->p_memsz - phdr->p_filesz;
    if (bss_size == 0) {
        return SUCCESS;
    }
    
    // Clear BSS section
    u64 bss_start = phdr->p_vaddr + phdr->p_filesz;
    u8* zero_buffer = memory_alloc(PAGE_SIZE);
    if (!zero_buffer) {
        return E_NOMEM;
    }
    
    memset(zero_buffer, 0, PAGE_SIZE);
    
    for (u64 offset = 0; offset < bss_size; offset += PAGE_SIZE) {
        u64 size = (offset + PAGE_SIZE > bss_size) ? (bss_size - offset) : PAGE_SIZE;
        error_t result = memory_copy_to_process(process->address_space, bss_start + offset, zero_buffer, size);
        if (result != SUCCESS) {
            memory_free(zero_buffer);
            return result;
        }
    }
    
    memory_free(zero_buffer);
    return SUCCESS;
}

error_t elf_validate_segment(const elf64_program_header_t* phdr, size_t file_size) {
    if (!phdr) {
        return E_INVAL;
    }
    
    // Check if segment fits in file
    if (phdr->p_offset + phdr->p_filesz > file_size) {
        KERROR("Segment extends beyond file size");
        return ELF_ERROR_INVALID_SEGMENT;
    }
    
    // Check alignment
    if (phdr->p_align != 0 && (phdr->p_vaddr % phdr->p_align) != 0) {
        KERROR("Invalid segment alignment");
        return ELF_ERROR_INVALID_SEGMENT;
    }
    
    return SUCCESS;
}

bool elf_is_valid(const void* data, size_t size) {
    if (!data || size < sizeof(elf64_header_t)) {
        return false;
    }
    
    const elf64_header_t* header = (const elf64_header_t*)data;
    return *(u32*)header->e_ident == ELF_MAGIC;
}

elf64_header_t* elf_get_header(const void* data) {
    if (!elf_is_valid(data, sizeof(elf64_header_t))) {
        return NULL;
    }
    return (elf64_header_t*)data;
}

const char* elf_get_error_string(error_t error) {
    switch (error) {
        case ELF_ERROR_INVALID_MAGIC:
            return "Invalid ELF magic number";
        case ELF_ERROR_UNSUPPORTED_CLASS:
            return "Unsupported ELF class";
        case ELF_ERROR_UNSUPPORTED_MACHINE:
            return "Unsupported ELF machine";
        case ELF_ERROR_INVALID_TYPE:
            return "Invalid ELF type";
        case ELF_ERROR_LOAD_FAILED:
            return "ELF load failed";
        case ELF_ERROR_INVALID_SEGMENT:
            return "Invalid ELF segment";
        default:
            return "Unknown ELF error";
    }
}

void elf_dump_header(const elf64_header_t* header) {
    if (!header) {
        return;
    }
    
    KINFO("ELF Header:");
    KINFO("  Magic: %02x %02x %02x %02x", 
          header->e_ident[0], header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    KINFO("  Class: %u", header->e_ident[4]);
    KINFO("  Data: %u", header->e_ident[5]);
    KINFO("  Version: %u", header->e_ident[6]);
    KINFO("  OS/ABI: %u", header->e_ident[7]);
    KINFO("  Type: %u", header->e_type);
    KINFO("  Machine: %u", header->e_machine);
    KINFO("  Version: %u", header->e_version);
    KINFO("  Entry: 0x%llx", header->e_entry);
    KINFO("  Program headers: %u", header->e_phnum);
    KINFO("  Section headers: %u", header->e_shnum);
} 