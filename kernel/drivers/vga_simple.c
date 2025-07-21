/*
 * RaeenOS Simple VGA Text Mode Driver
 * Provides early boot text output for kernel initialization
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// VGA colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA state
static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint16_t *vga_buffer;

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

static void vga_scroll(void) {
    // Move all lines up by one
    for (size_t row = 0; row < VGA_HEIGHT - 1; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            vga_buffer[row * VGA_WIDTH + col] = vga_buffer[(row + 1) * VGA_WIDTH + col];
        }
    }
    
    // Clear the last line
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', vga_color);
    }
    
    vga_row = VGA_HEIGHT - 1;
    vga_column = 0;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_buffer = (uint16_t*) VGA_MEMORY;
    
    // Clear screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
}

void vga_setcolor(uint8_t color) {
    vga_color = color;
}

void vga_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        vga_column = 0;
        return;
    }
    
    if (c == '\t') {
        vga_column = (vga_column + 8) & ~7;
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }
    
    vga_putentryat(c, vga_color, vga_column, vga_row);
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

void vga_print(const char *data) {
    if (!data) return;
    
    while (*data) {
        vga_putchar(*data);
        data++;
    }
}

// Simple implementation of strlen
static size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// Simple implementation of itoa
static char *itoa(int value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    if (value < 0 && base == 10) {
        value = -value;
        *ptr++ = '-';
    }
    
    while (value) {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    }
    
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

void vga_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[32];
    const char *ptr = format;
    
    while (*ptr) {
        if (*ptr == '%' && *(ptr + 1)) {
            ptr++;
            switch (*ptr) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    itoa(val, buffer, 10);
                    vga_print(buffer);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    itoa(val, buffer, 10);
                    vga_print(buffer);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    itoa(val, buffer, 16);
                    vga_print(buffer);
                    break;
                }
                case 'X': {
                    unsigned int val = va_arg(args, unsigned int);
                    itoa(val, buffer, 16);
                    // Convert to uppercase
                    char *p = buffer;
                    while (*p) {
                        if (*p >= 'a' && *p <= 'f') {
                            *p = *p - 'a' + 'A';
                        }
                        p++;
                    }
                    vga_print(buffer);
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);
                    vga_print(str ? str : "(null)");
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);
                    vga_putchar(c);
                    break;
                }
                case '%': {
                    vga_putchar('%');
                    break;
                }
                default: {
                    vga_putchar('%');
                    vga_putchar(*ptr);
                    break;
                }
            }
        } else {
            vga_putchar(*ptr);
        }
        ptr++;
    }
    
    va_end(args);
}