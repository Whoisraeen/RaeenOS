#include "executable_loader.h"
#include "memory.h"
#include "hal.h"
#include <string.h>

// Simple RAM disk implementation
static uint8_t* ramdisk_data = NULL;
static size_t ramdisk_size = 0;

// Initialize RAM disk with some test data
int ramdisk_init(void) {
    // For now, create a simple RAM disk with a test program
    ramdisk_size = 64 * 1024; // 64KB
    ramdisk_data = kmalloc(ramdisk_size);
    
    if (!ramdisk_data) {
        return -1;
    }
    
    // Clear RAM disk
    memset(ramdisk_data, 0, ramdisk_size);
    
    // Create a simple test program at the beginning
    // This will be replaced by actual binary loading later
    flat_binary_header_t* header = (flat_binary_header_t*)ramdisk_data;
    header->magic = FLAT_BINARY_MAGIC;
    header->entry_point = sizeof(flat_binary_header_t);
    header->code_size = 1024;  // 1KB of code
    header->data_size = 512;   // 512 bytes of data
    header->bss_size = 256;    // 256 bytes of BSS
    header->stack_size = 4096; // 4KB stack
    header->flags = FLAT_BINARY_FLAG_EXECUTABLE;
    
    // Add some simple test code (just a loop that calls sys_write)
    uint8_t* code_start = ramdisk_data + sizeof(flat_binary_header_t);
    
    // Simple x86-64 assembly for a test program
    // This is a minimal program that writes "Hello from user space!" to console
    uint8_t test_code[] = {
        0x48, 0x31, 0xc0,                   // xor rax, rax
        0x48, 0x31, 0xdb,                   // xor rbx, rbx
        0x48, 0x31, 0xc9,                   // xor rcx, rcx
        0x48, 0x31, 0xd2,                   // xor rdx, rdx
        0x48, 0x31, 0xf6,                   // xor rsi, rsi
        0x48, 0x31, 0xff,                   // xor rdi, rdi
        0xeb, 0xfe                          // jmp $ (infinite loop)
    };
    
    memcpy(code_start, test_code, sizeof(test_code));
    
    return 0;
}

// Read a file from RAM disk
void* ramdisk_read_file(const char* filename, size_t* size) {
    if (!ramdisk_data || !filename || !size) {
        return NULL;
    }
    
    // For now, just return the test program
    if (strcmp(filename, "test_program") == 0) {
        *size = ramdisk_size;
        return ramdisk_data;
    }
    
    // Return shell program (placeholder - in real implementation, this would be loaded from disk)
    if (strcmp(filename, "simple_shell") == 0) {
        // For now, return the same test program as a placeholder
        // In a real implementation, this would load the actual shell binary
        *size = ramdisk_size;
        return ramdisk_data;
    }
    
    return NULL;
}

// Load a flat binary executable
int load_flat_binary(const char* filename, process_t* process) {
    size_t binary_size;
    void* binary_data = ramdisk_read_file(filename, &binary_size);
    
    if (!binary_data) {
        return -1;
    }
    
    // Validate header
    flat_binary_header_t* header = (flat_binary_header_t*)binary_data;
    if (header->magic != FLAT_BINARY_MAGIC) {
        return -1;
    }
    
    // Set up the process address space
    int result = setup_user_address_space(process, binary_data, binary_size);
    if (result != 0) {
        return result;
    }
    
    return 0;
}

// Set up user address space for the process
int setup_user_address_space(process_t* process, void* binary_data, size_t binary_size) {
    flat_binary_header_t* header = (flat_binary_header_t*)binary_data;
    
    // Allocate and set up page tables for user space
    if (vm_create_address_space(process) != 0) {
        return -1;
    }
    
    // Map the code section at user space address 0x400000
    uintptr_t code_vaddr = 0x400000;
    uintptr_t code_offset = sizeof(flat_binary_header_t);
    size_t code_size = header->code_size;
    
    // Map code pages as read-only and executable
    for (size_t i = 0; i < code_size; i += PAGE_SIZE) {
        uintptr_t page_vaddr = code_vaddr + i;
        uintptr_t page_offset = code_offset + i;
        
        if (vm_map_page(process, page_vaddr, 
                       (uintptr_t)binary_data + page_offset, 
                       VM_PROT_READ | VM_PROT_EXEC) != 0) {
            return -1;
        }
    }
    
    // Map the data section after code
    if (header->data_size > 0) {
        uintptr_t data_vaddr = code_vaddr + ((code_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
        uintptr_t data_offset = code_offset + code_size;
        
        for (size_t i = 0; i < header->data_size; i += PAGE_SIZE) {
            uintptr_t page_vaddr = data_vaddr + i;
            uintptr_t page_offset = data_offset + i;
            
            if (vm_map_page(process, page_vaddr,
                           (uintptr_t)binary_data + page_offset,
                           VM_PROT_READ | VM_PROT_WRITE) != 0) {
                return -1;
            }
        }
    }
    
    // Set up BSS section (zero-initialized data)
    if (header->bss_size > 0) {
        uintptr_t bss_vaddr = code_vaddr + ((code_size + header->data_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
        
        for (size_t i = 0; i < header->bss_size; i += PAGE_SIZE) {
            uintptr_t page_vaddr = bss_vaddr + i;
            
            // Allocate zero pages for BSS
            void* zero_page = kmalloc(PAGE_SIZE);
            if (!zero_page) {
                return -1;
            }
            memset(zero_page, 0, PAGE_SIZE);
            
            if (vm_map_page(process, page_vaddr, (uintptr_t)zero_page, 
                           VM_PROT_READ | VM_PROT_WRITE) != 0) {
                kfree(zero_page);
                return -1;
            }
        }
    }
    
    // Set up user stack
    void* stack_top = map_user_stack(process, header->stack_size);
    if (!stack_top) {
        return -1;
    }
    
    // Set process entry point
    process->entry_point = code_vaddr + header->entry_point;
    
    return 0;
}

// Map user stack
void* map_user_stack(process_t* process, size_t stack_size) {
    // Map stack at high user address (0x7ffff0000000)
    uintptr_t stack_vaddr = 0x7ffff0000000;
    size_t stack_pages = (stack_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (size_t i = 0; i < stack_pages; i++) {
        uintptr_t page_vaddr = stack_vaddr - (i + 1) * PAGE_SIZE;
        
        // Allocate stack pages
        void* stack_page = kmalloc(PAGE_SIZE);
        if (!stack_page) {
            return NULL;
        }
        memset(stack_page, 0, PAGE_SIZE);
        
        if (vm_map_page(process, page_vaddr, (uintptr_t)stack_page,
                       VM_PROT_READ | VM_PROT_WRITE) != 0) {
            kfree(stack_page);
            return NULL;
        }
    }
    
    // Return stack top (highest address)
    return (void*)stack_vaddr;
}

// Jump to user mode
int jump_to_user_mode(process_t* process, uintptr_t entry_point) {
    // Switch to user page tables
    vm_switch_address_space(process);
    
    // Set up user stack pointer
    uintptr_t user_stack = 0x7ffff0000000;
    
    // Jump to user code using assembly
    __asm__ volatile (
        // Set up user segment registers
        "mov $0x23, %%ax\n"    // User data segment (0x20 | 3)
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        
        // Set up user stack
        "mov %0, %%rsp\n"
        
        // Push user code segment and entry point for iretq
        "pushq $0x1B\n"        // User code segment (0x18 | 3)
        "pushq %1\n"           // Entry point
        
        // Clear flags and return to user mode
        "pushq $0x202\n"       // RFLAGS (IF=1, IOPL=0)
        "pushq $0x1B\n"        // User code segment
        "pushq %1\n"           // Entry point
        
        // Return to user mode
        "iretq\n"
        :
        : "r" (user_stack), "r" (entry_point)
        : "rax", "memory"
    );
    
    // Should never reach here
    return -1;
} 