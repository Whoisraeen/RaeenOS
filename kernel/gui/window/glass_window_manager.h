#ifndef GLASS_WINDOW_MANAGER_H
#define GLASS_WINDOW_MANAGER_H

#include "../rendering/glass_compositor.h"
#include <stdint.h>
#include <stdbool.h>

// Glass Window Manager - Elegant window management with glass effects
// Combines macOS window elegance with Windows Snap Layouts

// Window states
typedef enum {
    WINDOW_STATE_NORMAL = 0,     // Normal window
    WINDOW_STATE_MINIMIZED,      // Minimized
    WINDOW_STATE_MAXIMIZED,      // Maximized
    WINDOW_STATE_FULLSCREEN,     // Fullscreen
    WINDOW_STATE_TILED,          // Tiled (Snap Layouts)
    WINDOW_STATE_FLOATING        // Floating (always on top)
} window_state_t;

// Window types
typedef enum {
    WINDOW_TYPE_NORMAL = 0,      // Standard application window
    WINDOW_TYPE_DIALOG,          // Dialog/modal window
    WINDOW_TYPE_TOOLTIP,         // Tooltip window
    WINDOW_TYPE_MENU,            // Menu window
    WINDOW_TYPE_NOTIFICATION,    // Notification window
    WINDOW_TYPE_OVERLAY,         // Overlay window
    WINDOW_TYPE_SPLASH           // Splash screen
} window_type_t;

// Window decorations
typedef enum {
    WINDOW_DECORATION_NONE = 0,  // No decorations
    WINDOW_DECORATION_MINIMAL,   // Minimal (close button only)
    WINDOW_DECORATION_STANDARD,  // Standard (title bar, buttons)
    WINDOW_DECORATION_CUSTOM     // Custom decorations
} window_decoration_t;

// Snap zones (Windows 11 style)
typedef enum {
    SNAP_ZONE_NONE = 0,
    SNAP_ZONE_LEFT,              // Left half
    SNAP_ZONE_RIGHT,             // Right half
    SNAP_ZONE_TOP,               // Top half
    SNAP_ZONE_BOTTOM,            // Bottom half
    SNAP_ZONE_TOP_LEFT,          // Top-left quarter
    SNAP_ZONE_TOP_RIGHT,         // Top-right quarter
    SNAP_ZONE_BOTTOM_LEFT,       // Bottom-left quarter
    SNAP_ZONE_BOTTOM_RIGHT,      // Bottom-right quarter
    SNAP_ZONE_CENTER,            // Center (restore)
    SNAP_ZONE_FULLSCREEN         // Fullscreen
} snap_zone_t;

// Window structure
typedef struct {
    uint32_t id;
    char title[256];
    char app_name[64];
    window_type_t type;
    window_state_t state;
    window_decoration_t decoration;
    raeen_rect_t bounds;
    raeen_rect_t original_bounds; // Bounds before maximize/minimize
    raeen_texture_t* content_texture;
    raeen_texture_t* title_bar_texture;
    glass_layer_t* glass_effect;
    bool visible;
    bool focused;
    bool resizable;
    bool movable;
    bool closeable;
    bool minimizable;
    bool maximizable;
    float opacity;
    float z_order;
    uint32_t process_id;
    void* user_data;
} window_t;

// Window configuration
typedef struct {
    char title[256];
    char app_name[64];
    window_type_t type;
    window_decoration_t decoration;
    raeen_rect_t initial_bounds;
    bool resizable;
    bool movable;
    bool closeable;
    bool minimizable;
    bool maximizable;
    float initial_opacity;
    uint32_t process_id;
} window_config_t;

// Snap layout (Windows 11 style)
typedef struct {
    uint32_t id;
    char name[64];
    raeen_rect_t bounds;
    uint32_t* window_ids;
    uint32_t window_count;
    bool active;
} snap_layout_t;

// AI tiling suggestion
typedef struct {
    uint32_t window_id;
    raeen_rect_t suggested_bounds;
    float confidence;
    const char* reason;
} ai_tiling_suggestion_t;

// Window manager configuration
typedef struct {
    uint32_t max_windows;
    bool enable_glass_effects;
    bool enable_animations;
    bool enable_snap_layouts;
    bool enable_ai_tiling;
    bool enable_window_shadows;
    bool enable_title_bar_blur;
    float animation_duration;
    float glass_blur_radius;
    uint32_t title_bar_height;
    uint32_t border_width;
    uint32_t shadow_offset_x;
    uint32_t shadow_offset_y;
    float shadow_blur;
    uint32_t shadow_color;
} window_manager_config_t;

// Window manager context
typedef struct {
    window_manager_config_t config;
    glass_compositor_t* compositor;
    window_t* windows;
    uint32_t window_count;
    uint32_t next_window_id;
    uint32_t focused_window;
    uint32_t hovered_window;
    snap_layout_t* snap_layouts;
    uint32_t snap_layout_count;
    raeen_rect_t screen_bounds;
    bool initialized;
    bool snap_layouts_visible;
    uint32_t drag_window;
    raeen_rect_t drag_start_bounds;
    float drag_start_x, drag_start_y;
} glass_window_manager_t;

// Function prototypes

// Initialization and shutdown
glass_window_manager_t* glass_window_manager_init(glass_compositor_t* compositor, window_manager_config_t* config);
void glass_window_manager_shutdown(glass_window_manager_t* wm);
void glass_window_manager_resize(glass_window_manager_t* wm, uint32_t width, uint32_t height);

// Window management
uint32_t glass_window_manager_create_window(glass_window_manager_t* wm, window_config_t* config);
void glass_window_manager_destroy_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_show_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_hide_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_close_window(glass_window_manager_t* wm, uint32_t window_id);

// Window state management
void glass_window_manager_minimize_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_maximize_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_restore_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_fullscreen_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_set_window_state(glass_window_manager_t* wm, uint32_t window_id, window_state_t state);

// Window positioning and sizing
void glass_window_manager_move_window(glass_window_manager_t* wm, uint32_t window_id, float x, float y);
void glass_window_manager_resize_window(glass_window_manager_t* wm, uint32_t window_id, float width, float height);
void glass_window_manager_set_window_bounds(glass_window_manager_t* wm, uint32_t window_id, raeen_rect_t bounds);
raeen_rect_t glass_window_manager_get_window_bounds(glass_window_manager_t* wm, uint32_t window_id);

// Window focus and visibility
void glass_window_manager_focus_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_raise_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_lower_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_set_window_opacity(glass_window_manager_t* wm, uint32_t window_id, float opacity);
void glass_window_manager_set_window_visible(glass_window_manager_t* wm, uint32_t window_id, bool visible);

// Window content
void glass_window_manager_set_window_content(glass_window_manager_t* wm, uint32_t window_id, raeen_texture_t* texture);
void glass_window_manager_set_window_title(glass_window_manager_t* wm, uint32_t window_id, const char* title);
void glass_window_manager_set_window_title_bar(glass_window_manager_t* wm, uint32_t window_id, raeen_texture_t* texture);

// Glass effects
void glass_window_manager_set_window_glass_effect(glass_window_manager_t* wm, uint32_t window_id, glass_effect_type_t effect_type);
void glass_window_manager_set_window_glass_params(glass_window_manager_t* wm, uint32_t window_id, raeen_glass_params_t* params);
void glass_window_manager_enable_title_bar_blur(glass_window_manager_t* wm, uint32_t window_id, bool enable);

// Snap layouts (Windows 11 style)
void glass_window_manager_show_snap_layouts(glass_window_manager_t* wm);
void glass_window_manager_hide_snap_layouts(glass_window_manager_t* wm);
void glass_window_manager_snap_window(glass_window_manager_t* wm, uint32_t window_id, snap_zone_t zone);
uint32_t glass_window_manager_create_snap_layout(glass_window_manager_t* wm, const char* name, raeen_rect_t bounds);
void glass_window_manager_destroy_snap_layout(glass_window_manager_t* wm, uint32_t layout_id);
void glass_window_manager_add_window_to_layout(glass_window_manager_t* wm, uint32_t layout_id, uint32_t window_id);
void glass_window_manager_remove_window_from_layout(glass_window_manager_t* wm, uint32_t layout_id, uint32_t window_id);

// AI-enhanced tiling
ai_tiling_suggestion_t* glass_window_manager_get_ai_tiling_suggestions(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_apply_ai_tiling_suggestion(glass_window_manager_t* wm, ai_tiling_suggestion_t* suggestion);
void glass_window_manager_learn_user_tiling_preference(glass_window_manager_t* wm, uint32_t window_id, raeen_rect_t bounds);

// Rendering
void glass_window_manager_render(glass_window_manager_t* wm);
void glass_window_manager_update(glass_window_manager_t* wm, float delta_time);
void glass_window_manager_render_snap_layouts(glass_window_manager_t* wm);

// Input handling
bool glass_window_manager_handle_mouse_move(glass_window_manager_t* wm, float x, float y);
bool glass_window_manager_handle_mouse_click(glass_window_manager_t* wm, float x, float y, bool left_click);
bool glass_window_manager_handle_mouse_drag(glass_window_manager_t* wm, float x, float y, bool start_drag);
bool glass_window_manager_handle_mouse_wheel(glass_window_manager_t* wm, float x, float y, float delta);
bool glass_window_manager_handle_key_press(glass_window_manager_t* wm, uint32_t key_code);

// Window arrangement
void glass_window_manager_arrange_windows_cascade(glass_window_manager_t* wm);
void glass_window_manager_arrange_windows_tile_horizontal(glass_window_manager_t* wm);
void glass_window_manager_arrange_windows_tile_vertical(glass_window_manager_t* wm);
void glass_window_manager_arrange_windows_grid(glass_window_manager_t* wm, uint32_t columns, uint32_t rows);

// Window information
uint32_t glass_window_manager_get_window_count(glass_window_manager_t* wm);
uint32_t glass_window_manager_get_focused_window(glass_window_manager_t* wm);
uint32_t glass_window_manager_get_window_at_point(glass_window_manager_t* wm, float x, float y);
window_t* glass_window_manager_get_window(glass_window_manager_t* wm, uint32_t window_id);
window_state_t glass_window_manager_get_window_state(glass_window_manager_t* wm, uint32_t window_id);

// Utility functions
void glass_window_manager_center_window(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_center_window_on_screen(glass_window_manager_t* wm, uint32_t window_id);
void glass_window_manager_align_window_to_edge(glass_window_manager_t* wm, uint32_t window_id, snap_zone_t zone);
void glass_window_manager_restore_all_windows(glass_window_manager_t* wm);
void glass_window_manager_minimize_all_windows(glass_window_manager_t* wm);

// Callbacks
typedef void (*window_focus_change_callback_t)(glass_window_manager_t* wm, uint32_t window_id, bool focused, void* user_data);
typedef void (*window_state_change_callback_t)(glass_window_manager_t* wm, uint32_t window_id, window_state_t old_state, window_state_t new_state, void* user_data);
typedef void (*window_close_callback_t)(glass_window_manager_t* wm, uint32_t window_id, void* user_data);

void glass_window_manager_set_focus_change_callback(glass_window_manager_t* wm, window_focus_change_callback_t callback, void* user_data);
void glass_window_manager_set_state_change_callback(glass_window_manager_t* wm, window_state_change_callback_t callback, void* user_data);
void glass_window_manager_set_close_callback(glass_window_manager_t* wm, window_close_callback_t callback, void* user_data);

// Preset configurations
window_manager_config_t glass_window_manager_preset_macos_style(void);
window_manager_config_t glass_window_manager_preset_windows_style(void);
window_manager_config_t glass_window_manager_preset_hybrid_style(void);
window_manager_config_t glass_window_manager_preset_minimal_style(void);

// Error handling
typedef enum {
    GLASS_WINDOW_MANAGER_SUCCESS = 0,
    GLASS_WINDOW_MANAGER_ERROR_INVALID_CONTEXT,
    GLASS_WINDOW_MANAGER_ERROR_INVALID_WINDOW,
    GLASS_WINDOW_MANAGER_ERROR_OUT_OF_MEMORY,
    GLASS_WINDOW_MANAGER_ERROR_INVALID_CONFIG,
    GLASS_WINDOW_MANAGER_ERROR_RENDER_FAILED,
} glass_window_manager_error_t;

glass_window_manager_error_t glass_window_manager_get_last_error(void);
const char* glass_window_manager_get_error_string(glass_window_manager_error_t error);

#endif // GLASS_WINDOW_MANAGER_H 