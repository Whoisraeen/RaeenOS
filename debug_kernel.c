// Debug kernel to test what's visible during boot
#include <stdint.h>

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;

void debug_print(const char* str, uint16_t color) {
    static int pos = 0;
    while (*str) {
        vga_buffer[pos++] = (*str) | color;
        str++;
        if (pos >= 80*25) pos = 0;
    }
}

void kernel_main(void) {
    // Clear screen first
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = 0x0720;
    }
    
    // Immediate visible output
    debug_print("RAEENOS DEBUG: KERNEL STARTED!", 0x0F00);
    debug_print(" GRUB HANDOFF SUCCESSFUL!", 0x0A00);
    debug_print(" VGA OUTPUT WORKING!", 0x0E00);
    debug_print(" MULTIBOOT OK!", 0x0C00);
    
    // Keep running
    while(1) {
        asm volatile("hlt");
    }
}