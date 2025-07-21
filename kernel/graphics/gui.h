#ifndef RAEENOS_GUI_H
#define RAEENOS_GUI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// Color structure
typedef struct color {
    uint8_t r, g, b, a;         // Red, Green, Blue, Alpha
} color_t;

// Point structure
typedef struct point {
    int x, y;                   // X and Y coordinates
} point_t;

// Rectangle structure
typedef struct rect {
    int x, y;                   // Top-left corner
    int width, height;          // Width and height
} rect_t;

// Graphics context structure
typedef struct graphics_context {
    void* framebuffer;          // Framebuffer pointer
    uint32_t width;             // Screen width
    uint32_t height;            // Screen height
    uint32_t pitch;             // Bytes per line
    uint32_t bpp;               // Bits per pixel
    color_t foreground;         // Foreground color
    color_t background;         // Background color
    uint32_t font_size;         // Font size
    bool clipping_enabled;      // Clipping enabled
    rect_t clip_rect;           // Clipping rectangle
} graphics_context_t;

// Window types
typedef enum {
    WINDOW_TYPE_NORMAL = 0,     // Normal window
    WINDOW_TYPE_DIALOG = 1,     // Dialog window
    WINDOW_TYPE_POPUP = 2,      // Popup window
    WINDOW_TYPE_TOOLTIP = 3,    // Tooltip window
    WINDOW_TYPE_MENU = 4        // Menu window
} window_type_t;

// Window states
typedef enum {
    WINDOW_STATE_NORMAL = 0,    // Normal state
    WINDOW_STATE_MINIMIZED = 1, // Minimized
    WINDOW_STATE_MAXIMIZED = 2, // Maximized
    WINDOW_STATE_HIDDEN = 3     // Hidden
} window_state_t;

// Window flags
#define WINDOW_FLAG_RESIZABLE    0x0001
#define WINDOW_FLAG_MOVABLE      0x0002
#define WINDOW_FLAG_CLOSABLE     0x0004
#define WINDOW_FLAG_MINIMIZABLE  0x0008
#define WINDOW_FLAG_MAXIMIZABLE  0x0010
#define WINDOW_FLAG_ALWAYS_ON_TOP 0x0020
#define WINDOW_FLAG_FULLSCREEN   0x0040
#define WINDOW_FLAG_BORDERLESS   0x0080

// Window structure
typedef struct window {
    char title[64];             // Window title
    window_type_t type;         // Window type
    window_state_t state;       // Window state
    uint32_t flags;             // Window flags
    rect_t bounds;              // Window bounds
    rect_t client_bounds;       // Client area bounds
    color_t background;         // Background color
    graphics_context_t* gc;     // Graphics context
    void* user_data;            // User data
    struct window* parent;      // Parent window
    struct window* children;    // First child window
    struct window* next_sibling; // Next sibling window
    struct window* prev_sibling; // Previous sibling window
    struct window* next;        // Next window in list
} window_t;

// Widget types
typedef enum {
    WIDGET_TYPE_BUTTON = 0,     // Button widget
    WIDGET_TYPE_LABEL = 1,      // Label widget
    WIDGET_TYPE_TEXTBOX = 2,    // Text box widget
    WIDGET_TYPE_LISTBOX = 3,    // List box widget
    WIDGET_TYPE_CHECKBOX = 4,   // Check box widget
    WIDGET_TYPE_RADIOBUTTON = 5, // Radio button widget
    WIDGET_TYPE_SLIDER = 6,     // Slider widget
    WIDGET_TYPE_PROGRESSBAR = 7, // Progress bar widget
    WIDGET_TYPE_MENU = 8,       // Menu widget
    WIDGET_TYPE_TOOLBAR = 9,    // Toolbar widget
    WIDGET_TYPE_CUSTOM = 10     // Custom widget
} widget_type_t;

// Widget states
typedef enum {
    WIDGET_STATE_NORMAL = 0,    // Normal state
    WIDGET_STATE_HOVER = 1,     // Mouse hover
    WIDGET_STATE_PRESSED = 2,   // Mouse pressed
    WIDGET_STATE_FOCUSED = 3,   // Keyboard focus
    WIDGET_STATE_DISABLED = 4   // Disabled
} widget_state_t;

// Widget structure
typedef struct widget {
    char name[32];              // Widget name
    widget_type_t type;         // Widget type
    widget_state_t state;       // Widget state
    rect_t bounds;              // Widget bounds
    color_t background;         // Background color
    color_t foreground;         // Foreground color
    char* text;                 // Widget text
    void* data;                 // Widget data
    window_t* window;           // Parent window
    struct widget* parent;      // Parent widget
    struct widget* children;    // First child widget
    struct widget* next_sibling; // Next sibling widget
    struct widget* prev_sibling; // Previous sibling widget
    
    // Event handlers
    void (*on_click)(struct widget* widget, int x, int y);
    void (*on_mouse_down)(struct widget* widget, int x, int y, int button);
    void (*on_mouse_up)(struct widget* widget, int x, int y, int button);
    void (*on_mouse_move)(struct widget* widget, int x, int y);
    void (*on_key_down)(struct widget* widget, int key);
    void (*on_key_up)(struct widget* widget, int key);
    void (*on_focus)(struct widget* widget);
    void (*on_blur)(struct widget* widget);
    void (*on_paint)(struct widget* widget, graphics_context_t* gc);
} widget_t;

// Event types
typedef enum {
    EVENT_TYPE_MOUSE_MOVE = 0,  // Mouse move event
    EVENT_TYPE_MOUSE_DOWN = 1,  // Mouse button down
    EVENT_TYPE_MOUSE_UP = 2,    // Mouse button up
    EVENT_TYPE_MOUSE_CLICK = 3, // Mouse click
    EVENT_TYPE_MOUSE_DOUBLE_CLICK = 4, // Mouse double click
    EVENT_TYPE_MOUSE_WHEEL = 5, // Mouse wheel
    EVENT_TYPE_KEY_DOWN = 6,    // Key down
    EVENT_TYPE_KEY_UP = 7,      // Key up
    EVENT_TYPE_KEY_PRESS = 8,   // Key press
    EVENT_TYPE_WINDOW_CLOSE = 9, // Window close
    EVENT_TYPE_WINDOW_RESIZE = 10, // Window resize
    EVENT_TYPE_WINDOW_MOVE = 11, // Window move
    EVENT_TYPE_WINDOW_FOCUS = 12, // Window focus
    EVENT_TYPE_WINDOW_BLUR = 13, // Window blur
    EVENT_TYPE_TIMER = 14,      // Timer event
    EVENT_TYPE_CUSTOM = 15      // Custom event
} event_type_t;

// Event structure
typedef struct event {
    event_type_t type;          // Event type
    uint64_t timestamp;         // Event timestamp
    window_t* window;           // Target window
    widget_t* widget;           // Target widget
    int x, y;                   // Mouse coordinates
    int button;                 // Mouse button
    int key;                    // Key code
    int modifiers;              // Key modifiers
    void* data;                 // Event data
    struct event* next;         // Next event
} event_t;

// GUI system structure
typedef struct gui_system {
    graphics_context_t* screen_gc; // Screen graphics context
    window_t* windows;          // List of windows
    window_t* focused_window;   // Currently focused window
    widget_t* focused_widget;   // Currently focused widget
    event_t* event_queue;       // Event queue
    uint32_t window_count;      // Number of windows
    uint32_t event_count;       // Number of events
    bool running;               // GUI system running
    spinlock_t lock;            // GUI system lock
} gui_system_t;

// Function prototypes

// GUI system initialization
int gui_init(void);
void gui_shutdown(void);
gui_system_t* gui_get_system(void);

// Graphics operations
graphics_context_t* graphics_context_create(uint32_t width, uint32_t height, uint32_t bpp);
void graphics_context_destroy(graphics_context_t* gc);
void graphics_clear(graphics_context_t* gc, color_t color);
void graphics_set_pixel(graphics_context_t* gc, int x, int y, color_t color);
color_t graphics_get_pixel(graphics_context_t* gc, int x, int y);
void graphics_draw_line(graphics_context_t* gc, int x1, int y1, int x2, int y2, color_t color);
void graphics_draw_rect(graphics_context_t* gc, rect_t rect, color_t color, bool filled);
void graphics_draw_circle(graphics_context_t* gc, int x, int y, int radius, color_t color, bool filled);
void graphics_draw_text(graphics_context_t* gc, int x, int y, const char* text, color_t color);
void graphics_copy_rect(graphics_context_t* gc, rect_t src_rect, graphics_context_t* dst_gc, rect_t dst_rect);
void graphics_set_clipping(graphics_context_t* gc, rect_t rect);
void graphics_clear_clipping(graphics_context_t* gc);

// Color operations
color_t color_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
color_t color_blend(color_t color1, color_t color2, float alpha);
uint32_t color_to_rgba(color_t color);
color_t color_from_rgba(uint32_t rgba);

// Window management
window_t* window_create(const char* title, rect_t bounds, window_type_t type, uint32_t flags);
void window_destroy(window_t* window);
int window_show(window_t* window);
int window_hide(window_t* window);
int window_close(window_t* window);
int window_move(window_t* window, int x, int y);
int window_resize(window_t* window, int width, int height);
int window_set_title(window_t* window, const char* title);
int window_set_state(window_t* window, window_state_t state);
int window_focus(window_t* window);
int window_raise(window_t* window);
int window_lower(window_t* window);
window_t* window_find_at_point(int x, int y);
window_t* window_get_top_level(window_t* window);

// Widget management
widget_t* widget_create(widget_type_t type, rect_t bounds, const char* name);
void widget_destroy(widget_t* widget);
int widget_add_child(widget_t* parent, widget_t* child);
int widget_remove_child(widget_t* parent, widget_t* child);
int widget_move(widget_t* widget, int x, int y);
int widget_resize(widget_t* widget, int width, int height);
int widget_set_text(widget_t* widget, const char* text);
int widget_set_visible(widget_t* widget, bool visible);
int widget_set_enabled(widget_t* widget, bool enabled);
int widget_focus(widget_t* widget);
widget_t* widget_find_at_point(widget_t* parent, int x, int y);
widget_t* widget_find_by_name(widget_t* parent, const char* name);

// Widget-specific operations
widget_t* button_create(rect_t bounds, const char* text);
widget_t* label_create(rect_t bounds, const char* text);
widget_t* textbox_create(rect_t bounds, const char* text);
widget_t* listbox_create(rect_t bounds);
widget_t* checkbox_create(rect_t bounds, const char* text);
widget_t* radiobutton_create(rect_t bounds, const char* text);
widget_t* slider_create(rect_t bounds, int min, int max, int value);
widget_t* progressbar_create(rect_t bounds, int min, int max, int value);
widget_t* menu_create(rect_t bounds);
widget_t* toolbar_create(rect_t bounds);

// Event handling
int gui_post_event(event_t* event);
event_t* gui_get_event(void);
void gui_process_events(void);
event_t* event_create(event_type_t type, window_t* window, widget_t* widget);
void event_destroy(event_t* event);
int event_set_mouse_data(event_t* event, int x, int y, int button);
int event_set_key_data(event_t* event, int key, int modifiers);

// Input handling
int gui_handle_mouse_move(int x, int y);
int gui_handle_mouse_button(int x, int y, int button, bool pressed);
int gui_handle_mouse_wheel(int x, int y, int delta);
int gui_handle_key(int key, bool pressed);
int gui_handle_text_input(const char* text);

// Rendering
void gui_render_all(void);
void gui_render_window(window_t* window);
void gui_render_widget(widget_t* widget, graphics_context_t* gc);
void gui_composite_screen(void);

// Window manager
int window_manager_init(void);
void window_manager_shutdown(void);
int window_manager_add_window(window_t* window);
int window_manager_remove_window(window_t* window);
int window_manager_focus_window(window_t* window);
int window_manager_raise_window(window_t* window);
int window_manager_lower_window(window_t* window);
window_t* window_manager_get_focused_window(void);
window_t* window_manager_get_window_at_point(int x, int y);

// Desktop environment
int desktop_init(void);
void desktop_shutdown(void);
int desktop_show(void);
int desktop_hide(void);
int desktop_add_icon(const char* name, const char* icon_path, int x, int y);
int desktop_remove_icon(const char* name);
int desktop_show_context_menu(int x, int y);

// Theme system
typedef struct theme {
    char name[32];              // Theme name
    color_t background;         // Background color
    color_t foreground;         // Foreground color
    color_t accent;             // Accent color
    color_t border;             // Border color
    color_t highlight;          // Highlight color
    color_t shadow;             // Shadow color
    uint32_t border_width;      // Border width
    uint32_t padding;           // Padding
    uint32_t font_size;         // Font size
    char font_name[32];         // Font name
} theme_t;

int theme_load(const char* name);
theme_t* theme_get_current(void);
int theme_set_current(const char* name);
color_t theme_get_color(const char* name);

// GUI utilities
rect_t rect_make(int x, int y, int width, int height);
bool rect_contains(rect_t rect, int x, int y);
bool rect_intersects(rect_t rect1, rect_t rect2);
rect_t rect_intersection(rect_t rect1, rect_t rect2);
rect_t rect_union(rect_t rect1, rect_t rect2);

point_t point_make(int x, int y);
bool point_in_rect(point_t point, rect_t rect);

// GUI debugging
void gui_dump_windows(void);
void gui_dump_widgets(widget_t* widget, int depth);
void gui_dump_events(void);

// GUI constants
#define GUI_MAX_WINDOWS     100
#define GUI_MAX_WIDGETS     1000
#define GUI_MAX_EVENTS      100
#define GUI_DEFAULT_WIDTH   1024
#define GUI_DEFAULT_HEIGHT  768
#define GUI_DEFAULT_BPP     32

#define GUI_COLOR_BLACK     {0, 0, 0, 255}
#define GUI_COLOR_WHITE     {255, 255, 255, 255}
#define GUI_COLOR_RED       {255, 0, 0, 255}
#define GUI_COLOR_GREEN     {0, 255, 0, 255}
#define GUI_COLOR_BLUE      {0, 0, 255, 255}
#define GUI_COLOR_YELLOW    {255, 255, 0, 255}
#define GUI_COLOR_CYAN      {0, 255, 255, 255}
#define GUI_COLOR_MAGENTA   {255, 0, 255, 255}
#define GUI_COLOR_GRAY      {128, 128, 128, 255}
#define GUI_COLOR_LIGHT_GRAY {192, 192, 192, 255}
#define GUI_COLOR_DARK_GRAY {64, 64, 64, 255}

#define GUI_MOUSE_BUTTON_LEFT   1
#define GUI_MOUSE_BUTTON_RIGHT  2
#define GUI_MOUSE_BUTTON_MIDDLE 3

#define GUI_KEY_ESCAPE      27
#define GUI_KEY_ENTER       13
#define GUI_KEY_TAB         9
#define GUI_KEY_BACKSPACE   8
#define GUI_KEY_DELETE      127
#define GUI_KEY_UP          256
#define GUI_KEY_DOWN        257
#define GUI_KEY_LEFT        258
#define GUI_KEY_RIGHT       259
#define GUI_KEY_HOME        260
#define GUI_KEY_END         261
#define GUI_KEY_PAGE_UP     262
#define GUI_KEY_PAGE_DOWN   263

#define GUI_MODIFIER_CTRL   0x01
#define GUI_MODIFIER_SHIFT  0x02
#define GUI_MODIFIER_ALT    0x04
#define GUI_MODIFIER_SUPER  0x08

#endif // RAEENOS_GUI_H 