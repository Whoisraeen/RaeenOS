/*
 * RaeenOS Basic Global Descriptor Table (GDT)
 * Essential for protected mode operation
 */

#include <stdint.h>

// GDT entry structure
struct gdt_entry {
    uint16_t limit_low;      // Lower 16 bits of the limit
    uint16_t base_low;       // Lower 16 bits of the base
    uint8_t  base_middle;    // Next 8 bits of the base
    uint8_t  access;         // Access flags
    uint8_t  granularity;    // Granularity flags
    uint8_t  base_high;      // Higher 8 bits of the base
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;          // Upper 16 bits of all selector limits
    uint32_t base;           // Address of the first gdt_entry
} __attribute__((packed));

// GDT entries
static struct gdt_entry gdt_entries[5];
static struct gdt_ptr gdt_ptr;

// Assembly function to load GDT
extern void gdt_flush(uint32_t);

// Set a GDT entry
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

// Initialize GDT
void gdt_init(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

    gdt_flush((uint32_t)&gdt_ptr);
}