#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256
#define MAX_FILENAME 256

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int line_count;
    int cursor_x;
    int cursor_y;
    char filename[MAX_FILENAME];
    bool modified;
} editor_t;

// Initialize editor
void editor_init(editor_t* editor) {
    memset(editor, 0, sizeof(editor_t));
    editor->line_count = 1;
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->modified = false;
}

// Load file into editor
int editor_load_file(editor_t* editor, const char* filename) {
    if (!filename) {
        return -1;
    }
    
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Error: Cannot open file '%s'\n", filename);
        return -1;
    }
    
    strncpy(editor->filename, filename, MAX_FILENAME - 1);
    editor->filename[MAX_FILENAME - 1] = '\0';
    
    char buffer[MAX_LINE_LENGTH];
    int line_index = 0;
    int char_index = 0;
    
    while (line_index < MAX_LINES - 1) {
        char c;
        ssize_t bytes_read = read(fd, &c, 1);
        
        if (bytes_read <= 0) {
            break;
        }
        
        if (c == '\n') {
            buffer[char_index] = '\0';
            strncpy(editor->lines[line_index], buffer, MAX_LINE_LENGTH - 1);
            editor->lines[line_index][MAX_LINE_LENGTH - 1] = '\0';
            line_index++;
            char_index = 0;
        } else if (char_index < MAX_LINE_LENGTH - 1) {
            buffer[char_index++] = c;
        }
    }
    
    // Add last line if not empty
    if (char_index > 0) {
        buffer[char_index] = '\0';
        strncpy(editor->lines[line_index], buffer, MAX_LINE_LENGTH - 1);
        editor->lines[line_index][MAX_LINE_LENGTH - 1] = '\0';
        line_index++;
    }
    
    editor->line_count = line_index;
    if (editor->line_count == 0) {
        editor->line_count = 1;
    }
    
    close(fd);
    editor->modified = false;
    
    printf("Loaded file '%s' (%d lines)\n", filename, editor->line_count);
    return 0;
}

// Save file from editor
int editor_save_file(editor_t* editor) {
    if (!editor->filename[0]) {
        printf("Error: No filename specified\n");
        return -1;
    }
    
    int fd = open(editor->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("Error: Cannot create file '%s'\n", editor->filename);
        return -1;
    }
    
    for (int i = 0; i < editor->line_count; i++) {
        write(fd, editor->lines[i], strlen(editor->lines[i]));
        if (i < editor->line_count - 1) {
            write(fd, "\n", 1);
        }
    }
    
    close(fd);
    editor->modified = false;
    
    printf("Saved file '%s'\n", editor->filename);
    return 0;
}

// Insert character at cursor position
void editor_insert_char(editor_t* editor, char c) {
    if (editor->cursor_y >= editor->line_count) {
        return;
    }
    
    char* line = editor->lines[editor->cursor_y];
    int len = strlen(line);
    
    if (editor->cursor_x > len) {
        editor->cursor_x = len;
    }
    
    if (len >= MAX_LINE_LENGTH - 1) {
        return; // Line too long
    }
    
    // Shift characters to make room
    for (int i = len; i > editor->cursor_x; i--) {
        line[i] = line[i - 1];
    }
    
    line[editor->cursor_x] = c;
    line[len + 1] = '\0';
    
    editor->cursor_x++;
    editor->modified = true;
}

// Delete character at cursor position
void editor_delete_char(editor_t* editor) {
    if (editor->cursor_y >= editor->line_count) {
        return;
    }
    
    char* line = editor->lines[editor->cursor_y];
    int len = strlen(line);
    
    if (editor->cursor_x > len) {
        editor->cursor_x = len;
    }
    
    if (editor->cursor_x == 0) {
        return; // At beginning of line
    }
    
    // Shift characters to remove character
    for (int i = editor->cursor_x - 1; i < len; i++) {
        line[i] = line[i + 1];
    }
    
    editor->cursor_x--;
    editor->modified = true;
}

// Insert new line
void editor_insert_line(editor_t* editor) {
    if (editor->line_count >= MAX_LINES - 1) {
        return; // Too many lines
    }
    
    // Shift lines down
    for (int i = editor->line_count; i > editor->cursor_y + 1; i--) {
        strncpy(editor->lines[i], editor->lines[i - 1], MAX_LINE_LENGTH - 1);
        editor->lines[i][MAX_LINE_LENGTH - 1] = '\0';
    }
    
    // Split current line
    char* line = editor->lines[editor->cursor_y];
    int len = strlen(line);
    
    if (editor->cursor_x > len) {
        editor->cursor_x = len;
    }
    
    // Create new line with remaining text
    strncpy(editor->lines[editor->cursor_y + 1], line + editor->cursor_x, MAX_LINE_LENGTH - 1);
    editor->lines[editor->cursor_y + 1][MAX_LINE_LENGTH - 1] = '\0';
    
    // Truncate current line
    line[editor->cursor_x] = '\0';
    
    editor->line_count++;
    editor->cursor_y++;
    editor->cursor_x = 0;
    editor->modified = true;
}

// Move cursor
void editor_move_cursor(editor_t* editor, int dx, int dy) {
    int new_y = editor->cursor_y + dy;
    int new_x = editor->cursor_x + dx;
    
    if (new_y >= 0 && new_y < editor->line_count) {
        editor->cursor_y = new_y;
        
        int line_len = strlen(editor->lines[editor->cursor_y]);
        if (new_x >= 0 && new_x <= line_len) {
            editor->cursor_x = new_x;
        }
    }
}

// Display editor
void editor_display(editor_t* editor) {
    // Clear screen (simple implementation)
    printf("\033[2J\033[H"); // Clear screen and move cursor to top
    
    // Display status line
    printf("RaeenOS Text Editor - %s %s\n", 
           editor->filename[0] ? editor->filename : "Untitled",
           editor->modified ? "[Modified]" : "");
    printf("Commands: Ctrl+S=Save, Ctrl+Q=Quit, Ctrl+N=New\n");
    printf("Line: %d, Column: %d\n", editor->cursor_y + 1, editor->cursor_x + 1);
    printf("----------------------------------------\n");
    
    // Display text
    for (int i = 0; i < editor->line_count; i++) {
        if (i == editor->cursor_y) {
            // Highlight current line
            printf("> %s", editor->lines[i]);
            if (editor->cursor_x == strlen(editor->lines[i])) {
                printf("_"); // Show cursor position
            }
            printf("\n");
        } else {
            printf("  %s\n", editor->lines[i]);
        }
    }
    
    printf("----------------------------------------\n");
}

// Handle input
int editor_handle_input(editor_t* editor) {
    char c;
    ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
    
    if (bytes_read <= 0) {
        return 0;
    }
    
    switch (c) {
        case 3: // Ctrl+C
            return -1;
            
        case 19: // Ctrl+S
            editor_save_file(editor);
            break;
            
        case 17: // Ctrl+Q
            if (editor->modified) {
                printf("Warning: File has unsaved changes. Save before quitting? (y/n): ");
                char response;
                read(STDIN_FILENO, &response, 1);
                if (response == 'y' || response == 'Y') {
                    editor_save_file(editor);
                }
            }
            return -1;
            
        case 14: // Ctrl+N
            editor_init(editor);
            break;
            
        case 127: // Backspace
            editor_delete_char(editor);
            break;
            
        case 10: // Enter
            editor_insert_line(editor);
            break;
            
        case 27: // Escape sequence
            {
                char seq[2];
                read(STDIN_FILENO, seq, 2);
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': // Up arrow
                            editor_move_cursor(editor, 0, -1);
                            break;
                        case 'B': // Down arrow
                            editor_move_cursor(editor, 0, 1);
                            break;
                        case 'C': // Right arrow
                            editor_move_cursor(editor, 1, 0);
                            break;
                        case 'D': // Left arrow
                            editor_move_cursor(editor, -1, 0);
                            break;
                    }
                }
            }
            break;
            
        default:
            if (c >= 32 && c <= 126) { // Printable characters
                editor_insert_char(editor, c);
            }
            break;
    }
    
    return 0;
}

// Main function
int main(int argc, char** argv) {
    printf("RaeenOS Text Editor v1.0\n");
    printf("A simple text editor for RaeenOS\n\n");
    
    editor_t editor;
    editor_init(&editor);
    
    // Load file if specified
    if (argc > 1) {
        if (editor_load_file(&editor, argv[1]) != 0) {
            printf("Failed to load file '%s'\n", argv[1]);
            return 1;
        }
    }
    
    printf("Starting editor...\n");
    printf("Press any key to continue...\n");
    getchar();
    
    // Main editor loop
    while (1) {
        editor_display(&editor);
        
        if (editor_handle_input(&editor) != 0) {
            break;
        }
    }
    
    printf("Editor closed.\n");
    return 0;
} 