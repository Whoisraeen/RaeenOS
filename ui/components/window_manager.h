#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "../framework/include/ui_framework.h"
#include "types.h"

// Window states
typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_FULLSCREEN
} window_state_t;

// Window decorations
typedef enum {
    WINDOW_DECORATION_NONE,
    WINDOW_DECORATION_MINIMAL,
    WINDOW_DECORATION_FULL
} window_decoration_t;

// Window manager window structure
typedef struct window_manager_window {
    ui_window_t* ui_window;
    window_state_t state;
    window_decoration_t decoration;
    
    // Window properties
    char title[256];
    ui_rect_t original_bounds;  // Bounds before maximize/minimize
    ui_rect_t current_bounds;
    bool resizable;
    bool movable;
    bool closeable;
    bool minimizable;
    bool maximizable;
    
    // Z-order management
    struct window_manager_window* next;
    struct window_manager_window* prev;
    
    // Window manager specific
    bool is_desktop;
    bool is_taskbar;
    bool is_system_window;
    u32 window_id;
    
    // Event handlers
    void (*on_close)(struct window_manager_window* window);
    void (*on_minimize)(struct window_manager_window* window);
    void (*on_maximize)(struct window_manager_window* window);
    void (*on_restore)(struct window_manager_window* window);
    void (*on_move)(struct window_manager_window* window, ui_point_t new_pos);
    void (*on_resize)(struct window_manager_window* window, ui_size_t new_size);
} window_manager_window_t;

// Window manager configuration
typedef struct {
    bool compositing_enabled;
    bool window_animations;
    bool desktop_effects;
    u32 animation_duration_ms;
    window_decoration_t default_decoration;
    ui_color_t desktop_background;
    ui_color_t taskbar_color;
} window_manager_config_t;

// Window manager functions
error_t window_manager_init(const window_manager_config_t* config);
void window_manager_shutdown(void);

// Window management
window_manager_window_t* window_manager_create_window(const char* title, ui_rect_t bounds);
void window_manager_destroy_window(window_manager_window_t* window);
void window_manager_show_window(window_manager_window_t* window);
void window_manager_hide_window(window_manager_window_t* window);

// Window state management
void window_manager_minimize_window(window_manager_window_t* window);
void window_manager_maximize_window(window_manager_window_t* window);
void window_manager_restore_window(window_manager_window_t* window);
void window_manager_fullscreen_window(window_manager_window_t* window);

// Window positioning and sizing
void window_manager_move_window(window_manager_window_t* window, ui_point_t position);
void window_manager_resize_window(window_manager_window_t* window, ui_size_t size);
void window_manager_center_window(window_manager_window_t* window);

// Z-order management
void window_manager_raise_window(window_manager_window_t* window);
void window_manager_lower_window(window_manager_window_t* window);
void window_manager_bring_to_front(window_manager_window_t* window);
void window_manager_send_to_back(window_manager_window_t* window);

// Focus management
void window_manager_set_focus(window_manager_window_t* window);
window_manager_window_t* window_manager_get_focused_window(void);
window_manager_window_t* window_manager_get_window_at_point(ui_point_t point);

// Desktop management
void window_manager_set_desktop_background(ui_color_t color);
void window_manager_set_desktop_background_image(const char* image_path);
void window_manager_redraw_desktop(void);

// Taskbar management
window_manager_window_t* window_manager_create_taskbar(void);
void window_manager_add_to_taskbar(window_manager_window_t* window);
void window_manager_remove_from_taskbar(window_manager_window_t* window);

// Event handling
bool window_manager_handle_mouse_event(ui_event_t* event);
bool window_manager_handle_keyboard_event(ui_event_t* event);
void window_manager_process_events(void);

// Rendering
void window_manager_render_frame(void);
void window_manager_invalidate_window(window_manager_window_t* window);

// Configuration
void window_manager_set_config(const window_manager_config_t* config);
window_manager_config_t* window_manager_get_config(void);

// Utility functions
window_manager_window_t* window_manager_find_window_by_id(u32 window_id);
window_manager_window_t* window_manager_find_window_by_title(const char* title);
u32 window_manager_get_window_count(void);
void window_manager_list_windows(window_manager_window_t** windows, u32* count);

#endif // WINDOW_MANAGER_H 