#include "console.h"
#include "../core/kernel.h"
#include "../hal/hal.h"
#include "../drivers/keyboard.h"
#include <string.h>
#include <stdarg.h>

// Global console subsystem
static console_subsystem_t console_subsystem = {0};
static bool console_initialized = false;

// VGA text buffer pointer
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_TEXT_MEMORY;

// Default console attributes
static const console_attr_t default_attr = {
    .foreground = CONSOLE_COLOR_LIGHT_GREY,
    .background = CONSOLE_COLOR_BLACK,
    .blink = false
};

// Initialize console subsystem
error_t console_init(void) {
    if (console_initialized) {
        return E_ALREADY;
    }
    
    KINFO("Initializing console subsystem");
    
    // Initialize console subsystem structure
    memset(&console_subsystem, 0, sizeof(console_subsystem_t));
    console_subsystem.initialized = true;
    console_subsystem.buffer.current_attr = default_attr;
    console_subsystem.buffer.cursor_visible = true;
    console_subsystem.buffer.echo_enabled = true;
    console_subsystem.buffer.scroll_enabled = true;
    console_subsystem.line_mode = true;
    
    // Clear screen
    console_clear();
    
    // Enable cursor
    console_enable_cursor(14, 15);
    
    console_initialized = true;
    
    KINFO("Console subsystem initialized");
    return SUCCESS;
}

// Shutdown console subsystem
void console_shutdown(void) {
    if (!console_initialized) {
        return;
    }
    
    KINFO("Shutting down console subsystem");
    
    // Disable cursor
    console_disable_cursor();
    
    console_initialized = false;
    
    KINFO("Console subsystem shut down");
}

// Make VGA color byte
uint8_t console_make_color(console_color_t foreground, console_color_t background) {
    return foreground | (background << 4);
}

// Make VGA entry
uint16_t console_make_vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

// Update hardware cursor
void console_update_cursor(void) {
    if (!console_initialized) {
        return;
    }
    
    uint16_t pos = console_subsystem.buffer.cursor.y * CONSOLE_WIDTH + 
                   console_subsystem.buffer.cursor.x;
    
    hal_outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_LOW);
    hal_outb(VGA_CRTC_DATA_PORT, (uint8_t)(pos & 0xFF));
    hal_outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_HIGH);
    hal_outb(VGA_CRTC_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

// Enable cursor
void console_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    hal_outb(VGA_CRTC_INDEX_PORT, 0x0A);
    hal_outb(VGA_CRTC_DATA_PORT, (hal_inb(VGA_CRTC_DATA_PORT) & 0xC0) | cursor_start);
    
    hal_outb(VGA_CRTC_INDEX_PORT, 0x0B);
    hal_outb(VGA_CRTC_DATA_PORT, (hal_inb(VGA_CRTC_DATA_PORT) & 0xE0) | cursor_end);
}

// Disable cursor
void console_disable_cursor(void) {
    hal_outb(VGA_CRTC_INDEX_PORT, 0x0A);
    hal_outb(VGA_CRTC_DATA_PORT, 0x20);
}

// Get cursor position
uint16_t console_get_cursor_position(void) {
    uint16_t pos = 0;
    
    hal_outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_LOW);
    pos |= hal_inb(VGA_CRTC_DATA_PORT);
    hal_outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_HIGH);
    pos |= ((uint16_t)hal_inb(VGA_CRTC_DATA_PORT)) << 8;
    
    return pos;
}

// Scroll screen up
error_t console_scroll_up(uint16_t lines) {
    if (!console_initialized || lines == 0) {
        return E_INVAL;
    }
    
    if (lines >= CONSOLE_HEIGHT) {
        return console_clear();
    }
    
    // Move lines up
    for (uint16_t y = 0; y < CONSOLE_HEIGHT - lines; y++) {
        for (uint16_t x = 0; x < CONSOLE_WIDTH; x++) {
            uint16_t src_pos = (y + lines) * CONSOLE_WIDTH + x;
            uint16_t dst_pos = y * CONSOLE_WIDTH + x;
            
            console_subsystem.buffer.buffer[dst_pos] = 
                console_subsystem.buffer.buffer[src_pos];
            
            uint8_t color = console_make_color(
                console_subsystem.buffer.buffer[dst_pos].attributes.foreground,
                console_subsystem.buffer.buffer[dst_pos].attributes.background);
            
            vga_buffer[dst_pos] = console_make_vga_entry(
                console_subsystem.buffer.buffer[dst_pos].character, color);
        }
    }
    
    // Clear bottom lines
    console_attr_t clear_attr = {
        .foreground = CONSOLE_COLOR_LIGHT_GREY,
        .background = CONSOLE_COLOR_BLACK,
        .blink = false
    };
    
    for (uint16_t y = CONSOLE_HEIGHT - lines; y < CONSOLE_HEIGHT; y++) {
        for (uint16_t x = 0; x < CONSOLE_WIDTH; x++) {
            uint16_t pos = y * CONSOLE_WIDTH + x;
            
            console_subsystem.buffer.buffer[pos].character = ' ';
            console_subsystem.buffer.buffer[pos].attributes = clear_attr;
            
            uint8_t color = console_make_color(clear_attr.foreground, clear_attr.background);
            vga_buffer[pos] = console_make_vga_entry(' ', color);
        }
    }
    
    console_subsystem.stats.scroll_operations++;
    return SUCCESS;
}

// Put character at current cursor position
error_t console_putchar(char c) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_pos_t* cursor = &console_subsystem.buffer.cursor;
    
    switch (c) {
        case '\n':
            cursor->x = 0;
            cursor->y++;
            break;
            
        case '\r':
            cursor->x = 0;
            break;
            
        case '\t':
            cursor->x = (cursor->x + CONSOLE_TAB_SIZE) & ~(CONSOLE_TAB_SIZE - 1);
            if (cursor->x >= CONSOLE_WIDTH) {
                cursor->x = 0;
                cursor->y++;
            }
            break;
            
        case '\b':
            if (cursor->x > 0) {
                cursor->x--;
                // Clear character at cursor position
                uint16_t pos = cursor->y * CONSOLE_WIDTH + cursor->x;
                console_subsystem.buffer.buffer[pos].character = ' ';
                console_subsystem.buffer.buffer[pos].attributes = 
                    console_subsystem.buffer.current_attr;
                
                uint8_t color = console_make_color(
                    console_subsystem.buffer.current_attr.foreground,
                    console_subsystem.buffer.current_attr.background);
                vga_buffer[pos] = console_make_vga_entry(' ', color);
            }
            break;
            
        default:
            if (c >= 32 && c <= 126) { // Printable characters
                uint16_t pos = cursor->y * CONSOLE_WIDTH + cursor->x;
                
                console_subsystem.buffer.buffer[pos].character = c;
                console_subsystem.buffer.buffer[pos].attributes = 
                    console_subsystem.buffer.current_attr;
                
                uint8_t color = console_make_color(
                    console_subsystem.buffer.current_attr.foreground,
                    console_subsystem.buffer.current_attr.background);
                vga_buffer[pos] = console_make_vga_entry(c, color);
                
                cursor->x++;
                if (cursor->x >= CONSOLE_WIDTH) {
                    cursor->x = 0;
                    cursor->y++;
                }
            }
            break;
    }
    
    // Handle scrolling
    if (cursor->y >= CONSOLE_HEIGHT) {
        if (console_subsystem.buffer.scroll_enabled) {
            console_scroll_up(cursor->y - CONSOLE_HEIGHT + 1);
            cursor->y = CONSOLE_HEIGHT - 1;
        } else {
            cursor->y = CONSOLE_HEIGHT - 1;
        }
    }
    
    // Update hardware cursor
    console_update_cursor();
    
    console_subsystem.stats.characters_written++;
    return SUCCESS;
}

// Put string
error_t console_puts(const char* str) {
    if (!console_initialized || !str) {
        return E_INVAL;
    }
    
    while (*str) {
        error_t result = console_putchar(*str);
        if (result != SUCCESS) {
            return result;
        }
        str++;
    }
    
    console_subsystem.stats.lines_written++;
    return SUCCESS;
}

// Printf implementation
error_t console_printf(const char* format, ...) {
    if (!console_initialized || !format) {
        return E_INVAL;
    }
    
    va_list args;
    va_start(args, format);
    error_t result = console_vprintf(format, args);
    va_end(args);
    
    return result;
}

// Vprintf implementation
error_t console_vprintf(const char* format, va_list args) {
    if (!console_initialized || !format) {
        return E_INVAL;
    }
    
    char buffer[1024];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (len < 0) {
        return E_INVAL;
    }
    
    return console_puts(buffer);
}

// Get character from keyboard
error_t console_getchar(char* c) {
    if (!console_initialized || !c) {
        return E_INVAL;
    }
    
    key_event_t event;
    while (true) {
        if (keyboard_read_event(&event) == SUCCESS) {
            if (event.type == KEY_EVENT_PRESS && event.ascii != 0) {
                *c = event.ascii;
                
                if (console_subsystem.buffer.echo_enabled) {
                    console_putchar(*c);
                }
                
                console_subsystem.stats.characters_read++;
                return SUCCESS;
            }
        }
        
        // Yield to other processes
        scheduler_yield();
    }
}

// Get string (line mode)
error_t console_gets(char* buffer, size_t size) {
    if (!console_initialized || !buffer || size == 0) {
        return E_INVAL;
    }
    
    size_t pos = 0;
    char c;
    
    while (pos < size - 1) {
        error_t result = console_getchar(&c);
        if (result != SUCCESS) {
            return result;
        }
        
        if (c == '\n' || c == '\r') {
            break;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
            }
        } else {
            buffer[pos++] = c;
        }
    }
    
    buffer[pos] = '\0';
    console_subsystem.stats.lines_read++;
    return SUCCESS;
}

// Read line with editing support
error_t console_readline(char* buffer, size_t size) {
    if (!console_initialized || !buffer || size == 0) {
        return E_INVAL;
    }
    
    console_input_t* input = &console_subsystem.input;
    input->length = 0;
    input->cursor_pos = 0;
    input->complete = false;
    
    while (!input->complete) {
        key_event_t event;
        if (keyboard_read_event(&event) == SUCCESS && event.type == KEY_EVENT_PRESS) {
            switch (event.scancode) {
                case KEY_ENTER:
                    input->complete = true;
                    console_putchar('\n');
                    break;
                    
                case KEY_BACKSPACE:
                    if (input->cursor_pos > 0) {
                        input->cursor_pos--;
                        input->length--;
                        console_putchar('\b');
                    }
                    break;
                    
                default:
                    if (event.ascii != 0 && input->length < size - 1) {
                        input->line[input->length++] = event.ascii;
                        input->cursor_pos++;
                        if (console_subsystem.buffer.echo_enabled) {
                            console_putchar(event.ascii);
                        }
                    }
                    break;
            }
        }
        
        scheduler_yield();
    }
    
    strncpy(buffer, input->line, input->length);
    buffer[input->length] = '\0';
    
    console_subsystem.stats.lines_read++;
    return SUCCESS;
}

// Clear screen
error_t console_clear(void) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_attr_t clear_attr = {
        .foreground = CONSOLE_COLOR_LIGHT_GREY,
        .background = CONSOLE_COLOR_BLACK,
        .blink = false
    };
    
    for (uint16_t i = 0; i < CONSOLE_BUFFER_SIZE; i++) {
        console_subsystem.buffer.buffer[i].character = ' ';
        console_subsystem.buffer.buffer[i].attributes = clear_attr;
        
        uint8_t color = console_make_color(clear_attr.foreground, clear_attr.background);
        vga_buffer[i] = console_make_vga_entry(' ', color);
    }
    
    console_subsystem.buffer.cursor.x = 0;
    console_subsystem.buffer.cursor.y = 0;
    console_update_cursor();
    
    console_subsystem.stats.clear_operations++;
    return SUCCESS;
}

// Set cursor position
error_t console_set_cursor(uint16_t x, uint16_t y) {
    if (!console_initialized || x >= CONSOLE_WIDTH || y >= CONSOLE_HEIGHT) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.cursor.x = x;
    console_subsystem.buffer.cursor.y = y;
    console_update_cursor();
    
    return SUCCESS;
}

// Get cursor position
error_t console_get_cursor(uint16_t* x, uint16_t* y) {
    if (!console_initialized || !x || !y) {
        return E_INVAL;
    }
    
    *x = console_subsystem.buffer.cursor.x;
    *y = console_subsystem.buffer.cursor.y;
    
    return SUCCESS;
}

// Set console colors
error_t console_set_color(console_color_t foreground, console_color_t background) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.current_attr.foreground = foreground;
    console_subsystem.buffer.current_attr.background = background;
    
    return SUCCESS;
}

// Set foreground color
error_t console_set_foreground(console_color_t color) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.current_attr.foreground = color;
    return SUCCESS;
}

// Set background color
error_t console_set_background(console_color_t color) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.current_attr.background = color;
    return SUCCESS;
}

// Reset attributes to default
error_t console_reset_attributes(void) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.current_attr = default_attr;
    return SUCCESS;
}

// Write string at specific position
error_t console_write_at(uint16_t x, uint16_t y, const char* str) {
    if (!console_initialized || !str || x >= CONSOLE_WIDTH || y >= CONSOLE_HEIGHT) {
        return E_INVAL;
    }
    
    uint16_t old_x = console_subsystem.buffer.cursor.x;
    uint16_t old_y = console_subsystem.buffer.cursor.y;
    
    console_set_cursor(x, y);
    error_t result = console_puts(str);
    console_set_cursor(old_x, old_y);
    
    return result;
}

// Get console statistics
error_t console_get_stats(console_stats_t* stats) {
    if (!console_initialized || !stats) {
        return E_INVAL;
    }
    
    *stats = console_subsystem.stats;
    return SUCCESS;
}

// Get console info
const char* console_get_info(void) {
    static char info[256];
    snprintf(info, sizeof(info),
             "Console: %s, Size: %dx%d, Cursor: (%d,%d), Echo: %s",
             console_initialized ? "Initialized" : "Not initialized",
             CONSOLE_WIDTH, CONSOLE_HEIGHT,
             console_subsystem.buffer.cursor.x,
             console_subsystem.buffer.cursor.y,
             console_subsystem.buffer.echo_enabled ? "On" : "Off");
    return info;
}

// Check if console is initialized
bool console_is_initialized(void) {
    return console_initialized;
}

// Set echo mode
error_t console_set_echo(bool enabled) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.buffer.echo_enabled = enabled;
    return SUCCESS;
}

// Set raw mode
error_t console_set_raw_mode(bool enabled) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.raw_mode = enabled;
    return SUCCESS;
}

// Set line mode
error_t console_set_line_mode(bool enabled) {
    if (!console_initialized) {
        return E_INVAL;
    }
    
    console_subsystem.line_mode = enabled;
    return SUCCESS;
}

// Get console size
error_t console_get_size(uint16_t* width, uint16_t* height) {
    if (!width || !height) {
        return E_INVAL;
    }
    
    *width = CONSOLE_WIDTH;
    *height = CONSOLE_HEIGHT;
    return SUCCESS;
}