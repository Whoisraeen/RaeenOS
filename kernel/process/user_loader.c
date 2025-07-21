#include "kernel.h"
#include "process/process.h"
#include "memory/memory.h"
#include "hal/hal.h"
#include <string.h>

// User program loader configuration
#define USER_STACK_SIZE (64 * 1024)  // 64KB user stack
#define USER_HEAP_SIZE (1024 * 1024) // 1MB user heap
#define USER_CODE_BASE 0x400000      // User code starts at 4MB
#define USER_STACK_BASE 0x800000     // User stack starts at 8MB
#define USER_HEAP_BASE 0x900000      // User heap starts at 9MB

// Flat binary header structure
typedef struct {
    u32 magic;           // Magic number to identify flat binary
    u32 entry_point;     // Entry point offset from start
    u32 code_size;       // Size of code section
    u32 data_size;       // Size of data section
    u32 bss_size;        // Size of BSS section
    u32 stack_size;      // Required stack size
    u32 flags;           // Program flags
} __attribute__((packed)) flat_binary_header_t;

// Flat binary magic number
#define FLAT_BINARY_MAGIC 0x464C4154  // "FLAT"

// User program flags
#define FLAT_FLAG_32BIT    0x00000001
#define FLAT_FLAG_64BIT    0x00000002
#define FLAT_FLAG_STATIC   0x00000004
#define FLAT_FLAG_DYNAMIC  0x00000008

// User program context
typedef struct {
    u64 entry_point;     // Program entry point
    u64 stack_pointer;   // Initial stack pointer
    u64 heap_start;      // Heap start address
    u64 heap_end;        // Heap end address
    u32 code_pages;      // Number of code pages
    u32 data_pages;      // Number of data pages
    u32 stack_pages;     // Number of stack pages
    bool loaded;         // Whether program is loaded
} user_program_t;

// Global user program state
static user_program_t current_user_program = {0};
static bool user_loader_initialized = false;

// Forward declarations
static error_t user_loader_validate_binary(const void* binary, size_t size);
static error_t user_loader_parse_header(const void* binary, flat_binary_header_t* header);
static error_t user_loader_allocate_address_space(user_program_t* program);
static error_t user_loader_load_sections(const void* binary, flat_binary_header_t* header, user_program_t* program);
static error_t user_loader_setup_stack(user_program_t* program);
static error_t user_loader_setup_heap(user_program_t* program);
static error_t user_loader_setup_page_tables(user_program_t* program);
static error_t user_loader_validate_user_pointer(const void* ptr, size_t size);

// Initialize user program loader
error_t user_loader_init(void) {
    KINFO("Initializing user program loader");
    
    // Initialize user program state
    memset(&current_user_program, 0, sizeof(user_program_t));
    
    user_loader_initialized = true;
    
    KINFO("User program loader initialized");
    return SUCCESS;
}

// Load a flat binary program
error_t user_loader_load_program(const void* binary, size_t size, u32* process_id) {
    if (!user_loader_initialized || !binary || size == 0) {
        return E_INVAL;
    }
    
    KINFO("Loading user program (size: %zu bytes)", size);
    
    // Validate binary format
    error_t result = user_loader_validate_binary(binary, size);
    if (result != SUCCESS) {
        KERROR("Invalid binary format");
        return result;
    }
    
    // Parse binary header
    flat_binary_header_t header;
    result = user_loader_parse_header(binary, &header);
    if (result != SUCCESS) {
        KERROR("Failed to parse binary header");
        return result;
    }
    
    KINFO("Program: entry=0x%x, code=%u, data=%u, bss=%u, stack=%u",
          header.entry_point, header.code_size, header.data_size, 
          header.bss_size, header.stack_size);
    
    // Allocate address space for the program
    result = user_loader_allocate_address_space(&current_user_program);
    if (result != SUCCESS) {
        KERROR("Failed to allocate address space");
        return result;
    }
    
    // Load program sections
    result = user_loader_load_sections(binary, &header, &current_user_program);
    if (result != SUCCESS) {
        KERROR("Failed to load program sections");
        return result;
    }
    
    // Setup stack
    result = user_loader_setup_stack(&current_user_program);
    if (result != SUCCESS) {
        KERROR("Failed to setup stack");
        return result;
    }
    
    // Setup heap
    result = user_loader_setup_heap(&current_user_program);
    if (result != SUCCESS) {
        KERROR("Failed to setup heap");
        return result;
    }
    
    // Setup page tables
    result = user_loader_setup_page_tables(&current_user_program);
    if (result != SUCCESS) {
        KERROR("Failed to setup page tables");
        return result;
    }
    
    // Create process
    process_t* process = process_create("user_program", 1000, 1000); // Default user
    if (!process) {
        KERROR("Failed to create user process");
        return E_NOMEM;
    }
    
    // Set process address space
    process->user_program = current_user_program;
    process->state = PROCESS_READY;
    
    if (process_id) {
        *process_id = process->pid;
    }
    
    current_user_program.loaded = true;
    
    KINFO("User program loaded successfully (PID: %u)", process->pid);
    return SUCCESS;
}

// Execute user program
error_t user_loader_execute_program(u32 process_id) {
    if (!user_loader_initialized || !current_user_program.loaded) {
        return E_INVAL;
    }
    
    process_t* process = process_get_by_id(process_id);
    if (!process) {
        return E_NOENT;
    }
    
    KINFO("Executing user program (PID: %u)", process_id);
    
    // Switch to user mode
    user_mode_entry(&process->user_program);
    
    return SUCCESS;
}

// Validate flat binary format
static error_t user_loader_validate_binary(const void* binary, size_t size) {
    if (!binary || size < sizeof(flat_binary_header_t)) {
        return E_INVAL;
    }
    
    const flat_binary_header_t* header = (const flat_binary_header_t*)binary;
    
    // Check magic number
    if (header->magic != FLAT_BINARY_MAGIC) {
        KERROR("Invalid magic number: 0x%x", header->magic);
        return E_INVAL;
    }
    
    // Check sizes
    if (header->code_size > size - sizeof(flat_binary_header_t)) {
        KERROR("Code size too large: %u", header->code_size);
        return E_INVAL;
    }
    
    if (header->data_size > size - sizeof(flat_binary_header_t) - header->code_size) {
        KERROR("Data size too large: %u", header->data_size);
        return E_INVAL;
    }
    
    // Check entry point
    if (header->entry_point >= header->code_size) {
        KERROR("Invalid entry point: 0x%x", header->entry_point);
        return E_INVAL;
    }
    
    return SUCCESS;
}

// Parse flat binary header
static error_t user_loader_parse_header(const void* binary, flat_binary_header_t* header) {
    if (!binary || !header) {
        return E_INVAL;
    }
    
    memcpy(header, binary, sizeof(flat_binary_header_t));
    
    // Validate header fields
    if (header->magic != FLAT_BINARY_MAGIC) {
        return E_INVAL;
    }
    
    return SUCCESS;
}

// Allocate address space for user program
static error_t user_loader_allocate_address_space(user_program_t* program) {
    if (!program) {
        return E_INVAL;
    }
    
    // Calculate required pages
    program->code_pages = (4096 + sizeof(flat_binary_header_t) - 1) / 4096; // At least 1 page
    program->data_pages = (4096 + sizeof(flat_binary_header_t) - 1) / 4096; // At least 1 page
    program->stack_pages = USER_STACK_SIZE / 4096;
    
    // Set addresses
    program->entry_point = USER_CODE_BASE;
    program->stack_pointer = USER_STACK_BASE + USER_STACK_SIZE - 16; // Align to 16 bytes
    program->heap_start = USER_HEAP_BASE;
    program->heap_end = USER_HEAP_BASE + USER_HEAP_SIZE;
    
    KDEBUG("Address space: code=0x%llx, stack=0x%llx, heap=0x%llx-0x%llx",
           program->entry_point, program->stack_pointer, 
           program->heap_start, program->heap_end);
    
    return SUCCESS;
}

// Load program sections into memory
static error_t user_loader_load_sections(const void* binary, flat_binary_header_t* header, user_program_t* program) {
    if (!binary || !header || !program) {
        return E_INVAL;
    }
    
    const u8* binary_data = (const u8*)binary;
    u32 offset = sizeof(flat_binary_header_t);
    
    // Load code section
    if (header->code_size > 0) {
        void* code_addr = (void*)program->entry_point;
        
        // Map code pages
        for (u32 i = 0; i < program->code_pages; i++) {
            void* page_addr = (void*)(program->entry_point + i * 4096);
            void* phys_page = memory_alloc_page();
            if (!phys_page) {
                KERROR("Failed to allocate code page %u", i);
                return E_NOMEM;
            }
            
            // Map page
            memory_map_page(page_addr, phys_page, MEMORY_USER | MEMORY_READ | MEMORY_EXEC);
        }
        
        // Copy code
        memcpy(code_addr, binary_data + offset, header->code_size);
        offset += header->code_size;
        
        KDEBUG("Loaded code section: %u bytes at 0x%llx", 
               header->code_size, program->entry_point);
    }
    
    // Load data section
    if (header->data_size > 0) {
        void* data_addr = (void*)(program->entry_point + header->code_size);
        
        // Map data pages
        for (u32 i = 0; i < program->data_pages; i++) {
            void* page_addr = (void*)(program->entry_point + header->code_size + i * 4096);
            void* phys_page = memory_alloc_page();
            if (!phys_page) {
                KERROR("Failed to allocate data page %u", i);
                return E_NOMEM;
            }
            
            // Map page
            memory_map_page(page_addr, phys_page, MEMORY_USER | MEMORY_READ | MEMORY_WRITE);
        }
        
        // Copy data
        memcpy(data_addr, binary_data + offset, header->data_size);
        offset += header->data_size;
        
        KDEBUG("Loaded data section: %u bytes at 0x%llx", 
               header->data_size, program->entry_point + header->code_size);
    }
    
    // Setup BSS section (zero-initialized data)
    if (header->bss_size > 0) {
        void* bss_addr = (void*)(program->entry_point + header->code_size + header->data_size);
        memset(bss_addr, 0, header->bss_size);
        
        KDEBUG("Setup BSS section: %u bytes at 0x%llx", 
               header->bss_size, program->entry_point + header->code_size + header->data_size);
    }
    
    return SUCCESS;
}

// Setup user stack
static error_t user_loader_setup_stack(user_program_t* program) {
    if (!program) {
        return E_INVAL;
    }
    
    // Map stack pages
    for (u32 i = 0; i < program->stack_pages; i++) {
        void* page_addr = (void*)(USER_STACK_BASE + i * 4096);
        void* phys_page = memory_alloc_page();
        if (!phys_page) {
            KERROR("Failed to allocate stack page %u", i);
            return E_NOMEM;
        }
        
        // Map page (grow downward)
        memory_map_page(page_addr, phys_page, MEMORY_USER | MEMORY_READ | MEMORY_WRITE);
    }
    
    KDEBUG("Setup user stack: %u pages at 0x%llx", program->stack_pages, USER_STACK_BASE);
    
    return SUCCESS;
}

// Setup user heap
static error_t user_loader_setup_heap(user_program_t* program) {
    if (!program) {
        return E_INVAL;
    }
    
    // Map initial heap pages
    u32 heap_pages = USER_HEAP_SIZE / 4096;
    for (u32 i = 0; i < heap_pages; i++) {
        void* page_addr = (void*)(USER_HEAP_BASE + i * 4096);
        void* phys_page = memory_alloc_page();
        if (!phys_page) {
            KERROR("Failed to allocate heap page %u", i);
            return E_NOMEM;
        }
        
        // Map page
        memory_map_page(page_addr, phys_page, MEMORY_USER | MEMORY_READ | MEMORY_WRITE);
    }
    
    KDEBUG("Setup user heap: %u pages at 0x%llx", heap_pages, USER_HEAP_BASE);
    
    return SUCCESS;
}

// Setup page tables for user program
static error_t user_loader_setup_page_tables(user_program_t* program) {
    if (!program) {
        return E_INVAL;
    }
    
    // This would involve setting up a separate page table for the user process
    // For now, we'll use the kernel page table with user mappings
    
    KDEBUG("Setup page tables for user program");
    
    return SUCCESS;
}

// Validate user pointer (for syscalls)
static error_t user_loader_validate_user_pointer(const void* ptr, size_t size) {
    if (!ptr) {
        return E_INVAL;
    }
    
    u64 addr = (u64)ptr;
    
    // Check if pointer is in user space
    if (addr < 0x400000 || addr >= 0x800000000000) {
        return E_FAULT;
    }
    
    // Check if range is valid
    if (addr + size > 0x800000000000) {
        return E_FAULT;
    }
    
    // Check if pages are mapped and accessible
    // This would involve checking page table entries
    
    return SUCCESS;
}

// Get current user program
user_program_t* user_loader_get_current_program(void) {
    if (!user_loader_initialized || !current_user_program.loaded) {
        return NULL;
    }
    
    return &current_user_program;
}

// Check if user loader is initialized
bool user_loader_is_initialized(void) {
    return user_loader_initialized;
}

// Dump user program information
void user_loader_dump_info(void) {
    KINFO("=== User Program Loader Information ===");
    KINFO("Initialized: %s", user_loader_initialized ? "Yes" : "No");
    
    if (current_user_program.loaded) {
        KINFO("Current Program:");
        KINFO("  Entry Point: 0x%llx", current_user_program.entry_point);
        KINFO("  Stack Pointer: 0x%llx", current_user_program.stack_pointer);
        KINFO("  Heap: 0x%llx - 0x%llx", current_user_program.heap_start, current_user_program.heap_end);
        KINFO("  Pages: Code=%u, Data=%u, Stack=%u", 
              current_user_program.code_pages, current_user_program.data_pages, current_user_program.stack_pages);
    } else {
        KINFO("No program currently loaded");
    }
} 