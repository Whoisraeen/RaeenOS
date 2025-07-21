#include "apps/text_editor.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "ui/components/window_manager.h"
#include "filesystem/vfs.h"
#include "kernel.h"
#include <string.h>
#include <stdlib.h>

// Text editor constants
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256
#define EDITOR_WIDTH 60
#define EDITOR_HEIGHT 20
#define STATUS_HEIGHT 2

// Text editor state
static text_editor_state_t editor_state = {0};
static bool editor_initialized = false;
static char* text_lines[MAX_LINES];
static int line_count = 0;
static int cursor_x = 0;
static int cursor_y = 0;
static int scroll_y = 0;
static int window_id = -1;
static bool modified = false;
static char filename[256] = "";

// Initialize text editor
error_t text_editor_init(void) {
    if (editor_initialized) {
        return SUCCESS;
    }
    
    KINFO("Initializing text editor...");
    
    // Initialize editor state
    memset(&editor_state, 0, sizeof(editor_state));
    editor_state.running = true;
    editor_state.insert_mode = true;
    
    // Initialize text lines
    for (int i = 0; i < MAX_LINES; i++) {
        text_lines[i] = malloc(MAX_LINE_LENGTH);
        if (text_lines[i]) {
            text_lines[i][0] = '\0';
        }
    }
    
    // Add initial empty line
    line_count = 1;
    
    // Create editor window
    window_id = window_manager_create_window("Text Editor", 5, 2, EDITOR_WIDTH + 2, EDITOR_HEIGHT + STATUS_HEIGHT + 2);
    if (window_id == -1) {
        KERROR("Failed to create editor window");
        return E_FAIL;
    }
    
    // Add status bar
    window_manager_add_widget(window_id, WIDGET_TYPE_LABEL, 1, EDITOR_HEIGHT + 1, EDITOR_WIDTH, 1, "Ready");
    
    editor_initialized = true;
    KINFO("Text editor initialized successfully");
    
    return SUCCESS;
}

// Main text editor loop
void text_editor_main_loop(void) {
    if (!editor_initialized) {
        KERROR("Text editor not initialized");
        return;
    }
    
    KINFO("Starting text editor main loop");
    
    // Show editor window
    window_manager_show_window(window_id);
    
    while (editor_state.running) {
        // Handle input
        text_editor_handle_input();
        
        // Redraw editor
        text_editor_redraw();
        
        // Update status
        text_editor_update_status();
        
        // Yield to other processes
        // TODO: Implement proper process yielding
    }
    
    // Clean up
    text_editor_cleanup();
}

// Handle keyboard input
void text_editor_handle_input(void) {
    int c = keyboard_read_char();
    if (c == -1) {
        return;
    }
    
    switch (c) {
        case 27: // Escape
            text_editor_handle_escape();
            break;
        case 13: // Enter
            text_editor_handle_enter();
            break;
        case 8:  // Backspace
            text_editor_handle_backspace();
            break;
        case 9:  // Tab
            text_editor_handle_tab();
            break;
        case 17: // Ctrl+Q
            text_editor_quit();
            break;
        case 19: // Ctrl+S
            text_editor_save();
            break;
        case 15: // Ctrl+O
            text_editor_open();
            break;
        default:
            if (c >= 32 && c <= 126) {
                text_editor_handle_character(c);
            }
            break;
    }
}

// Handle escape key
void text_editor_handle_escape(void) {
    // Toggle insert mode
    editor_state.insert_mode = !editor_state.insert_mode;
    text_editor_update_status();
}

// Handle enter key
void text_editor_handle_enter(void) {
    if (line_count >= MAX_LINES) {
        return;
    }
    
    // Insert new line
    for (int i = line_count; i > cursor_y + 1; i--) {
        strcpy(text_lines[i], text_lines[i - 1]);
    }
    
    // Split current line
    if (cursor_y < line_count) {
        char* current_line = text_lines[cursor_y];
        int line_len = strlen(current_line);
        
        if (cursor_x < line_len) {
            // Create new line with remaining text
            strcpy(text_lines[cursor_y + 1], current_line + cursor_x);
            current_line[cursor_x] = '\0';
        } else {
            // Empty new line
            text_lines[cursor_y + 1][0] = '\0';
        }
    }
    
    line_count++;
    cursor_y++;
    cursor_x = 0;
    modified = true;
}

// Handle backspace key
void text_editor_handle_backspace(void) {
    if (cursor_x > 0) {
        // Delete character at cursor
        char* line = text_lines[cursor_y];
        int line_len = strlen(line);
        
        for (int i = cursor_x - 1; i < line_len; i++) {
            line[i] = line[i + 1];
        }
        
        cursor_x--;
        modified = true;
    } else if (cursor_y > 0) {
        // Merge with previous line
        char* current_line = text_lines[cursor_y];
        char* prev_line = text_lines[cursor_y - 1];
        int prev_len = strlen(prev_line);
        
        if (prev_len + strlen(current_line) < MAX_LINE_LENGTH) {
            strcat(prev_line, current_line);
            
            // Remove current line
            for (int i = cursor_y; i < line_count - 1; i++) {
                strcpy(text_lines[i], text_lines[i + 1]);
            }
            
            line_count--;
            cursor_y--;
            cursor_x = prev_len;
            modified = true;
        }
    }
}

// Handle tab key
void text_editor_handle_tab(void) {
    // Insert spaces for tab
    for (int i = 0; i < 4; i++) {
        text_editor_handle_character(' ');
    }
}

// Handle character input
void text_editor_handle_character(char c) {
    if (cursor_y >= line_count) {
        return;
    }
    
    char* line = text_lines[cursor_y];
    int line_len = strlen(line);
    
    if (cursor_x < MAX_LINE_LENGTH - 1) {
        if (editor_state.insert_mode) {
            // Insert mode: shift characters right
            for (int i = line_len; i > cursor_x; i--) {
                line[i] = line[i - 1];
            }
        }
        
        line[cursor_x] = c;
        cursor_x++;
        
        if (cursor_x > line_len) {
            line[cursor_x] = '\0';
        }
        
        modified = true;
    }
}

// Redraw editor content
void text_editor_redraw(void) {
    if (window_id == -1) {
        return;
    }
    
    // Clear editor area
    for (int y = 0; y < EDITOR_HEIGHT; y++) {
        int line_index = scroll_y + y;
        
        if (line_index < line_count) {
            // Display line
            char* line = text_lines[line_index];
            int line_len = strlen(line);
            
            for (int x = 0; x < EDITOR_WIDTH && x < line_len; x++) {
                int screen_x = 6 + x; // Window border offset
                int screen_y = 3 + y; // Window border + title offset
                
                vga_set_cursor(screen_x, screen_y);
                vga_putchar(line[x]);
            }
        }
    }
    
    // Draw cursor
    int cursor_screen_x = 6 + cursor_x;
    int cursor_screen_y = 3 + (cursor_y - scroll_y);
    
    if (cursor_screen_y >= 3 && cursor_screen_y < 3 + EDITOR_HEIGHT) {
        vga_set_cursor(cursor_screen_x, cursor_screen_y);
        vga_putchar('_'); // Cursor indicator
    }
}

// Update status bar
void text_editor_update_status(void) {
    if (window_id == -1) {
        return;
    }
    
    char status[256];
    snprintf(status, sizeof(status), "Line %d, Col %d | %s | %s", 
             cursor_y + 1, cursor_x + 1,
             editor_state.insert_mode ? "INS" : "OVR",
             modified ? "Modified" : "Saved");
    
    // Update status widget
    // TODO: Update status widget text
}

// Save file
void text_editor_save(void) {
    if (strlen(filename) == 0) {
        // TODO: Show save dialog
        strcpy(filename, "untitled.txt");
    }
    
    int fd = vfs_open(filename, FILE_FLAG_WRITE | FILE_FLAG_CREATE | FILE_FLAG_TRUNCATE, 0644);
    if (fd == -1) {
        // TODO: Show error message
        return;
    }
    
    for (int i = 0; i < line_count; i++) {
        vfs_write(fd, text_lines[i], strlen(text_lines[i]));
        vfs_write(fd, "\n", 1);
    }
    
    vfs_close(fd);
    modified = false;
    
    KINFO("Saved file: %s", filename);
}

// Open file
void text_editor_open(void) {
    // TODO: Show open dialog
    strcpy(filename, "test.txt");
    
    int fd = vfs_open(filename, FILE_FLAG_READ, 0);
    if (fd == -1) {
        // TODO: Show error message
        return;
    }
    
    // Clear current content
    for (int i = 0; i < line_count; i++) {
        text_lines[i][0] = '\0';
    }
    line_count = 0;
    
    // Read file content
    char buffer[MAX_LINE_LENGTH];
    int buffer_pos = 0;
    
    while (1) {
        char c;
        ssize_t result = vfs_read(fd, &c, 1);
        if (result <= 0) {
            break;
        }
        
        if (c == '\n') {
            buffer[buffer_pos] = '\0';
            if (line_count < MAX_LINES) {
                strcpy(text_lines[line_count], buffer);
                line_count++;
            }
            buffer_pos = 0;
        } else if (buffer_pos < MAX_LINE_LENGTH - 1) {
            buffer[buffer_pos++] = c;
        }
    }
    
    // Add last line if not empty
    if (buffer_pos > 0 && line_count < MAX_LINES) {
        buffer[buffer_pos] = '\0';
        strcpy(text_lines[line_count], buffer);
        line_count++;
    }
    
    // Add empty line if file is empty
    if (line_count == 0) {
        line_count = 1;
    }
    
    vfs_close(fd);
    cursor_x = 0;
    cursor_y = 0;
    scroll_y = 0;
    modified = false;
    
    KINFO("Opened file: %s (%d lines)", filename, line_count);
}

// Quit editor
void text_editor_quit(void) {
    if (modified) {
        // TODO: Ask user to save changes
        text_editor_save();
    }
    
    editor_state.running = false;
}

// Clean up editor
void text_editor_cleanup(void) {
    if (window_id != -1) {
        window_manager_destroy_window(window_id);
        window_id = -1;
    }
    
    // Free text lines
    for (int i = 0; i < MAX_LINES; i++) {
        if (text_lines[i]) {
            free(text_lines[i]);
            text_lines[i] = NULL;
        }
    }
    
    editor_initialized = false;
    KINFO("Text editor cleanup complete");
}

// Get editor state
text_editor_state_t* text_editor_get_state(void) {
    return &editor_state;
}

// Check if editor is initialized
bool text_editor_is_initialized(void) {
    return editor_initialized;
}

// Get current filename
const char* text_editor_get_filename(void) {
    return filename;
}

// Set filename
void text_editor_set_filename(const char* name) {
    if (name) {
        strncpy(filename, name, sizeof(filename) - 1);
    }
}

// Check if file is modified
bool text_editor_is_modified(void) {
    return modified;
}

// Get line count
int text_editor_get_line_count(void) {
    return line_count;
}

// Get cursor position
void text_editor_get_cursor(int* x, int* y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

// Set cursor position
void text_editor_set_cursor(int x, int y) {
    if (x >= 0 && y >= 0 && y < line_count) {
        cursor_x = x;
        cursor_y = y;
        
        // Adjust scroll position if needed
        if (cursor_y < scroll_y) {
            scroll_y = cursor_y;
        } else if (cursor_y >= scroll_y + EDITOR_HEIGHT) {
            scroll_y = cursor_y - EDITOR_HEIGHT + 1;
        }
    }
} 