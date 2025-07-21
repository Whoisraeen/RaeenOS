#ifndef TERMINAL_EMULATOR_H
#define TERMINAL_EMULATOR_H

#include "../../gui/rendering/glass_compositor.h"
#include "../../gui/window/glass_window_manager.h"
#include <stdint.h>
#include <stdbool.h>

// Terminal Emulator - Advanced terminal for RaeenOS
// Provides modern terminal features, customization, and development tools

// Terminal types
typedef enum {
    TERMINAL_TYPE_XTERM = 0,         // xterm compatible
    TERMINAL_TYPE_VT100,             // VT100 compatible
    TERMINAL_TYPE_VT220,             // VT220 compatible
    TERMINAL_TYPE_ANSI,              // ANSI compatible
    TERMINAL_TYPE_MODERN,            // Modern terminal
    TERMINAL_TYPE_POWER_SHELL,       // PowerShell terminal
    TERMINAL_TYPE_BASH,              // Bash terminal
    TERMINAL_TYPE_ZSH,               // Zsh terminal
    TERMINAL_TYPE_FISH,              // Fish terminal
} terminal_type_t;

// Color schemes
typedef enum {
    COLOR_SCHEME_DEFAULT = 0,        // Default colors
    COLOR_SCHEME_DARK,               // Dark theme
    COLOR_SCHEME_LIGHT,              // Light theme
    COLOR_SCHEME_SOLARIZED_DARK,     // Solarized dark
    COLOR_SCHEME_SOLARIZED_LIGHT,    // Solarized light
    COLOR_SCHEME_DRACULA,            // Dracula theme
    COLOR_SCHEME_GRUVBOX,            // Gruvbox theme
    COLOR_SCHEME_MONOKAI,            // Monokai theme
    COLOR_SCHEME_NORD,               // Nord theme
    COLOR_SCHEME_TOKYO_NIGHT,        // Tokyo Night theme
    COLOR_SCHEME_CUSTOM,             // Custom colors
} color_scheme_t;

// Font types
typedef enum {
    FONT_TYPE_MONOSPACE = 0,         // Monospace font
    FONT_TYPE_PROPORTIONAL,          // Proportional font
    FONT_TYPE_PROGRAMMING,           // Programming font
    FONT_TYPE_NERD,                  // Nerd font
    FONT_TYPE_CUSTOM,                // Custom font
} font_type_t;

// Terminal modes
typedef enum {
    TERMINAL_MODE_NORMAL = 0,        // Normal mode
    TERMINAL_MODE_INSERT,            // Insert mode
    TERMINAL_MODE_VISUAL,            // Visual mode
    TERMINAL_MODE_COMMAND,           // Command mode
    TERMINAL_MODE_SEARCH,            // Search mode
    TERMINAL_MODE_SELECTION,         // Selection mode
} terminal_mode_t;

// Terminal colors
typedef struct {
    uint32_t background;             // Background color
    uint32_t foreground;             // Foreground color
    uint32_t cursor;                 // Cursor color
    uint32_t selection;              // Selection color
    uint32_t palette[16];            // Color palette
    uint32_t bold_foreground;        // Bold foreground color
    uint32_t dim_foreground;         // Dim foreground color
    uint32_t italic_foreground;      // Italic foreground color
    uint32_t underline_foreground;   // Underline foreground color
} terminal_colors_t;

// Terminal font
typedef struct {
    font_type_t type;                // Font type
    char name[64];                   // Font name
    uint32_t size;                   // Font size
    bool bold;                       // Bold font
    bool italic;                     // Italic font
    bool antialiasing;               // Antialiasing
    bool ligatures;                  // Font ligatures
    float line_height;               // Line height multiplier
} terminal_font_t;

// Terminal character
typedef struct {
    char character;                  // Character
    uint32_t foreground_color;       // Foreground color
    uint32_t background_color;       // Background color
    bool bold;                       // Bold
    bool italic;                     // Italic
    bool underline;                  // Underline
    bool blink;                      // Blink
    bool reverse;                    // Reverse video
    bool invisible;                  // Invisible
    bool strikethrough;              // Strikethrough
    bool double_underline;           // Double underline
    bool overline;                   // Overline
} terminal_char_t;

// Terminal line
typedef struct {
    terminal_char_t* characters;     // Characters in line
    uint32_t length;                 // Line length
    uint32_t capacity;               // Line capacity
    bool wrapped;                    // Is line wrapped
    bool modified;                   // Is line modified
} terminal_line_t;

// Terminal buffer
typedef struct {
    terminal_line_t* lines;          // Lines in buffer
    uint32_t line_count;             // Number of lines
    uint32_t max_lines;              // Maximum lines
    uint32_t scrollback_size;        // Scrollback size
    uint32_t viewport_start;         // Viewport start line
    uint32_t viewport_end;           // Viewport end line
    bool auto_scroll;                // Auto scroll
} terminal_buffer_t;

// Terminal cursor
typedef struct {
    uint32_t x, y;                   // Cursor position
    uint32_t saved_x, saved_y;       // Saved cursor position
    bool visible;                    // Cursor visibility
    bool blink;                      // Cursor blink
    uint32_t blink_rate;             // Blink rate in ms
    uint64_t last_blink_time;        // Last blink time
    terminal_char_t* character;      // Character under cursor
} terminal_cursor_t;

// Terminal selection
typedef struct {
    uint32_t start_x, start_y;       // Selection start
    uint32_t end_x, end_y;           // Selection end
    bool active;                     // Is selection active
    bool rectangular;                // Rectangular selection
    char* selected_text;             // Selected text
    uint32_t text_length;            // Selected text length
} terminal_selection_t;

// Terminal tab
typedef struct {
    uint32_t id;
    char title[64];                  // Tab title
    char working_directory[256];     // Working directory
    terminal_type_t type;            // Terminal type
    terminal_buffer_t* buffer;       // Terminal buffer
    terminal_cursor_t cursor;        // Terminal cursor
    terminal_selection_t selection;  // Terminal selection
    bool is_active;                  // Is active tab
    bool is_closing;                 // Is closing
    void* process_data;              // Process data
} terminal_tab_t;

// Terminal configuration
typedef struct {
    uint32_t window_width, window_height;
    terminal_type_t type;
    color_scheme_t color_scheme;
    terminal_colors_t custom_colors;
    terminal_font_t font;
    uint32_t columns, rows;
    uint32_t scrollback_size;
    bool enable_scrollback;
    bool enable_selection;
    bool enable_copy_paste;
    bool enable_mouse_support;
    bool enable_bell;
    bool enable_blink;
    bool enable_ligatures;
    bool enable_unicode;
    bool enable_emoji;
    uint32_t tab_count;
    uint32_t max_tabs;
    bool enable_tabs;
    bool enable_split;
    bool enable_session_management;
} terminal_config_t;

// Terminal context
typedef struct {
    terminal_config_t config;
    glass_compositor_t* compositor;
    glass_window_manager_t* window_manager;
    uint32_t window_id;
    terminal_tab_t* tabs;
    uint32_t tab_count;
    uint32_t max_tabs;
    uint32_t active_tab;
    terminal_buffer_t* scrollback_buffer;
    char* clipboard_data;
    uint32_t clipboard_size;
    bool initialized;
    bool is_focused;
    terminal_mode_t mode;
    uint32_t next_tab_id;
    uint64_t last_update_time;
} terminal_emulator_t;

// Function prototypes

// Initialization and shutdown
terminal_emulator_t* terminal_emulator_init(glass_compositor_t* compositor,
                                           glass_window_manager_t* window_manager,
                                           terminal_config_t* config);
void terminal_emulator_shutdown(terminal_emulator_t* terminal);
bool terminal_emulator_is_initialized(terminal_emulator_t* terminal);

// Window management
void terminal_emulator_show(terminal_emulator_t* terminal);
void terminal_emulator_hide(terminal_emulator_t* terminal);
void terminal_emulator_minimize(terminal_emulator_t* terminal);
void terminal_emulator_maximize(terminal_emulator_t* terminal);
void terminal_emulator_restore(terminal_emulator_t* terminal);
void terminal_emulator_close(terminal_emulator_t* terminal);
void terminal_emulator_set_size(terminal_emulator_t* terminal, uint32_t width, uint32_t height);
void terminal_emulator_get_size(terminal_emulator_t* terminal, uint32_t* width, uint32_t* height);

// Tab management
uint32_t terminal_emulator_create_tab(terminal_emulator_t* terminal, const char* title);
void terminal_emulator_close_tab(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_activate_tab(terminal_emulator_t* terminal, uint32_t tab_id);
terminal_tab_t* terminal_emulator_get_tab(terminal_emulator_t* terminal, uint32_t tab_id);
terminal_tab_t* terminal_emulator_get_active_tab(terminal_emulator_t* terminal);
uint32_t terminal_emulator_get_tab_count(terminal_emulator_t* terminal);
void terminal_emulator_set_tab_title(terminal_emulator_t* terminal, uint32_t tab_id, const char* title);
const char* terminal_emulator_get_tab_title(terminal_emulator_t* terminal, uint32_t tab_id);

// Terminal output
void terminal_emulator_write(terminal_emulator_t* terminal, uint32_t tab_id, const char* text);
void terminal_emulator_write_char(terminal_emulator_t* terminal, uint32_t tab_id, char character);
void terminal_emulator_write_line(terminal_emulator_t* terminal, uint32_t tab_id, const char* line);
void terminal_emulator_clear_screen(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_clear_line(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t line);
void terminal_emulator_scroll_up(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t lines);
void terminal_emulator_scroll_down(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t lines);

// Terminal input
void terminal_emulator_send_key(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t key_code, uint32_t modifiers);
void terminal_emulator_send_text(terminal_emulator_t* terminal, uint32_t tab_id, const char* text);
void terminal_emulator_send_mouse(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t x, uint32_t y, uint32_t button, bool pressed);
void terminal_emulator_send_paste(terminal_emulator_t* terminal, uint32_t tab_id, const char* text);

// Cursor management
void terminal_emulator_set_cursor_position(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t x, uint32_t y);
void terminal_emulator_get_cursor_position(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t* x, uint32_t* y);
void terminal_emulator_show_cursor(terminal_emulator_t* terminal, uint32_t tab_id, bool show);
void terminal_emulator_set_cursor_style(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t style);
void terminal_emulator_save_cursor(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_restore_cursor(terminal_emulator_t* terminal, uint32_t tab_id);

// Selection management
void terminal_emulator_start_selection(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t x, uint32_t y);
void terminal_emulator_update_selection(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t x, uint32_t y);
void terminal_emulator_end_selection(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_clear_selection(terminal_emulator_t* terminal, uint32_t tab_id);
char* terminal_emulator_get_selected_text(terminal_emulator_t* terminal, uint32_t tab_id);
bool terminal_emulator_has_selection(terminal_emulator_t* terminal, uint32_t tab_id);

// Copy and paste
void terminal_emulator_copy_selection(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_paste(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_copy_to_clipboard(terminal_emulator_t* terminal, const char* text);
char* terminal_emulator_get_from_clipboard(terminal_emulator_t* terminal);

// Color and styling
void terminal_emulator_set_color_scheme(terminal_emulator_t* terminal, color_scheme_t scheme);
color_scheme_t terminal_emulator_get_color_scheme(terminal_emulator_t* terminal);
void terminal_emulator_set_custom_colors(terminal_emulator_t* terminal, terminal_colors_t* colors);
terminal_colors_t* terminal_emulator_get_custom_colors(terminal_emulator_t* terminal);
void terminal_emulator_set_foreground_color(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t color);
void terminal_emulator_set_background_color(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t color);
void terminal_emulator_reset_colors(terminal_emulator_t* terminal, uint32_t tab_id);

// Font management
void terminal_emulator_set_font(terminal_emulator_t* terminal, terminal_font_t* font);
terminal_font_t* terminal_emulator_get_font(terminal_emulator_t* terminal);
void terminal_emulator_set_font_size(terminal_emulator_t* terminal, uint32_t size);
uint32_t terminal_emulator_get_font_size(terminal_emulator_t* terminal);
void terminal_emulator_set_font_name(terminal_emulator_t* terminal, const char* name);
const char* terminal_emulator_get_font_name(terminal_emulator_t* terminal);
void terminal_emulator_enable_ligatures(terminal_emulator_t* terminal, bool enable);
bool terminal_emulator_are_ligatures_enabled(terminal_emulator_t* terminal);

// Buffer management
void terminal_emulator_clear_buffer(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_resize_buffer(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t columns, uint32_t rows);
void terminal_emulator_get_buffer_size(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t* columns, uint32_t* rows);
void terminal_emulator_enable_scrollback(terminal_emulator_t* terminal, bool enable);
bool terminal_emulator_is_scrollback_enabled(terminal_emulator_t* terminal);
void terminal_emulator_set_scrollback_size(terminal_emulator_t* terminal, uint32_t size);
uint32_t terminal_emulator_get_scrollback_size(terminal_emulator_t* terminal);

// Process management
uint32_t terminal_emulator_start_process(terminal_emulator_t* terminal, uint32_t tab_id, const char* command, const char* working_directory);
void terminal_emulator_stop_process(terminal_emulator_t* terminal, uint32_t tab_id);
bool terminal_emulator_is_process_running(terminal_emulator_t* terminal, uint32_t tab_id);
uint32_t terminal_emulator_get_process_id(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_send_signal(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t signal);

// Session management
void terminal_emulator_save_session(terminal_emulator_t* terminal, const char* session_name);
void terminal_emulator_load_session(terminal_emulator_t* terminal, const char* session_name);
void terminal_emulator_delete_session(terminal_emulator_t* terminal, const char* session_name);
char** terminal_emulator_get_sessions(terminal_emulator_t* terminal, uint32_t* count);
bool terminal_emulator_session_exists(terminal_emulator_t* terminal, const char* session_name);

// Split management
uint32_t terminal_emulator_split_horizontal(terminal_emulator_t* terminal, uint32_t tab_id);
uint32_t terminal_emulator_split_vertical(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_close_split(terminal_emulator_t* terminal, uint32_t split_id);
void terminal_emulator_resize_split(terminal_emulator_t* terminal, uint32_t split_id, uint32_t width, uint32_t height);
void terminal_emulator_focus_split(terminal_emulator_t* terminal, uint32_t split_id);

// Rendering
void terminal_emulator_render(terminal_emulator_t* terminal);
void terminal_emulator_render_tab(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_render_cursor(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_render_selection(terminal_emulator_t* terminal, uint32_t tab_id);
void terminal_emulator_render_scrollbar(terminal_emulator_t* terminal, uint32_t tab_id);

// Input handling
bool terminal_emulator_handle_key_press(terminal_emulator_t* terminal, uint32_t key_code, uint32_t modifiers);
bool terminal_emulator_handle_text_input(terminal_emulator_t* terminal, const char* text);
bool terminal_emulator_handle_mouse_move(terminal_emulator_t* terminal, float x, float y);
bool terminal_emulator_handle_mouse_click(terminal_emulator_t* terminal, float x, float y, bool left_click);
bool terminal_emulator_handle_mouse_wheel(terminal_emulator_t* terminal, float x, float y, float delta);

// Information
uint32_t terminal_emulator_get_line_count(terminal_emulator_t* terminal, uint32_t tab_id);
uint32_t terminal_emulator_get_column_count(terminal_emulator_t* terminal, uint32_t tab_id);
bool terminal_emulator_is_focused(terminal_emulator_t* terminal);
terminal_mode_t terminal_emulator_get_mode(terminal_emulator_t* terminal);
const char* terminal_emulator_get_working_directory(terminal_emulator_t* terminal, uint32_t tab_id);

// Utility functions
char* terminal_emulator_get_type_name(terminal_type_t type);
char* terminal_emulator_get_color_scheme_name(color_scheme_t scheme);
char* terminal_emulator_get_font_type_name(font_type_t type);
bool terminal_emulator_is_unicode_supported(char character);
bool terminal_emulator_is_emoji_supported(char* emoji);
uint32_t terminal_emulator_get_character_width(char character);
uint32_t terminal_emulator_get_string_width(const char* string);

// Callbacks
typedef void (*terminal_output_callback_t)(terminal_emulator_t* terminal, uint32_t tab_id, const char* output, void* user_data);
typedef void (*terminal_process_exit_callback_t)(terminal_emulator_t* terminal, uint32_t tab_id, uint32_t exit_code, void* user_data);
typedef void (*terminal_tab_change_callback_t)(terminal_emulator_t* terminal, uint32_t old_tab, uint32_t new_tab, void* user_data);

void terminal_emulator_set_output_callback(terminal_emulator_t* terminal, terminal_output_callback_t callback, void* user_data);
void terminal_emulator_set_process_exit_callback(terminal_emulator_t* terminal, terminal_process_exit_callback_t callback, void* user_data);
void terminal_emulator_set_tab_change_callback(terminal_emulator_t* terminal, terminal_tab_change_callback_t callback, void* user_data);

// Preset configurations
terminal_config_t terminal_emulator_preset_normal_style(void);
terminal_config_t terminal_emulator_preset_developer_style(void);
terminal_config_t terminal_emulator_preset_minimal_style(void);
terminal_config_t terminal_emulator_preset_power_user_style(void);

// Error handling
typedef enum {
    TERMINAL_EMULATOR_SUCCESS = 0,
    TERMINAL_EMULATOR_ERROR_INVALID_CONTEXT,
    TERMINAL_EMULATOR_ERROR_INVALID_TAB,
    TERMINAL_EMULATOR_ERROR_INVALID_BUFFER,
    TERMINAL_EMULATOR_ERROR_OUT_OF_MEMORY,
    TERMINAL_EMULATOR_ERROR_PROCESS_FAILED,
    TERMINAL_EMULATOR_ERROR_INVALID_COMMAND,
    TERMINAL_EMULATOR_ERROR_SESSION_FAILED,
} terminal_emulator_error_t;

terminal_emulator_error_t terminal_emulator_get_last_error(void);
const char* terminal_emulator_get_error_string(terminal_emulator_error_t error);

#endif // TERMINAL_EMULATOR_H 