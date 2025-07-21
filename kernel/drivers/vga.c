#include "drivers/vga.h"
#include "kernel.h"
#include "hal/hal.h"
#include <string.h>

// VGA constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)

// VGA colors
#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_GREEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GREY 7
#define VGA_DARK_GREY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN 14
#define VGA_WHITE 15

// VGA memory-mapped I/O addresses
#define VGA_MEMORY 0xB8000
#define VGA_INDEX_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5

// VGA registers
#define VGA_REG_CURSOR_HIGH 0x0E
#define VGA_REG_CURSOR_LOW 0x0F

// Global VGA state
static vga_state_t vga_state = {0};
static bool vga_initialized = false;

// VGA buffer (memory-mapped)
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_MEMORY;

// Color functions
static uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Initialize VGA driver
error_t vga_init(void) {
    if (vga_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing VGA driver...");
    
    // Initialize VGA state
    memset(&vga_state, 0, sizeof(vga_state));
    vga_state.terminal_row = 0;
    vga_state.terminal_column = 0;
    vga_state.terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_state.terminal_buffer = vga_buffer;
    vga_state.terminal_width = VGA_WIDTH;
    vga_state.terminal_height = VGA_HEIGHT;
    
    // Clear the terminal
    vga_clear();
    
    // Set cursor position
    vga_set_cursor(0, 0);
    
    vga_initialized = true;
    KINFO("VGA driver initialized successfully");
    
    return SUCCESS;
}

// Clear the terminal
void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_state.terminal_color);
        }
    }
    vga_state.terminal_row = 0;
    vga_state.terminal_column = 0;
}

// Set terminal color
void vga_set_color(uint8_t color) {
    vga_state.terminal_color = color;
}

// Set cursor position
void vga_set_cursor(size_t x, size_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }
    
    vga_state.terminal_column = x;
    vga_state.terminal_row = y;
    
    // Update hardware cursor
    uint16_t pos = y * VGA_WIDTH + x;
    
    // Write to VGA registers
    hal_outb(VGA_INDEX_REGISTER, VGA_REG_CURSOR_HIGH);
    hal_outb(VGA_DATA_REGISTER, (pos >> 8) & 0xFF);
    hal_outb(VGA_INDEX_REGISTER, VGA_REG_CURSOR_LOW);
    hal_outb(VGA_DATA_REGISTER, pos & 0xFF);
}

// Put character at current position
void vga_putchar(char c) {
    if (c == '\n') {
        vga_state.terminal_column = 0;
        vga_state.terminal_row++;
        if (vga_state.terminal_row >= VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        vga_state.terminal_column = 0;
        return;
    }
    
    if (c == '\t') {
        vga_state.terminal_column = (vga_state.terminal_column + 8) & ~7;
        if (vga_state.terminal_column >= VGA_WIDTH) {
            vga_state.terminal_column = 0;
            vga_state.terminal_row++;
            if (vga_state.terminal_row >= VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }
    
    if (c == '\b') {
        if (vga_state.terminal_column > 0) {
            vga_state.terminal_column--;
        }
        return;
    }
    
    // Put character at current position
    const size_t index = vga_state.terminal_row * VGA_WIDTH + vga_state.terminal_column;
    vga_buffer[index] = vga_entry(c, vga_state.terminal_color);
    
    // Move to next position
    vga_state.terminal_column++;
    if (vga_state.terminal_column >= VGA_WIDTH) {
        vga_state.terminal_column = 0;
        vga_state.terminal_row++;
        if (vga_state.terminal_row >= VGA_HEIGHT) {
            vga_scroll();
        }
    }
    
    // Update cursor position
    vga_set_cursor(vga_state.terminal_column, vga_state.terminal_row);
}

// Write string to terminal
void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

// Write string to terminal (null-terminated)
void vga_writestring(const char* data) {
    vga_write(data, strlen(data));
}

// Scroll terminal up
void vga_scroll(void) {
    // Move all lines up by one
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src_index = y * VGA_WIDTH + x;
            const size_t dst_index = (y - 1) * VGA_WIDTH + x;
            vga_buffer[dst_index] = vga_buffer[src_index];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buffer[index] = vga_entry(' ', vga_state.terminal_color);
    }
    
    // Move cursor to last line
    vga_state.terminal_row = VGA_HEIGHT - 1;
    vga_set_cursor(vga_state.terminal_column, vga_state.terminal_row);
}

// Get terminal dimensions
void vga_get_dimensions(size_t* width, size_t* height) {
    if (width) {
        *width = VGA_WIDTH;
    }
    if (height) {
        *height = VGA_HEIGHT;
    }
}

// Get current cursor position
void vga_get_cursor(size_t* x, size_t* y) {
    if (x) {
        *x = vga_state.terminal_column;
    }
    if (y) {
        *y = vga_state.terminal_row;
    }
}

// Set background color
void vga_set_background(enum vga_color color) {
    uint8_t fg = vga_state.terminal_color & 0x0F;
    vga_state.terminal_color = vga_entry_color(fg, color);
}

// Set foreground color
void vga_set_foreground(enum vga_color color) {
    uint8_t bg = (vga_state.terminal_color >> 4) & 0x0F;
    vga_state.terminal_color = vga_entry_color(color, bg);
}

// Draw a box
void vga_draw_box(size_t x, size_t y, size_t width, size_t height, char border_char) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }
    
    // Draw top border
    for (size_t i = 0; i < width && (x + i) < VGA_WIDTH; i++) {
        const size_t index = y * VGA_WIDTH + (x + i);
        vga_buffer[index] = vga_entry(border_char, vga_state.terminal_color);
    }
    
    // Draw bottom border
    if (y + height - 1 < VGA_HEIGHT) {
        for (size_t i = 0; i < width && (x + i) < VGA_WIDTH; i++) {
            const size_t index = (y + height - 1) * VGA_WIDTH + (x + i);
            vga_buffer[index] = vga_entry(border_char, vga_state.terminal_color);
        }
    }
    
    // Draw left border
    for (size_t i = 1; i < height - 1 && (y + i) < VGA_HEIGHT; i++) {
        const size_t index = (y + i) * VGA_WIDTH + x;
        vga_buffer[index] = vga_entry(border_char, vga_state.terminal_color);
    }
    
    // Draw right border
    if (x + width - 1 < VGA_WIDTH) {
        for (size_t i = 1; i < height - 1 && (y + i) < VGA_HEIGHT; i++) {
            const size_t index = (y + i) * VGA_WIDTH + (x + width - 1);
            vga_buffer[index] = vga_entry(border_char, vga_state.terminal_color);
        }
    }
}

// Fill area with character
void vga_fill_area(size_t x, size_t y, size_t width, size_t height, char fill_char) {
    for (size_t row = y; row < y + height && row < VGA_HEIGHT; row++) {
        for (size_t col = x; col < x + width && col < VGA_WIDTH; col++) {
            const size_t index = row * VGA_WIDTH + col;
            vga_buffer[index] = vga_entry(fill_char, vga_state.terminal_color);
        }
    }
}

// Print formatted string (basic implementation)
void vga_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    vga_writestring(buffer);
}

// Check if VGA is initialized
bool vga_is_initialized(void) {
    return vga_initialized;
}

// Get VGA state
vga_state_t* vga_get_state(void) {
    return &vga_state;
} 
} 