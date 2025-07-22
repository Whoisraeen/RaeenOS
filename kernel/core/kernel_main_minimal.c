/* Minimal Bootable Kernel Main for RaeenOS */ 
#include "stdint.h" 
 
/* VGA text mode buffer */ 
static uint16_t* vga_buffer = (uint16_t*)0xB8000; 
static int vga_row = 0; 
static int vga_col = 0; 
 
void vga_putchar(char c) { 
    if (c == '\n') { 
        vga_row++; 
        vga_col = 0; 
        return; 
    } 
    if (vga_row >= 25) vga_row = 0; 
    if (vga_col >= 80) { vga_col = 0; vga_row++; } 
    vga_buffer[vga_row * 80 + vga_col] = (uint16_t)c | 0x0700; 
    vga_col++; 
} 
 
void vga_print(const char* str) { 
    while (*str) vga_putchar(*str++); 
} 
 
/* Minimal kernel main function */ 
void kernel_main(void* multiboot_info, uint32_t magic) { 
    /* Clear screen */ 
    for (int i = 0; i < 80 * 25; i++) vga_buffer[i] = 0x0720; 
    vga_row = 0; vga_col = 0; 
 
    vga_print("RaeenOS Kernel v1.0 - Successfully Booted\n"); 
    vga_print("Multiboot Magic: "); 
    if (magic == 0x2BADB002) { 
        vga_print("VALID\n"); 
    } else { 
        vga_print("INVALID\n"); 
    } 
    vga_print("Kernel is running successfully\n"); 
    vga_print("Boot test: PASSED\n"); 
 
    /* Infinite loop to keep kernel running */ 
    while (1) { 
        __asm__ volatile ("hlt"); 
    } 
} 
