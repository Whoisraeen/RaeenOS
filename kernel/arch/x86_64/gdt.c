#include "arch/x86_64/gdt.h"
#include "arch/x86_64/tss.h"
#include "kernel.h"
#include "memory/memory.h"
#include <string.h>

gdt_descriptor_t gdt[GDT_ENTRIES];
gdt_pointer_t gdt_ptr;

void gdt_set_descriptor(u32 i, u64 base, u64 limit, u8 access, u8 flags) {
    // Note: The TSS descriptor is a system descriptor and is set up differently.
    if (i >= GDT_ENTRIES) {
        KERROR("GDT index out of bounds: %u", i);
        return;
    }
    
    gdt_descriptor_t* descriptor = &gdt[i];
    descriptor->limit_low = (u16)(limit & 0xFFFF);
    descriptor->base_low = (u16)(base & 0xFFFF);
    descriptor->base_mid = (u8)((base >> 16) & 0xFF);
    descriptor->access = access;
    descriptor->flags = flags | ((limit >> 16) & 0xF);
    descriptor->base_high = (u8)((base >> 24) & 0xFF);
    descriptor->base_highest = (u32)((base >> 32) & 0xFFFFFFFF);
    descriptor->reserved = 0;
}

void gdt_set_tss_descriptor(u32 i, u64 base, u64 limit) {
    gdt_descriptor_t* descriptor = &gdt[i];
    // The TSS descriptor is 16 bytes, unlike a normal GDT entry.
    descriptor->limit_low = (u16)(limit & 0xFFFF);
    descriptor->base_low = (u16)(base & 0xFFFF);
    descriptor->base_mid = (u8)((base >> 16) & 0xFF);
    descriptor->access = GDT_PRESENT | GDT_EXECUTABLE | GDT_TSS; // TSS Access Byte
    descriptor->flags = (limit >> 16) & 0xF;
    descriptor->base_high = (u8)((base >> 24) & 0xFF);
    descriptor->base_highest = (u32)(base >> 32);
}

void gdt_init(void) {
    KINFO("Initializing GDT...");
    
    // Null descriptor
    gdt_set_descriptor(0, 0, 0, 0, 0);
    
    // Kernel code segment (0x08)
    gdt_set_descriptor(1, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_PRIVILEGE_KERNEL | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READABLE, GDT_FLAGS_LONG_MODE);
    
    // Kernel data segment (0x10)
    gdt_set_descriptor(2, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_PRIVILEGE_KERNEL | GDT_CODE_DATA | GDT_READABLE | GDT_WRITABLE, GDT_FLAGS_LONG_MODE);

    // User code segment (0x18 | 3 = 0x1B)
    gdt_set_descriptor(3, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_PRIVILEGE_USER | GDT_CODE_DATA | GDT_EXECUTABLE | GDT_READABLE, GDT_FLAGS_LONG_MODE);
    
    // User data segment (0x20 | 3 = 0x23)
    gdt_set_descriptor(4, 0, 0xFFFFFFFF, GDT_PRESENT | GDT_PRIVILEGE_USER | GDT_CODE_DATA | GDT_READABLE | GDT_WRITABLE, GDT_FLAGS_LONG_MODE);

    // Initialize the comprehensive TSS system
    error_t tss_result = tss_init();
    if (tss_result != SUCCESS) {
        KERROR("Failed to initialize TSS: %s", tss_get_error_string(tss_result));
        // Continue anyway, but log the error
    }
    
    // Set up TSS descriptor in GDT
    gdt_set_tss_descriptor(5, tss_get_base_address(), tss_get_size());
    tss_set_gdt_index(5);
    
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (u64)&gdt;

    // Load the GDT
    x64_load_gdt(&gdt_ptr);

    // Load the Task Register (TR) with the TSS segment selector.
    // The selector is 0x28 (index 5, RPL 0).
    __asm__ volatile("ltr %0" :: "r" ((u16)0x28));
    
    // Load the TSS
    tss_load();
    
    KINFO("GDT initialized. GDT base: 0x%llx", gdt_ptr.base);
    KINFO("TSS loaded at base: 0x%llx", tss_get_base_address());
}