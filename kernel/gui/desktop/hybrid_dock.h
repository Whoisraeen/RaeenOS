#ifndef HYBRID_DOCK_H
#define HYBRID_DOCK_H

#include "../rendering/glass_compositor.h"
#include <stdint.h>
#include <stdbool.h>

// Hybrid Dock/Taskbar - Combines macOS dock with Windows taskbar
// Provides glass-like effects, animations, and customization

// Dock position
typedef enum {
    DOCK_POSITION_BOTTOM = 0,    // Bottom (macOS style)
    DOCK_POSITION_TOP,           // Top
    DOCK_POSITION_LEFT,          // Left
    DOCK_POSITION_RIGHT,         // Right
    DOCK_POSITION_FLOATING       // Floating (customizable position)
} dock_position_t;

// Dock mode
typedef enum {
    DOCK_MODE_DOCK = 0,          // macOS-style dock
    DOCK_MODE_TASKBAR,           // Windows-style taskbar
    DOCK_MODE_HYBRID             // Hybrid mode (both features)
} dock_mode_t;

// App icon states
typedef enum {
    APP_ICON_NORMAL = 0,         // Normal state
    APP_ICON_HOVER,              // Mouse hover
    APP_ICON_ACTIVE,             // Application is running
    APP_ICON_FOCUSED,            // Application has focus
    APP_ICON_LAUNCHING,          // Application is launching
    APP_ICON_UPDATING,           // Application is updating
    APP_ICON_ERROR               // Application has error
} app_icon_state_t;

// App icon structure
typedef struct {
    uint32_t id;
    char name[64];
    char path[256];
    char icon_path[256];
    app_icon_state_t state;
    bool pinned;
    bool running;
    uint32_t window_count;
    raeen_rect_t bounds;
    raeen_texture_t* icon_texture;
    raeen_texture_t* hover_texture;
    raeen_texture_t* active_texture;
    float scale;
    float opacity;
    bool visible;
    uint64_t last_click_time;
    void* user_data;
} app_icon_t;

// Dock item types
typedef enum {
    DOCK_ITEM_APP = 0,           // Application
    DOCK_ITEM_FOLDER,            // Folder
    DOCK_ITEM_SEPARATOR,         // Visual separator
    DOCK_ITEM_WIDGET,            // Widget (clock, weather, etc.)
    DOCK_ITEM_CUSTOM             // Custom item
} dock_item_type_t;

// Dock item
typedef struct {
    uint32_t id;
    dock_item_type_t type;
    app_icon_t* app_icon;
    raeen_rect_t bounds;
    bool visible;
    bool enabled;
    void* user_data;
} dock_item_t;

// Dock configuration
typedef struct {
    dock_position_t position;
    dock_mode_t mode;
    uint32_t width, height;
    float opacity;
    float blur_radius;
    bool auto_hide;
    bool show_labels;
    bool show_badges;
    bool enable_animations;
    bool enable_glass_effects;
    uint32_t max_items;
    float icon_size;
    float icon_spacing;
    uint32_t background_color;
    uint32_t border_color;
    float corner_radius;
    bool enable_shadows;
    float shadow_offset_x;
    float shadow_offset_y;
    float shadow_blur;
    uint32_t shadow_color;
} dock_config_t;

// Dock context
typedef struct {
    dock_config_t config;
    glass_compositor_t* compositor;
    dock_item_t* items;
    uint32_t item_count;
    uint32_t next_item_id;
    raeen_rect_t bounds;
    bool visible;
    bool hovered;
    bool expanded;
    float expansion_scale;
    uint32_t hovered_item;
    uint32_t focused_item;
    bool initialized;
    uint64_t animation_start_time;
    float animation_duration;
} hybrid_dock_t;

// Function prototypes

// Initialization and shutdown
hybrid_dock_t* hybrid_dock_init(glass_compositor_t* compositor, dock_config_t* config);
void hybrid_dock_shutdown(hybrid_dock_t* dock);
void hybrid_dock_resize(hybrid_dock_t* dock, uint32_t width, uint32_t height);

// Configuration
void hybrid_dock_set_position(hybrid_dock_t* dock, dock_position_t position);
void hybrid_dock_set_mode(hybrid_dock_t* dock, dock_mode_t mode);
void hybrid_dock_set_opacity(hybrid_dock_t* dock, float opacity);
void hybrid_dock_set_blur_radius(hybrid_dock_t* dock, float blur_radius);
void hybrid_dock_set_auto_hide(hybrid_dock_t* dock, bool auto_hide);
void hybrid_dock_set_glass_effects(hybrid_dock_t* dock, bool enable);
void hybrid_dock_set_animations(hybrid_dock_t* dock, bool enable);

// Item management
uint32_t hybrid_dock_add_app(hybrid_dock_t* dock, const char* name, const char* path, const char* icon_path);
uint32_t hybrid_dock_add_folder(hybrid_dock_t* dock, const char* name, const char* path);
uint32_t hybrid_dock_add_separator(hybrid_dock_t* dock);
uint32_t hybrid_dock_add_widget(hybrid_dock_t* dock, const char* name, void* widget_data);
void hybrid_dock_remove_item(hybrid_dock_t* dock, uint32_t item_id);
void hybrid_dock_clear_items(hybrid_dock_t* dock);

// App management
void hybrid_dock_pin_app(hybrid_dock_t* dock, uint32_t item_id);
void hybrid_dock_unpin_app(hybrid_dock_t* dock, uint32_t item_id);
void hybrid_dock_set_app_state(hybrid_dock_t* dock, uint32_t item_id, app_icon_state_t state);
void hybrid_dock_set_app_running(hybrid_dock_t* dock, uint32_t item_id, bool running);
void hybrid_dock_set_app_focused(hybrid_dock_t* dock, uint32_t item_id, bool focused);
void hybrid_dock_set_app_window_count(hybrid_dock_t* dock, uint32_t item_id, uint32_t window_count);

// Rendering
void hybrid_dock_render(hybrid_dock_t* dock);
void hybrid_dock_update(hybrid_dock_t* dock, float delta_time);
void hybrid_dock_show(hybrid_dock_t* dock);
void hybrid_dock_hide(hybrid_dock_t* dock);
void hybrid_dock_toggle(hybrid_dock_t* dock);

// Input handling
bool hybrid_dock_handle_mouse_move(hybrid_dock_t* dock, float x, float y);
bool hybrid_dock_handle_mouse_click(hybrid_dock_t* dock, float x, float y, bool left_click);
bool hybrid_dock_handle_mouse_wheel(hybrid_dock_t* dock, float x, float y, float delta);
bool hybrid_dock_handle_key_press(hybrid_dock_t* dock, uint32_t key_code);

// Animations
void hybrid_dock_animate_show(hybrid_dock_t* dock, float duration);
void hybrid_dock_animate_hide(hybrid_dock_t* dock, float duration);
void hybrid_dock_animate_expand(hybrid_dock_t* dock, float duration);
void hybrid_dock_animate_collapse(hybrid_dock_t* dock, float duration);
void hybrid_dock_animate_item_hover(hybrid_dock_t* dock, uint32_t item_id, bool hover);

// Item interaction
uint32_t hybrid_dock_get_item_at_point(hybrid_dock_t* dock, float x, float y);
void hybrid_dock_launch_app(hybrid_dock_t* dock, uint32_t item_id);
void hybrid_dock_focus_app(hybrid_dock_t* dock, uint32_t item_id);
void hybrid_dock_show_app_menu(hybrid_dock_t* dock, uint32_t item_id, float x, float y);
void hybrid_dock_show_context_menu(hybrid_dock_t* dock, float x, float y);

// Customization
void hybrid_dock_set_theme(hybrid_dock_t* dock, const char* theme_name);
void hybrid_dock_set_icon_size(hybrid_dock_t* dock, float size);
void hybrid_dock_set_spacing(hybrid_dock_t* dock, float spacing);
void hybrid_dock_set_background_color(hybrid_dock_t* dock, uint32_t color);
void hybrid_dock_set_border_color(hybrid_dock_t* dock, uint32_t color);
void hybrid_dock_set_corner_radius(hybrid_dock_t* dock, float radius);

// Information
raeen_rect_t hybrid_dock_get_bounds(hybrid_dock_t* dock);
bool hybrid_dock_is_visible(hybrid_dock_t* dock);
bool hybrid_dock_is_hovered(hybrid_dock_t* dock);
uint32_t hybrid_dock_get_item_count(hybrid_dock_t* dock);
uint32_t hybrid_dock_get_running_app_count(hybrid_dock_t* dock);
uint32_t hybrid_dock_get_focused_app(hybrid_dock_t* dock);

// Utility functions
void hybrid_dock_center_on_screen(hybrid_dock_t* dock);
void hybrid_dock_align_to_edge(hybrid_dock_t* dock);
void hybrid_dock_update_layout(hybrid_dock_t* dock);
void hybrid_dock_refresh_icons(hybrid_dock_t* dock);

// Callbacks
typedef void (*dock_item_click_callback_t)(hybrid_dock_t* dock, uint32_t item_id, void* user_data);
typedef void (*dock_item_hover_callback_t)(hybrid_dock_t* dock, uint32_t item_id, bool hover, void* user_data);
typedef void (*dock_visibility_change_callback_t)(hybrid_dock_t* dock, bool visible, void* user_data);

void hybrid_dock_set_item_click_callback(hybrid_dock_t* dock, dock_item_click_callback_t callback, void* user_data);
void hybrid_dock_set_item_hover_callback(hybrid_dock_t* dock, dock_item_hover_callback_t callback, void* user_data);
void hybrid_dock_set_visibility_change_callback(hybrid_dock_t* dock, dock_visibility_change_callback_t callback, void* user_data);

// Preset configurations
dock_config_t hybrid_dock_preset_macos_style(void);
dock_config_t hybrid_dock_preset_windows_style(void);
dock_config_t hybrid_dock_preset_hybrid_style(void);
dock_config_t hybrid_dock_preset_minimal_style(void);

// Error handling
typedef enum {
    HYBRID_DOCK_SUCCESS = 0,
    HYBRID_DOCK_ERROR_INVALID_CONTEXT,
    HYBRID_DOCK_ERROR_INVALID_ITEM,
    HYBRID_DOCK_ERROR_OUT_OF_MEMORY,
    HYBRID_DOCK_ERROR_INVALID_CONFIG,
    HYBRID_DOCK_ERROR_RENDER_FAILED,
} hybrid_dock_error_t;

hybrid_dock_error_t hybrid_dock_get_last_error(void);
const char* hybrid_dock_get_error_string(hybrid_dock_error_t error);

#endif // HYBRID_DOCK_H 