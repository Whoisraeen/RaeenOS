#ifndef UI_FRAMEWORK_H
#define UI_FRAMEWORK_H

#include "types.h"
#include <stdbool.h>

// Forward declarations
typedef struct ui_window ui_window_t;
typedef struct ui_widget ui_widget_t;
typedef struct ui_event ui_event_t;
typedef struct ui_theme ui_theme_t;
typedef struct ui_context ui_context_t;

// Color structure
typedef struct {
    u8 r, g, b, a;
} ui_color_t;

// Point and size structures
typedef struct {
    s32 x, y;
} ui_point_t;

typedef struct {
    u32 width, height;
} ui_size_t;

// Rectangle structure
typedef struct {
    ui_point_t position;
    ui_size_t size;
} ui_rect_t;

// Font structure
typedef struct {
    char name[64];
    u32 size;
    u32 weight;
    bool italic;
} ui_font_t;

// Event types
typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_DOWN,
    UI_EVENT_MOUSE_UP,
    UI_EVENT_MOUSE_WHEEL,
    UI_EVENT_KEY_DOWN,
    UI_EVENT_KEY_UP,
    UI_EVENT_WINDOW_CLOSE,
    UI_EVENT_WINDOW_RESIZE,
    UI_EVENT_WINDOW_FOCUS,
    UI_EVENT_WINDOW_BLUR,
    UI_EVENT_PAINT,
    UI_EVENT_TIMER,
    UI_EVENT_CUSTOM
} ui_event_type_t;

// Mouse button enum
typedef enum {
    UI_MOUSE_LEFT = 1,
    UI_MOUSE_RIGHT = 2,
    UI_MOUSE_MIDDLE = 4
} ui_mouse_button_t;

// Key codes (subset of common keys)
typedef enum {
    UI_KEY_ESCAPE = 1,
    UI_KEY_ENTER = 28,
    UI_KEY_SPACE = 57,
    UI_KEY_BACKSPACE = 14,
    UI_KEY_TAB = 15,
    UI_KEY_LEFT = 75,
    UI_KEY_RIGHT = 77,
    UI_KEY_UP = 72,
    UI_KEY_DOWN = 80,
    UI_KEY_HOME = 71,
    UI_KEY_END = 79,
    UI_KEY_PAGE_UP = 73,
    UI_KEY_PAGE_DOWN = 81,
    UI_KEY_DELETE = 83,
    UI_KEY_F1 = 59,
    UI_KEY_F2 = 60,
    UI_KEY_F3 = 61,
    UI_KEY_F4 = 62,
    UI_KEY_F5 = 63,
    UI_KEY_F6 = 64,
    UI_KEY_F7 = 65,
    UI_KEY_F8 = 66,
    UI_KEY_F9 = 67,
    UI_KEY_F10 = 68,
    UI_KEY_F11 = 87,
    UI_KEY_F12 = 88
} ui_key_code_t;

// Widget types
typedef enum {
    UI_WIDGET_WINDOW,
    UI_WIDGET_BUTTON,
    UI_WIDGET_LABEL,
    UI_WIDGET_TEXTBOX,
    UI_WIDGET_CHECKBOX,
    UI_WIDGET_RADIO,
    UI_WIDGET_SLIDER,
    UI_WIDGET_PROGRESS,
    UI_WIDGET_LISTBOX,
    UI_WIDGET_COMBOBOX,
    UI_WIDGET_MENU,
    UI_WIDGET_TOOLBAR,
    UI_WIDGET_STATUSBAR,
    UI_WIDGET_PANEL,
    UI_WIDGET_CANVAS,
    UI_WIDGET_CUSTOM
} ui_widget_type_t;

// Event structure
struct ui_event {
    ui_event_type_t type;
    ui_widget_t* target;
    timestamp_t timestamp;
    
    union {
        struct {
            ui_point_t position;
            ui_point_t delta;
        } mouse_move;
        
        struct {
            ui_point_t position;
            ui_mouse_button_t button;
        } mouse_button;
        
        struct {
            ui_point_t position;
            s32 delta;
        } mouse_wheel;
        
        struct {
            ui_key_code_t key;
            char character;
            bool shift, ctrl, alt;
        } keyboard;
        
        struct {
            ui_size_t new_size;
            ui_size_t old_size;
        } resize;
        
        struct {
            ui_rect_t dirty_rect;
        } paint;
        
        struct {
            u32 timer_id;
        } timer;
        
        struct {
            u32 id;
            void* data;
        } custom;
    };
};

// Event handler function pointer
typedef bool (*ui_event_handler_t)(ui_widget_t* widget, ui_event_t* event);

// Widget base structure
struct ui_widget {
    ui_widget_type_t type;
    ui_widget_t* parent;
    ui_widget_t* first_child;
    ui_widget_t* last_child;
    ui_widget_t* next_sibling;
    ui_widget_t* prev_sibling;
    
    ui_rect_t bounds;
    bool visible;
    bool enabled;
    bool focused;
    
    ui_color_t background_color;
    ui_color_t foreground_color;
    ui_font_t font;
    
    char* text;
    void* user_data;
    
    ui_event_handler_t event_handler;
    
    // Virtual function table
    struct {
        void (*paint)(ui_widget_t* widget, ui_context_t* ctx);
        bool (*handle_event)(ui_widget_t* widget, ui_event_t* event);
        void (*destroy)(ui_widget_t* widget);
        ui_size_t (*get_preferred_size)(ui_widget_t* widget);
    } vtable;
};

// Window structure
struct ui_window {
    ui_widget_t base;
    char title[256];
    bool resizable;
    bool maximized;
    bool minimized;
    ui_size_t min_size;
    ui_size_t max_size;
};

// Theme structure
struct ui_theme {
    char name[64];
    
    // Colors
    ui_color_t window_background;
    ui_color_t widget_background;
    ui_color_t text_color;
    ui_color_t accent_color;
    ui_color_t border_color;
    ui_color_t shadow_color;
    
    // Fonts
    ui_font_t default_font;
    ui_font_t title_font;
    ui_font_t monospace_font;
    
    // Spacing and sizing
    u32 border_width;
    u32 padding;
    u32 margin;
    u32 corner_radius;
    
    // Animation settings
    u32 animation_duration_ms;
    bool animations_enabled;
};

// Rendering context
struct ui_context {
    void* framebuffer;
    u32 width, height;
    u32 pitch;
    u32 bpp;
    ui_theme_t* theme;
    ui_rect_t clip_rect;
};

// UI Framework initialization and management
error_t ui_init(u32 screen_width, u32 screen_height, u32 bpp);
void ui_shutdown(void);
bool ui_is_initialized(void);

// Event handling
void ui_poll_events(void);
bool ui_handle_event(ui_event_t* event);
void ui_post_event(ui_event_t* event);

// Window management
ui_window_t* ui_create_window(const char* title, ui_rect_t bounds);
void ui_destroy_window(ui_window_t* window);
void ui_show_window(ui_window_t* window);
void ui_hide_window(ui_window_t* window);
void ui_set_window_title(ui_window_t* window, const char* title);
void ui_move_window(ui_window_t* window, ui_point_t position);
void ui_resize_window(ui_window_t* window, ui_size_t size);

// Widget management
ui_widget_t* ui_create_widget(ui_widget_type_t type, ui_widget_t* parent);
void ui_destroy_widget(ui_widget_t* widget);
void ui_add_child(ui_widget_t* parent, ui_widget_t* child);
void ui_remove_child(ui_widget_t* parent, ui_widget_t* child);
void ui_set_widget_bounds(ui_widget_t* widget, ui_rect_t bounds);
void ui_set_widget_text(ui_widget_t* widget, const char* text);
void ui_set_widget_visible(ui_widget_t* widget, bool visible);
void ui_set_widget_enabled(ui_widget_t* widget, bool enabled);
void ui_set_widget_event_handler(ui_widget_t* widget, ui_event_handler_t handler);

// Focus management
void ui_set_focus(ui_widget_t* widget);
ui_widget_t* ui_get_focused_widget(void);

// Rendering
void ui_invalidate_widget(ui_widget_t* widget);
void ui_invalidate_rect(ui_rect_t rect);
void ui_render_frame(void);

// Theme management
error_t ui_load_theme(const char* theme_name);
ui_theme_t* ui_get_current_theme(void);
void ui_set_theme(ui_theme_t* theme);

// Drawing primitives (for custom widgets)
void ui_draw_rectangle(ui_context_t* ctx, ui_rect_t rect, ui_color_t color);
void ui_draw_filled_rectangle(ui_context_t* ctx, ui_rect_t rect, ui_color_t color);
void ui_draw_line(ui_context_t* ctx, ui_point_t start, ui_point_t end, ui_color_t color);
void ui_draw_text(ui_context_t* ctx, ui_point_t position, const char* text, ui_font_t* font, ui_color_t color);
void ui_draw_image(ui_context_t* ctx, ui_point_t position, void* image_data, ui_size_t size);

// Utility functions
ui_color_t ui_rgb(u8 r, u8 g, u8 b);
ui_color_t ui_rgba(u8 r, u8 g, u8 b, u8 a);
ui_point_t ui_point(s32 x, s32 y);
ui_size_t ui_size(u32 width, u32 height);
ui_rect_t ui_rect(s32 x, s32 y, u32 width, u32 height);
bool ui_point_in_rect(ui_point_t point, ui_rect_t rect);
ui_rect_t ui_rect_intersect(ui_rect_t a, ui_rect_t b);
ui_rect_t ui_rect_union(ui_rect_t a, ui_rect_t b);

// Built-in themes
extern ui_theme_t ui_theme_light;
extern ui_theme_t ui_theme_dark;
extern ui_theme_t ui_theme_macos_like;
extern ui_theme_t ui_theme_windows_like;

#endif // UI_FRAMEWORK_H