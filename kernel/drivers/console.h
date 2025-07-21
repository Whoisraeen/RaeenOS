#ifndef CONSOLE_H
#define CONSOLE_H

#include "../core/types.h"
#include "../core/error.h"

// Console constants
#define CONSOLE_WIDTH           80
#define CONSOLE_HEIGHT          25
#define CONSOLE_BUFFER_SIZE     (CONSOLE_WIDTH * CONSOLE_HEIGHT)
#define CONSOLE_TAB_SIZE        4
#define CONSOLE_HISTORY_SIZE    100

// VGA text mode constants
#define VGA_TEXT_MEMORY         0xB8000
#define VGA_CRTC_INDEX_PORT     0x3D4
#define VGA_CRTC_DATA_PORT      0x3D5
#define VGA_CURSOR_HIGH         14
#define VGA_CURSOR_LOW          15

// Console colors
typedef enum {
    CONSOLE_COLOR_BLACK         = 0,
    CONSOLE_COLOR_BLUE          = 1,
    CONSOLE_COLOR_GREEN         = 2,
    CONSOLE_COLOR_CYAN          = 3,
    CONSOLE_COLOR_RED           = 4,
    CONSOLE_COLOR_MAGENTA       = 5,
    CONSOLE_COLOR_BROWN         = 6,
    CONSOLE_COLOR_LIGHT_GREY    = 7,
    CONSOLE_COLOR_DARK_GREY     = 8,
    CONSOLE_COLOR_LIGHT_BLUE    = 9,
    CONSOLE_COLOR_LIGHT_GREEN   = 10,
    CONSOLE_COLOR_LIGHT_CYAN    = 11,
    CONSOLE_COLOR_LIGHT_RED     = 12,
    CONSOLE_COLOR_LIGHT_MAGENTA = 13,
    CONSOLE_COLOR_LIGHT_BROWN   = 14,
    CONSOLE_COLOR_WHITE         = 15
} console_color_t;

// Console attributes
typedef struct {
    console_color_t foreground;
    console_color_t background;
    bool blink;
} console_attr_t;

// Console position
typedef struct {
    uint16_t x;
    uint16_t y;
} console_pos_t;

// Console character
typedef struct {
    char character;
    console_attr_t attributes;
} console_char_t;

// Console buffer
typedef struct {
    console_char_t buffer[CONSOLE_BUFFER_SIZE];
    console_pos_t cursor;
    console_attr_t current_attr;
    bool cursor_visible;
    bool echo_enabled;
    bool scroll_enabled;
} console_buffer_t;

// Console input line
typedef struct {
    char line[256];
    uint32_t length;
    uint32_t cursor_pos;
    bool complete;
} console_input_t;

// Console history entry
typedef struct {
    char command[256];
    uint64_t timestamp;
} console_history_entry_t;

// Console statistics
typedef struct {
    uint64_t characters_written;
    uint64_t lines_written;
    uint64_t characters_read;
    uint64_t lines_read;
    uint64_t scroll_operations;
    uint64_t clear_operations;
} console_stats_t;

// Console subsystem structure
typedef struct {
    bool initialized;
    console_buffer_t buffer;
    console_input_t input;
    console_history_entry_t history[CONSOLE_HISTORY_SIZE];
    uint32_t history_count;
    uint32_t history_index;
    console_stats_t stats;
    bool raw_mode;
    bool line_mode;
} console_subsystem_t;

// Function prototypes

// Initialization and shutdown
error_t console_init(void);
void console_shutdown(void);

// Basic output functions
error_t console_putchar(char c);
error_t console_puts(const char* str);
error_t console_printf(const char* format, ...);
error_t console_vprintf(const char* format, va_list args);

// Basic input functions
error_t console_getchar(char* c);
error_t console_gets(char* buffer, size_t size);
error_t console_readline(char* buffer, size_t size);

// Cursor management
error_t console_set_cursor(uint16_t x, uint16_t y);
error_t console_get_cursor(uint16_t* x, uint16_t* y);
error_t console_show_cursor(bool visible);
error_t console_move_cursor(int16_t dx, int16_t dy);

// Screen management
error_t console_clear(void);
error_t console_clear_line(void);
error_t console_scroll_up(uint16_t lines);
error_t console_scroll_down(uint16_t lines);

// Color and attributes
error_t console_set_color(console_color_t foreground, console_color_t background);
error_t console_set_foreground(console_color_t color);
error_t console_set_background(console_color_t color);
error_t console_reset_attributes(void);

// Advanced output functions
error_t console_write_at(uint16_t x, uint16_t y, const char* str);
error_t console_write_char_at(uint16_t x, uint16_t y, char c, console_attr_t attr);
error_t console_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char c, console_attr_t attr);

// Input modes
error_t console_set_echo(bool enabled);
error_t console_set_raw_mode(bool enabled);
error_t console_set_line_mode(bool enabled);

// History management
error_t console_add_history(const char* command);
error_t console_get_history(uint32_t index, char* buffer, size_t size);
uint32_t console_get_history_count(void);
void console_clear_history(void);

// Utility functions
uint8_t console_make_color(console_color_t foreground, console_color_t background);
uint16_t console_make_vga_entry(char c, uint8_t color);
error_t console_get_size(uint16_t* width, uint16_t* height);

// Statistics and debugging
error_t console_get_stats(console_stats_t* stats);
void console_reset_stats(void);
const char* console_get_info(void);
bool console_is_initialized(void);

// Low-level VGA functions
void console_update_cursor(void);
void console_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void console_disable_cursor(void);
uint16_t console_get_cursor_position(void);

// Buffer management
error_t console_save_buffer(console_char_t* backup);
error_t console_restore_buffer(const console_char_t* backup);
error_t console_get_char_at(uint16_t x, uint16_t y, console_char_t* ch);

// Special functions
error_t console_beep(void);
error_t console_flash(void);
error_t console_set_title(const char* title);

#endif // CONSOLE_H