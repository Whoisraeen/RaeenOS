#ifndef VGA_H
#define VGA_H

#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// VGA functions
error_t vga_init(void);
void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_set_cursor(uint8_t x, uint8_t y);
void vga_putchar_at(char c, uint8_t x, uint8_t y);
void vga_putchar(char c);
void vga_scroll(void);
void vga_write(const char* str);
void vga_write_len(const char* str, size_t len);
char vga_getchar_at(uint8_t x, uint8_t y);
void vga_set_char_at(uint8_t x, uint8_t y, char c, uint8_t fg, uint8_t bg);
void vga_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, char c, uint8_t fg, uint8_t bg);
void vga_draw_border(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t fg, uint8_t bg);
void vga_draw_window(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char* title);
void vga_get_cursor(uint8_t* x, uint8_t* y);
void vga_hide_cursor(void);
void vga_show_cursor(void);
error_t vga_set_mode(uint8_t mode);
void vga_get_dimensions(uint8_t* width, uint8_t* height);

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

// VGA dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#endif // VGA_H 